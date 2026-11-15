# ohqrl: reinforcement learning for OpenHydroQual models

Train a control policy (valve settings, pump rates, releases, ...) for any
OpenHydroQual model with reinforcement learning.

You supply two things: the OHQ model as a normal `.ohq` script, and a short
YAML file that says
- what is controlled,
- what the controller sees,
- which of the model's own objective functions to minimize.

**The full user's manual is [docs/ohqrl_manual.pdf](docs/ohqrl_manual.pdf).**

```bash
# from the OpenHydroQual folder, after building OHQLib and PythonBindings
pip install -r ohqrl/requirements.txt
python -m ohqrl.tests.test_stepping                       # checks the bindings
python ohqrl/examples/pond/make_inputs.py                 # example inflow record
python -m ohqrl.evaluate ohqrl/examples/pond/pond.yaml --windows test --constant "valve=0.05" "valve=0.1"
python -m ohqrl.train ohqrl/examples/pond/pond.yaml --out runs/pond
python -m ohqrl.evaluate ohqrl/examples/pond/pond.yaml --windows validation \
    --ppo "runs/pond/checkpoint_u*.pt" --select runs/pond/best.pt
python -m ohqrl.evaluate ohqrl/examples/pond/pond.yaml --windows test --ppo runs/pond/best.pt --normalize
```

| file | purpose |
|---|---|
| `config.py` | reads and checks the YAML file |
| `env.py` | `OHQControlEnv`: steps the OHQ model one control interval at a time (a Gymnasium environment) |
| `train.py` | PPO training with parallel workers, warm start and checkpoints |
| `evaluate.py` | runs fixed controls and trained policies on validation/test windows; picks the best checkpoint |
| `examples/pond/` | stormwater pond with a controllable valve (script, YAML, rain data) |
| `tests/test_stepping.py` | stepped run vs. single solve parity test |
| `docs/` | user's manual (LaTeX source and PDF) |
