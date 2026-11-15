"""
Train a control policy for an OHQ model with PPO (proximal policy optimization).

    python -m ohqrl.train ohqrl/examples/pond/pond.yaml --out runs/pond

Settings come from the 'training' section of the YAML file (defaults in
ohqrl.config.TRAINING_DEFAULTS); command-line options override them, e.g.
--total-steps 500000 --workers 16.

The run folder (--out) receives:
    config.yaml            copy of the configuration used
    train_log.txt          one line per update
    checkpoint_uNNNN.pt    policy snapshots every save_every updates (u0000 is
                           the starting policy, e.g. the warm-start rule)
    final.pt               policy after the last update
Pick the best snapshot with ohqrl.evaluate --select (on validation windows).

Algorithm, in short: W worker processes each own one OHQ model and simulate
rollout_steps control intervals with the current policy (a Gaussian around
the actor network's output, for exploration). The main process computes
advantages with GAE, bootstraps from the critic at episode time limits, and
updates the actor and critic networks for a few epochs with the clipped PPO
objective. Optionally the actor is first fitted to a simple rule (warm start,
by behavior cloning) so training starts from that rule's performance.
"""
import argparse
import importlib
import json
import os
import shutil
import sys
import time

# One small OHQ model per worker process: keep numerical libraries
# single-threaded, or N workers x M threads oversubscribe the CPU.
for _v in ("OMP_NUM_THREADS", "OPENBLAS_NUM_THREADS", "MKL_NUM_THREADS"):
    os.environ.setdefault(_v, "1")

import multiprocessing as mp
import numpy as np
import torch
import torch.nn as nn

from .config import load_config


class ActorCritic(nn.Module):
    """Actor: observation -> mean action (tanh MLP); Gaussian exploration with a
    learned, state-independent log standard deviation. Critic: observation ->
    value (expected discounted return)."""

    def __init__(self, obs_dim, act_dim, hidden=64):
        super().__init__()
        self.actor = nn.Sequential(nn.Linear(obs_dim, hidden), nn.Tanh(),
                                   nn.Linear(hidden, hidden), nn.Tanh(),
                                   nn.Linear(hidden, act_dim))
        self.critic = nn.Sequential(nn.Linear(obs_dim, hidden), nn.Tanh(),
                                    nn.Linear(hidden, hidden), nn.Tanh(),
                                    nn.Linear(hidden, 1))
        self.log_std = nn.Parameter(torch.full((act_dim,), -0.5))

    def dist(self, obs):
        return torch.distributions.Normal(self.actor(obs), self.log_std.exp())

    def value(self, obs):
        return self.critic(obs).squeeze(-1)

    @torch.no_grad()
    def act(self, obs, deterministic=False):
        obs = torch.as_tensor(obs, dtype=torch.float32)
        if deterministic:
            return self.actor(obs).clamp(-1, 1).numpy()
        d = self.dist(obs)
        a = d.sample()
        return a.numpy(), d.log_prob(a).sum(-1).numpy(), self.value(obs).numpy()


def _warm_start_rule(ws):
    """The rule to imitate: constant controls, or a user function
    rule(env, **kwargs) -> action given as 'module:function'."""
    from .env import constant_policy
    if "controls" in ws:
        return constant_policy, {"controls": ws["controls"]}
    module, fn = ws["rule"].split(":")
    return getattr(importlib.import_module(module), fn), ws.get("kwargs", {})


def rollout_worker(remote, config_path, seed, hidden):
    """Owns one OHQ model. On request it loads the current policy weights and
    simulates T control intervals on its own, so a slow simulation step delays
    only this worker."""
    from .env import OHQControlEnv
    torch.set_num_threads(1)
    env = OHQControlEnv(config_path)
    obs, _ = env.reset(seed=seed)
    model = ActorCritic(env.observation_space.shape[0], env.action_space.shape[0], hidden)
    ep_ret = 0.0
    while True:
        cmd, data = remote.recv()
        if cmd == "close":
            env.close()
            remote.close()
            return
        if cmd == "warm_start":
            T = data
            rule, kwargs = _warm_start_rule(env.cfg["training"]["warm_start"])
            out = {k: [] for k in ("obs", "act", "rew", "done")}
            for _ in range(T):
                a = np.asarray(rule(env, **kwargs), dtype=np.float32).reshape(-1)
                next_obs, r, term, trunc, _ = env.step(a)
                done = term or trunc
                for k, x in zip(out, (obs, a, r, float(done))):
                    out[k].append(x)
                obs = env.reset()[0] if done else next_obs
            remote.send({k: np.array(v, dtype=np.float32) for k, v in out.items()})
            continue
        state_dict, T = data
        model.load_state_dict(state_dict)
        out = {k: [] for k in ("obs", "act", "logp", "val", "rew", "done", "boot")}
        finished = []
        for _ in range(T):
            a, logp, v = model.act(obs[None])
            next_obs, r, term, trunc, _ = env.step(np.clip(a[0], -1, 1))
            done = term or trunc
            # A time limit is not a terminal state: bootstrap from the critic's
            # value of the state after it.
            boot = 0.0
            if trunc and not term:
                boot = float(model.value(torch.as_tensor(next_obs[None], dtype=torch.float32))[0])
            for k, x in zip(out, (obs, a[0], logp[0], v[0], r, float(done), boot)):
                out[k].append(x)
            ep_ret += r
            if done:
                finished.append(ep_ret)
                ep_ret = 0.0
                next_obs, _ = env.reset()
            obs = next_obs
        with torch.no_grad():
            last_val = float(model.value(torch.as_tensor(obs[None], dtype=torch.float32))[0])
        remote.send(({k: np.array(v, dtype=np.float32) for k, v in out.items()}, last_val, finished))


class RunningMeanStd:
    def __init__(self):
        self.mean, self.var, self.count = 0.0, 1.0, 1e-4

    def update(self, x):
        x = np.asarray(x, dtype=np.float64).ravel()
        b_mean, b_var, b_count = x.mean(), x.var(), x.size
        delta, tot = b_mean - self.mean, self.count + b_count
        self.mean += delta * b_count / tot
        self.var = (self.var * self.count + b_var * b_count + delta ** 2 * self.count * b_count / tot) / tot
        self.count = tot


def _clip(x, c):
    return np.clip(x, -c, c) if c > 0 else x


def _discounted_returns(rew, done, gamma):
    out, g = np.zeros_like(rew), 0.0
    for t in reversed(range(len(rew))):
        g = rew[t] + gamma * g * (1.0 - done[t])
        out[t] = g
    return out


def warm_start(model, remotes, tr, ret_rms, log):
    """Fit the actor to the warm-start rule's actions and the critic to its
    returns (behavior cloning), then set the exploration spread."""
    ws = tr["warm_start"]
    for r_ in remotes:
        r_.send(("warm_start", ws["steps"]))
    batches = [r_.recv() for r_ in remotes]
    obs = torch.as_tensor(np.concatenate([b["obs"] for b in batches]))
    act = torch.as_tensor(np.clip(np.concatenate([b["act"] for b in batches]), -1, 1))
    rew = [b["rew"] for b in batches]
    if tr["normalize_rewards"]:
        for b, rw in zip(batches, rew):
            acc, accs = 0.0, []
            for x, d in zip(rw, b["done"]):
                acc = acc * tr["gamma"] + x
                accs.append(acc)
                acc *= 1.0 - d
            ret_rms.update(accs)
        rew = [_clip(rw / np.sqrt(ret_rms.var + 1e-8), tr["reward_clip"]) for rw in rew]
    ret = torch.as_tensor(np.concatenate([_discounted_returns(rw, b["done"], tr["gamma"])
                                          for rw, b in zip(rew, batches)]))
    opt = torch.optim.Adam(model.parameters(), lr=1e-3)
    n = len(obs)
    for _ in range(ws["epochs"]):
        perm = torch.randperm(n)
        for s in range(0, n, 256):
            idx = perm[s:s + 256]
            loss = (model.actor(obs[idx]) - act[idx]).pow(2).mean() \
                + 0.5 * (model.value(obs[idx]) - ret[idx]).pow(2).mean()
            opt.zero_grad()
            loss.backward()
            opt.step()
    with torch.no_grad():
        err = (model.actor(obs) - act).abs().mean().item()
        model.log_std.fill_(ws["log_std"])
    log(f"warm start: {n} intervals of {ws.get('rule', 'constant controls')}, mean |action error| "
        f"{err:.3f}, exploration std {np.exp(ws['log_std']):.2f}")


def train(config_path, out_dir, overrides=None):
    cfg = load_config(config_path)
    tr = cfg["training"]
    tr.update({k: v for k, v in (overrides or {}).items() if v is not None})
    os.makedirs(out_dir, exist_ok=True)
    shutil.copy(cfg["_path"], os.path.join(out_dir, "config.yaml"))
    logf = open(os.path.join(out_dir, "train_log.txt"), "a")

    def log(msg):
        print(msg, flush=True)
        logf.write(msg + "\n")
        logf.flush()

    log(f"config {cfg['_path']}; training {json.dumps(tr)}")
    torch.set_num_threads(1)
    torch.manual_seed(tr["seed"])
    np.random.seed(tr["seed"])

    from .env import OHQControlEnv
    probe = OHQControlEnv(cfg["_path"])
    obs_dim, act_dim = probe.observation_space.shape[0], probe.action_space.shape[0]
    probe.close()
    model = ActorCritic(obs_dim, act_dim)
    ctx = mp.get_context("spawn")
    remotes, procs = [], []
    for i in range(tr["workers"]):
        parent, child = ctx.Pipe()
        p = ctx.Process(target=rollout_worker, args=(child, cfg["_path"], tr["seed"] + i, 64), daemon=True)
        p.start()
        remotes.append(parent)
        procs.append(p)
    opt = torch.optim.Adam(model.parameters(), lr=tr["lr"])

    N, T, gamma = tr["workers"], tr["rollout_steps"], tr["gamma"]
    shp = (T, N)
    obs_buf = np.zeros(shp + (obs_dim,), np.float32)
    act_buf = np.zeros(shp + (act_dim,), np.float32)
    logp_buf, rew_buf, done_buf, val_buf, boot_buf = (np.zeros(shp, np.float32) for _ in range(5))

    finished_returns = []
    ret_rms, ret_acc = RunningMeanStd(), np.zeros(N)
    if tr["warm_start"]:
        warm_start(model, remotes, tr, ret_rms, log)
    ckpt = lambda u: os.path.join(out_dir, f"checkpoint_u{u:04d}.pt")
    if tr["save_every"]:
        torch.save(model.state_dict(), ckpt(0))
    n_updates = max(1, int(tr["total_steps"]) // (N * T))
    t_start = time.time()

    for update in range(1, n_updates + 1):
        weights = {k: v.detach().clone() for k, v in model.state_dict().items()}
        for r_ in remotes:
            r_.send(("rollout", (weights, T)))
        last_val = np.zeros(N, np.float32)
        for i, r_ in enumerate(remotes):
            batch, last_val[i], finished = r_.recv()
            obs_buf[:, i], act_buf[:, i], logp_buf[:, i] = batch["obs"], batch["act"], batch["logp"]
            val_buf[:, i], rew_buf[:, i], done_buf[:, i] = batch["val"], batch["rew"], batch["done"]
            boot_buf[:, i] = batch["boot"]
            finished_returns.extend(finished)

        raw_rew = rew_buf.copy()
        rew = rew_buf
        if tr["normalize_rewards"]:
            # Scale rewards by the running std of the discounted return.
            accs = np.zeros(shp)
            for t in range(T):
                ret_acc = ret_acc * gamma + rew_buf[t]
                accs[t] = ret_acc
                ret_acc = ret_acc * (1.0 - done_buf[t])
            ret_rms.update(accs)
            rew = _clip(rew_buf / np.sqrt(ret_rms.var + 1e-8), tr["reward_clip"]).astype(np.float32)
        rew = rew + gamma * boot_buf

        adv = np.zeros(shp, np.float32)
        gae = np.zeros(N, np.float32)
        for t in reversed(range(T)):
            next_val = last_val if t == T - 1 else val_buf[t + 1]
            nonterminal = 1.0 - done_buf[t]
            delta = rew[t] + gamma * next_val * nonterminal - val_buf[t]
            gae = delta + gamma * tr["lam"] * nonterminal * gae
            adv[t] = gae
        ret = adv + val_buf

        b_obs = torch.as_tensor(obs_buf.reshape(-1, obs_dim))
        b_act = torch.as_tensor(act_buf.reshape(-1, act_dim))
        b_logp = torch.as_tensor(logp_buf.reshape(-1))
        b_adv = torch.as_tensor(adv.reshape(-1))
        b_ret = torch.as_tensor(ret.reshape(-1))
        b_adv = (b_adv - b_adv.mean()) / (b_adv.std() + 1e-8)

        n = len(b_obs)
        for _ in range(tr["epochs"]):
            perm = torch.randperm(n)
            for s in range(0, n, tr["minibatch"]):
                idx = perm[s:s + tr["minibatch"]]
                d = model.dist(b_obs[idx])
                logp = d.log_prob(b_act[idx]).sum(-1)
                ratio = (logp - b_logp[idx]).exp()
                pg_loss = -torch.min(ratio * b_adv[idx],
                                     ratio.clamp(1 - tr["clip"], 1 + tr["clip"]) * b_adv[idx]).mean()
                v_loss = 0.5 * (model.value(b_obs[idx]) - b_ret[idx]).pow(2).mean()
                loss = pg_loss + tr["vf_coef"] * v_loss - tr["ent_coef"] * d.entropy().sum(-1).mean()
                opt.zero_grad()
                loss.backward()
                nn.utils.clip_grad_norm_(model.parameters(), 0.5)
                opt.step()

        if tr["save_every"] and (update % tr["save_every"] == 0 or update == n_updates):
            torch.save(model.state_dict(), ckpt(update))
        recent = finished_returns[-20:]
        steps = update * N * T
        log(f"update {update:4d}/{n_updates}  intervals {steps:9d}  episodes {len(finished_returns):4d}  "
            f"mean episode return (last 20) {np.mean(recent) if recent else float('nan'):10.4f}  "
            f"exploration std {model.log_std.exp().mean().item():.3f}  "
            f"reward/interval {raw_rew.mean():10.6f}  {steps / (time.time() - t_start):.0f} intervals/s")

    for r_ in remotes:
        r_.send(("close", None))
    for p in procs:
        p.join(timeout=30)
    torch.save(model.state_dict(), os.path.join(out_dir, "final.pt"))
    log(f"saved {os.path.join(out_dir, 'final.pt')}")
    logf.close()


def main(argv=None):
    p = argparse.ArgumentParser(description="Train a PPO control policy for an OHQ model (YAML config).")
    p.add_argument("config")
    p.add_argument("--out", required=True, help="run folder for checkpoints and the log")
    for k in ("total_steps", "workers", "rollout_steps", "epochs", "minibatch", "save_every", "seed"):
        p.add_argument("--" + k.replace("_", "-"), type=int, default=None)
    for k in ("lr", "gamma", "lam", "clip", "ent_coef"):
        p.add_argument("--" + k.replace("_", "-"), type=float, default=None)
    a = p.parse_args(argv)
    overrides = {k: v for k, v in vars(a).items() if k not in ("config", "out")}
    train(a.config, a.out, overrides)


if __name__ == "__main__":
    sys.exit(main())
