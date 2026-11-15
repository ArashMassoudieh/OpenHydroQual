"""
Evaluate policies for a generic OHQ control problem (YAML config) on fixed
windows, using the model's own objective functions.

    # baseline (constant controls from the config) and other constants
    python -m ohqrl.evaluate ohqrl/examples/pond/pond.yaml --windows test --constant "valve=0.05" "valve=0.1"
    # PPO checkpoints (deterministic); keep the best one on the validation windows
    python -m ohqrl.evaluate ohqrl/examples/pond/pond.yaml --windows validation \
        --ppo "runs/pond/checkpoint_u*.pt" --select runs/pond/best.pt

For every policy and window it reports each objective's integral over the
window (equal to OHQ's objective value for an integrate objective) and the
weighted total J = sum_i w_i * objective_i / scale_i (lower is better). With
--normalize, objectives are also divided by the baseline's.
"""
import os
os.environ.setdefault("OMP_NUM_THREADS", "1")
import argparse
import glob
import json
import shutil
from multiprocessing import Pool

import numpy as np

from .config import load_config


def _policy(spec, env):
    from .env import constant_policy
    kind = spec[0]
    if kind == "constant":
        return lambda obs: constant_policy(env, spec[1])
    if kind == "ppo":
        import torch
        from .train import ActorCritic
        model = ActorCritic(env.observation_space.shape[0], env.action_space.shape[0])
        model.load_state_dict(torch.load(spec[1]))
        return lambda obs: model.act(obs[None], deterministic=True)[0]
    raise ValueError(kind)


def run_window(job):
    """job = (config path, policy spec, start, seed) -> objective totals."""
    from .env import OHQControlEnv
    cfg_path, spec, start, seed = job
    env = OHQControlEnv(cfg_path)
    obs, _ = env.reset(seed=seed, options={"start": start, "initial_state": {}})
    policy = _policy(spec, env)
    done, failed = False, False
    while not done:
        obs, r, term, trunc, info = env.step(policy(obs))
        failed = failed or info.get("solver_failed", False)
        done = term or trunc
    totals = env.objective_totals()
    J = float(np.dot(env.objective_weights(), list(totals.values())))
    env.close()
    return dict(start=start, totals=totals, J=J, failed=failed)


def label(spec):
    return f"ppo:{os.path.basename(spec[1])}" if spec[0] == "ppo" else \
        "constant:" + ",".join(f"{k}={v}" for k, v in spec[1].items())


def main(args):
    cfg = load_config(args.config)
    starts = cfg["time"][f"{args.windows}_starts"]
    if not starts:
        raise SystemExit(f"no {args.windows}_starts in {args.config}")
    specs = [("constant", cfg["baseline"])]
    for c in args.constant:
        specs.append(("constant", {k: float(v) for k, v in (kv.split("=") for kv in c.split(","))}))
    for pattern in args.ppo:
        specs += [("ppo", p) for p in sorted(glob.glob(pattern))]
    jobs = [(cfg["_path"], sp, s, args.seed) for sp in specs for s in starts]
    with Pool(min(args.workers, len(jobs))) as pool:
        res = pool.map(run_window, jobs, chunksize=1)
    n = len(starts)
    names = [o["name"] for o in cfg["objectives"]]
    base = res[:n]
    table = []
    for i, sp in enumerate(specs):
        rr = res[i * n:(i + 1) * n]
        row = dict(policy=label(sp), path=sp[1] if sp[0] == "ppo" else None,
                   J=float(np.mean([r["J"] for r in rr])),
                   objectives={k: float(np.mean([r["totals"][k] for r in rr])) for k in names},
                   failed=any(r["failed"] for r in rr))
        if args.normalize:
            row["normalized"] = {k: float(np.mean([r["totals"][k] / b["totals"][k]
                                                   for r, b in zip(rr, base) if b["totals"][k] > 0]))
                                 for k in names}
        table.append(row)
        extra = "  normalized " + " ".join(f"{k} {v:.3f}" for k, v in row["normalized"].items()) \
            if args.normalize else ""
        print(f"{row['policy']:40s} J {row['J']:12.4f}  " +
              " ".join(f"{k} {v:.4g}" for k, v in row["objectives"].items()) + extra +
              ("  [solver failed]" if row["failed"] else ""), flush=True)
    out = dict(config=cfg["_path"], windows=args.windows, starts=starts, table=table)
    if args.select:
        cands = [r for r in table if r["path"] and not r["failed"]]
        if not cands:
            raise SystemExit("no PPO checkpoints to select from")
        best = min(cands, key=lambda r: r["J"])
        shutil.copy(best["path"], args.select)
        out["best"] = best["path"]
        print(f"best: {best['path']} (J {best['J']:.4f}) -> {args.select}")
    if args.log:
        json.dump(out, open(args.log, "w"), indent=1)


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("config")
    p.add_argument("--windows", choices=["validation", "test"], default="test")
    p.add_argument("--constant", nargs="*", default=[], help='constant controls, e.g. "valve=0.2"')
    p.add_argument("--ppo", nargs="*", default=[], help="PPO checkpoint files or glob patterns")
    p.add_argument("--select", default="", help="copy the best PPO checkpoint (lowest J) here")
    p.add_argument("--normalize", action="store_true", help="also report objectives / baseline")
    p.add_argument("--log", default="", help="write the table as JSON")
    p.add_argument("--seed", type=int, default=0)
    p.add_argument("--workers", type=int, default=16)
    main(p.parse_args())
