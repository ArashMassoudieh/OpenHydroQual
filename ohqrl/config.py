"""
YAML configuration of a generic OHQ control problem.

    model:
      script: pond.ohq                # OHQ script, relative to this file
      settings:                       # extra system settings (setvalue; object=system)
        initial_time_step: 0.0034722
    time:
      control_interval: 0.0416667     # model time units (e.g. days)
      episode_length: 1095
      train: [[29219, 43829]]         # [start, end] ranges for random episode starts
      validation_starts: [31000, 36000]
      test_starts: [43831, 44927]
    controls:                         # controlled forcings / settings (the actions)
      - {name: valve, object: valve, property: opening, kind: value,
         min: 0, max: 1, scale: square}
    observations:
      - {object: Pond, quantity: Storage, scale: 4000}
      - {object: CatchBasin, quantity: "Age:concentration"}
      - {forcing: inflow.txt, windows: [[0, 1], [0, 6], [0, 24]],
         scale: 100000, transform: log1p, noise: 0.3}
      - {season: 365.25}
    objectives:                       # OHQ objective functions (method: integrate)
      - {name: Flood, weight: 1}
      - {name: Young, weight: 1}
    initial_state:                    # optional randomization at reset
      - {object: Pond, quantity: Storage, uniform: [0, 2000]}
    snap:                             # optional: set |state| < below to 0 before each
      - {object: Pond, quantity: Storage, below: 0.1}     # interval (round-off states
                                                          # near empty slow the solver)
    baseline: {valve: 1.0}            # reference control (constant)
    solver_failure_penalty: 10
    training:                         # optional; defaults in TRAINING_DEFAULTS
      total_steps: 3000000
      warm_start: {controls: {valve: 0.1}}

In the script, ${OHQ_RESOURCES} and ${CONFIG_DIR} are replaced by the OHQ
template folder and the folder of the YAML file. Times are in the model's
own time unit. Only additive objectives are supported: every objective
function must use method 'integrate' (formulate anything else as an
integral of a suitable expression).
"""
import os

import yaml

CONTROL_KINDS = ("value", "timeseries")
CONTROL_SCALES = ("linear", "square", "log")
TRANSFORMS = ("none", "log1p")


TRAINING_DEFAULTS = dict(
    total_steps=1_000_000,     # simulated control intervals over all workers
    workers=8,                 # parallel OHQ models (one CPU core each)
    rollout_steps=256,         # intervals each worker simulates per update
    epochs=10, minibatch=256, lr=3e-4,
    gamma=0.99,                # discount per control interval
    lam=0.95,                  # GAE parameter
    clip=0.2, vf_coef=0.5, ent_coef=0.0,
    normalize_rewards=True, reward_clip=0.0,
    save_every=25,             # checkpoint every N updates (0: only the final policy)
    seed=0,
    warm_start=None,           # {controls: {...}} or {rule: "module:function", kwargs: {...}}
)
WARM_START_DEFAULTS = dict(steps=2000, epochs=30, log_std=-1.6)


class ConfigError(ValueError):
    pass


def _require(d, key, where):
    if key not in d:
        raise ConfigError(f"{where}: missing '{key}'")
    return d[key]


def load_config(path):
    path = os.path.abspath(path)
    with open(path) as f:
        cfg = yaml.safe_load(f)
    cfg["_dir"] = os.path.dirname(path)
    cfg["_path"] = path

    model = _require(cfg, "model", "config")
    model["script"] = os.path.join(cfg["_dir"], _require(model, "script", "model"))
    model.setdefault("settings", {})

    t = _require(cfg, "time", "config")
    for k in ("control_interval", "episode_length", "train"):
        _require(t, k, "time")
    t["control_interval"] = float(t["control_interval"])
    t["episode_length"] = float(t["episode_length"])
    t["train"] = [[float(a), float(b)] for a, b in t["train"]]
    t.setdefault("validation_starts", [])
    t.setdefault("test_starts", [])
    if t["episode_length"] < t["control_interval"]:
        raise ConfigError("time: episode_length is shorter than control_interval")

    controls = _require(cfg, "controls", "config")
    if not controls:
        raise ConfigError("controls: at least one control is required")
    for i, c in enumerate(controls):
        where = f"controls[{i}]"
        for k in ("object", "property", "min", "max"):
            _require(c, k, where)
        c.setdefault("name", f"{c['object']}.{c['property']}")
        c.setdefault("kind", "value")
        c.setdefault("scale", "linear")
        if c["kind"] not in CONTROL_KINDS:
            raise ConfigError(f"{where}: kind must be one of {CONTROL_KINDS}")
        if c["scale"] not in CONTROL_SCALES:
            raise ConfigError(f"{where}: scale must be one of {CONTROL_SCALES}")
        c["min"], c["max"] = float(c["min"]), float(c["max"])
        if c["max"] <= c["min"]:
            raise ConfigError(f"{where}: max must exceed min")
        if c["scale"] == "log" and c["min"] <= 0:
            raise ConfigError(f"{where}: scale 'log' needs min > 0")

    obs = _require(cfg, "observations", "config")
    for i, o in enumerate(obs):
        where = f"observations[{i}]"
        o.setdefault("transform", "none")
        o.setdefault("scale", 1.0)
        if o["transform"] not in TRANSFORMS:
            raise ConfigError(f"{where}: transform must be one of {TRANSFORMS}")
        if "forcing" in o:
            o["forcing"] = os.path.join(cfg["_dir"], o["forcing"])
            o.setdefault("windows", [[0, 1]])
            o.setdefault("noise", 0.0)
        elif "season" in o:
            o["season"] = float(o["season"])
        elif not ("object" in o and "quantity" in o):
            raise ConfigError(f"{where}: needs object+quantity, forcing, or season")

    objs = _require(cfg, "objectives", "config")
    if not objs:
        raise ConfigError("objectives: at least one objective function is required")
    for i, o in enumerate(objs):
        _require(o, "name", f"objectives[{i}]")
        o.setdefault("weight", 1.0)
        o.setdefault("scale", 1.0)

    cfg.setdefault("initial_state", [])
    cfg.setdefault("snap", [])
    for i, s in enumerate(cfg["snap"]):
        for k in ("object", "quantity", "below"):
            _require(s, k, f"snap[{i}]")
    cfg.setdefault("baseline", {})
    cfg.setdefault("solver_failure_penalty", 10.0)
    names = [c["name"] for c in controls]
    for name in cfg["baseline"]:
        if name not in names:
            raise ConfigError(f"baseline: unknown control '{name}'")

    tr = dict(TRAINING_DEFAULTS)
    for k, v in (cfg.get("training") or {}).items():
        if k not in TRAINING_DEFAULTS:
            raise ConfigError(f"training: unknown setting '{k}' (known: {sorted(TRAINING_DEFAULTS)})")
        tr[k] = v
    if tr["warm_start"]:
        ws = dict(WARM_START_DEFAULTS, **tr["warm_start"])
        if "controls" in ws:
            for name in ws["controls"]:
                if name not in names:
                    raise ConfigError(f"training.warm_start: unknown control '{name}'")
        elif "rule" not in ws:
            raise ConfigError("training.warm_start needs 'controls' or 'rule'")
        tr["warm_start"] = ws
    cfg["training"] = tr
    return cfg
