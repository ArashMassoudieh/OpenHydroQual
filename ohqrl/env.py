"""
Generic Gymnasium environment for closed-loop control of an OpenHydroQual
model, defined by a YAML file (see ohqrl/config.py).

Each episode loads the OHQ model once and steps it interval by interval with
the stepping API of the bindings (begin_stepping / advance_to): before every
interval the agent's action is written to the controlled forcings (a value
property with set_value, or a time-series property with set_timeseries),
the model advances one control interval, and the selected state variables
are read back. The reward is minus the weighted increase, over the interval,
of the model's own objective functions (method 'integrate'), so the sum of
the rewards over an episode is exactly minus the weighted objective that OHQ
computes for that window. The end of an episode is a time limit
(truncation), not a terminal state.
"""
import os
import re
import shutil
import tempfile

import numpy as np
import gymnasium as gym
from gymnasium import spaces

from .config import load_config

# The package lives in the OpenHydroQual repository (<repo>/ohqrl); by default the
# bindings and templates are found relative to it. OHQ_PY_PATH (folder holding
# openhydroqual_py*.so / .pyd) and OHQ_RESOURCES (template folder) override.
OHQ_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OHQ_RESOURCES = os.environ.get("OHQ_RESOURCES", os.path.join(OHQ_ROOT, "resources", ""))


def _find_bindings():
    if os.environ.get("OHQ_PY_PATH"):
        return os.environ["OHQ_PY_PATH"]
    import glob
    hits = sorted(glob.glob(os.path.join(OHQ_ROOT, "PythonBindings", "build", "*", "openhydroqual_py*")),
                  key=lambda p: ("Release" not in p, p))
    return os.path.dirname(hits[0]) if hits else ""


def _import_ohq():
    import sys
    path = _find_bindings()
    if path and path not in sys.path:
        sys.path.append(path)
    try:
        import openhydroqual_py
    except ImportError as e:
        raise ImportError("openhydroqual_py not found: build PythonBindings/PythonBindings.pro "
                          "or set OHQ_PY_PATH to the folder that contains it") from e
    return openhydroqual_py


class Forcing:
    """A forcing time series read from a two-column file (time, value), for
    window means used as observations (current conditions and forecasts)."""

    def __init__(self, path):
        d = np.loadtxt(path, delimiter=",", ndmin=2)
        self.t, self.v = d[:, 0], d[:, 1]
        # cumulative integral, for O(log n) window means
        self.F = np.concatenate([[0.0], np.cumsum(0.5 * (self.v[1:] + self.v[:-1]) * np.diff(self.t))])

    def mean(self, t1, t2):
        if t2 <= t1:
            return float(np.interp(t1, self.t, self.v))
        return float((np.interp(t2, self.t, self.F) - np.interp(t1, self.t, self.F)) / (t2 - t1))


class OHQControlEnv(gym.Env):
    metadata = {"render_modes": []}

    def __init__(self, config, split="train", workdir=None):
        super().__init__()
        self.cfg = load_config(config) if isinstance(config, str) else config
        self.split = split
        self.ohq = _import_ohq()
        self.workdir = workdir or tempfile.mkdtemp(prefix="ohqrl_")
        os.makedirs(self.workdir, exist_ok=True)
        self.script_file = os.path.join(self.workdir, "model.ohq")
        self._write_script()

        t = self.cfg["time"]
        self.dt, self.T = t["control_interval"], t["episode_length"]
        self.n_steps = int(round(self.T / self.dt))
        self.controls = self.cfg["controls"]
        self.objectives = self.cfg["objectives"]
        self.state_obs = [o for o in self.cfg["observations"] if "object" in o]
        self.forcings = {o["forcing"]: Forcing(o["forcing"])
                         for o in self.cfg["observations"] if "forcing" in o}

        n_obs = 0
        for o in self.cfg["observations"]:
            n_obs += len(o["windows"]) if "forcing" in o else 2 if "season" in o else 1
        self.observation_space = spaces.Box(-np.inf, np.inf, shape=(n_obs,), dtype=np.float32)
        self.action_space = spaces.Box(-1.0, 1.0, shape=(len(self.controls),), dtype=np.float32)

        self.sys = None
        self._check_model()

    # ------------------------------------------------------------ model
    def _write_script(self):
        text = open(self.cfg["model"]["script"]).read()
        text = text.replace("${OHQ_RESOURCES}", OHQ_RESOURCES.rstrip("/") + "/")
        text = text.replace("${CONFIG_DIR}", self.cfg["_dir"])
        unresolved = re.findall(r"\$\{[^}]+\}", text)
        if unresolved:
            raise ValueError(f"unresolved placeholders in {self.cfg['model']['script']}: {set(unresolved)}")
        with open(self.script_file, "w") as f:
            f.write(text)

    def _build(self, t0, t1):
        s = self.ohq.System()
        s.set_default_template_path(OHQ_RESOURCES)
        s.set_working_folder(self.workdir + "/")
        s.set_silent(True)
        if not s.create_from_script(self.script_file, OHQ_RESOURCES + "settings.json"):
            raise RuntimeError(f"OHQ could not build the model from {self.cfg['model']['script']}")
        settings = {"n_threads": "1",          # many models run in parallel
                    # An episode is one long run: lift OHQ's whole-run limits on
                    # matrix inversions (default 2e5; OHQ spells the key this way)
                    # and wall-clock time. The config can override both.
                    "maximum_number_of_matrix_inverstions": "2000000000",
                    "maximum_time_allowed": "1e9",
                    **{k: str(v) for k, v in self.cfg["model"]["settings"].items()},
                    "simulation_start_time": repr(float(t0)),
                    "simulation_end_time": repr(float(t1))}
        for k, v in settings.items():
            if not s.set_system_value(k, v):
                raise ValueError(f"OHQ rejected system setting {k}={v}")
        return s

    def _check_model(self):
        """Validate the config against the model once: objects, quantities,
        and that every objective function is additive (method integrate)."""
        t0 = self.cfg["time"]["train"][0][0]
        s = self._build(t0, t0 + self.dt)
        names = s.objective_names()
        for o in self.objectives:
            if o["name"] not in names:
                raise ValueError(f"objective function '{o['name']}' not in the model (has {names})")
            method = (s.objective_method(o["name"]) or "integrate").strip().lower()
            if method != "integrate":
                raise ValueError(f"objective function '{o['name']}' uses method '{method}'; only "
                                 "'integrate' is supported -- formulate it as an integral")
            o["ohq_weight"] = s.objective_weight(o["name"])
        s.begin_stepping(True)
        for o in self.state_obs + self.cfg["snap"]:
            s.get_value(o["object"], o["quantity"])           # raises KeyError if absent
        for c in self.controls:
            if c["kind"] == "value":
                s.get_value(c["object"], c["property"])
        s.end_stepping(False)

    # ---------------------------------------------------------- mapping
    @staticmethod
    def control_from_action(c, a):
        x = 0.5 * (float(np.clip(a, -1.0, 1.0)) + 1.0)
        if c["scale"] == "square":
            x = x * x
        if c["scale"] == "log":
            return float(c["min"] * (c["max"] / c["min"]) ** x)
        return float(c["min"] + x * (c["max"] - c["min"]))

    @staticmethod
    def action_from_control(c, u):
        u = float(np.clip(u, c["min"], c["max"]))
        if c["scale"] == "log":
            x = np.log(u / c["min"]) / np.log(c["max"] / c["min"])
        else:
            x = (u - c["min"]) / (c["max"] - c["min"])
            if c["scale"] == "square":
                x = np.sqrt(x)
        return float(2.0 * x - 1.0)

    def _apply(self, u):
        for c, v in zip(self.controls, u):
            if c["kind"] == "value":
                self.sys.set_value(c["object"], c["property"], v)
            else:
                # constant from now on; the next interval overwrites it
                self.sys.set_timeseries(c["object"], c["property"],
                                        [self.t - 1e-9, self.t + 10 * self.T], [v, v])

    # ------------------------------------------------------ observations
    def _obs(self):
        out = []
        vals = self.sys.get_values([(o["object"], o["quantity"]) for o in self.state_obs])
        it = iter(vals)
        for o in self.cfg["observations"]:
            if "object" in o:
                x = [next(it) / o["scale"]]
            elif "forcing" in o:
                f = self.forcings[o["forcing"]]
                x = []
                for k, (a, b) in enumerate(o["windows"]):
                    m = f.mean(self.t + a * self.dt, self.t + b * self.dt) / o["scale"]
                    if o["noise"] > 0 and b > 0:      # windows reaching into the future are forecasts
                        m *= float(np.exp(self.np_random.normal(0.0, o["noise"]) - 0.5 * o["noise"] ** 2))
                    x.append(m)
            else:
                ph = 2 * np.pi * self.t / o["season"]
                out += [np.sin(ph), np.cos(ph)]
                continue
            if o["transform"] == "log1p":
                x = [np.log1p(max(v, 0.0)) for v in x]
            out += x
        return np.array(out, dtype=np.float32)

    def _objective_integrals(self):
        return np.array([self.sys.objective_integral(o["name"]) for o in self.objectives])

    def objective_weights(self):
        return np.array([o["weight"] * o["ohq_weight"] / o["scale"] for o in self.objectives])

    # ------------------------------------------------------------ gym API
    def episode_start(self, rng):
        ranges = self.cfg["time"]["train"]
        lengths = np.array([max(0.0, b - a - self.T) for a, b in ranges])
        if lengths.sum() <= 0:
            raise ValueError("no training range is longer than the episode")
        k = rng.choice(len(ranges), p=lengths / lengths.sum())
        n = int(lengths[k] // self.dt)
        return ranges[k][0] + self.dt * int(rng.integers(0, n + 1))

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        options = options or {}
        if self.sys is not None and self.sys.is_stepping():
            self.sys.end_stepping(False)
        t0 = options.get("start")
        if t0 is None:
            t0 = self.episode_start(self.np_random)
        self.t0 = self.t = float(t0)
        self.sys = self._build(self.t0, self.t0 + self.T)
        if not self.sys.begin_stepping(True):
            raise RuntimeError("OHQ could not initialize the solver")
        init = options.get("initial_state")
        if init is None:
            init = {}
            for s in self.cfg["initial_state"]:
                lo, hi = s["uniform"]
                init[(s["object"], s["quantity"])] = float(self.np_random.uniform(lo, hi))
        for (obj, q), v in init.items():
            self.sys.set_value(obj, q, v)
        self.k = 0
        self.I = self._objective_integrals()
        return self._obs(), {"t": self.t}

    def _snap(self):
        for s in self.cfg["snap"]:
            if abs(self.sys.get_value(s["object"], s["quantity"])) < s["below"]:
                self.sys.set_value(s["object"], s["quantity"], 0.0)

    def step(self, action):
        a = np.asarray(action, dtype=np.float64).reshape(-1)
        u = [self.control_from_action(c, ai) for c, ai in zip(self.controls, a)]
        self._snap()
        self._apply(u)
        t_next = self.t0 + (self.k + 1) * self.dt
        ok = self.sys.advance_to(t_next)
        self.k += 1
        self.t = t_next
        info = {"t": self.t, "controls": u}
        if not ok:
            info["solver_failed"] = True
            return self._obs_safe(), -float(self.cfg["solver_failure_penalty"]), True, False, info
        I = self._objective_integrals()
        dI = I - self.I
        self.I = I
        reward = -float(np.dot(self.objective_weights(), dI))
        info["objective_increments"] = dI
        truncated = self.k >= self.n_steps
        return self._obs(), reward, False, truncated, info

    def _obs_safe(self):
        try:
            return self._obs()
        except Exception:
            return np.zeros(self.observation_space.shape, dtype=np.float32)

    def objective_totals(self):
        """Integrated objectives since the start of the episode (OHQ's own
        objective values for this window, before weighting)."""
        return dict(zip([o["name"] for o in self.objectives], self._objective_integrals()))

    def close(self):
        if self.sys is not None and self.sys.is_stepping():
            self.sys.end_stepping(False)
        shutil.rmtree(self.workdir, ignore_errors=True)


def constant_policy(env, controls=None):
    """Constant controls ({name: value}; default: the config's baseline),
    returned as an action. Usable as a baseline or as --bc-rule."""
    controls = controls if controls is not None else env.cfg["baseline"]
    return np.array([env.action_from_control(c, controls.get(c["name"], c["min"]))
                     for c in env.controls], dtype=np.float32)
