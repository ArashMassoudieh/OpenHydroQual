#!/usr/bin/env python3
"""Check the Heat-constituent test runs (build_models.py) against closed-form solutions.

  mixing      T = 16 + (10 - 16)*exp(-t/2)
  heater      T = 10 + dT*(1 - exp(-t/2)),  dT = H/(rho*c_p*Q) = 200*86.4/(4.186*500)
  surface     T = 25 - 15*exp(-k*t),        k = K*A*0.0864/(rho*c_p*V)
  timeseries  as surface up to day 10, then T = Te + (T10 - Te)*exp(-k*(t - 10)),
              Te = 25 + H/(K*A) = 30
Also checks that the heat content equals rho*c_p*V*T.

Tolerances: 0.05 C on the transient (implicit time stepping with dt = 0.01 d),
0.005 C at the end of the run (steady state).  Exit code 1 on any failure.
"""
import math, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
G = os.path.join(HERE, "generated")
RC = 4.186
T = "Tank_Temperature:concentration"


def read(fname):
    lines = open(os.path.join(G, fname), encoding="utf8").read().splitlines()
    head = [h.strip() for h in lines[0].split(",")]
    names = head[1::2]
    data = {n: ([], []) for n in names}
    for ln in lines[1:]:
        parts = ln.split(",")
        for i, n in enumerate(names):
            try:
                t, v = float(parts[2 * i]), float(parts[2 * i + 1])
            except (ValueError, IndexError):
                continue
            data[n][0].append(t)
            data[n][1].append(v)
    return data


def at(s, t):
    ts, vs = s
    for i in range(1, len(ts)):
        if ts[i] >= t:
            f = (t - ts[i - 1]) / (ts[i] - ts[i - 1])
            return vs[i - 1] + f * (vs[i] - vs[i - 1])
    return vs[-1]


k = 20 * 500 * 0.0864 / (RC * 1000)
dT = 200 * 86.4 / (RC * 500)
Te, T10 = 25 + 50e3 / (20 * 500), 25 - 15 * math.exp(-k * 10)
CASES = {
    "mixing": (20, lambda t: 16 + (10 - 16) * math.exp(-t / 2)),
    "heater": (20, lambda t: 10 + dT * (1 - math.exp(-t / 2))),
    "surface": (30, lambda t: 25 - 15 * math.exp(-k * t)),
    "timeseries": (30, lambda t: 25 - 15 * math.exp(-k * t) if t <= 10
                   else Te + (T10 - Te) * math.exp(-k * (t - 10))),
}

failed = False
for name, (tend, exact) in CASES.items():
    d = read(f"{name}_output.txt")
    print(name)
    for t in [0.5, 1, 2, 5, 10, 12, 15, 20, 30]:
        if t > tend:
            continue
        m, a = at(d[T], t), exact(t)
        tol = 0.005 if t == tend else 0.05
        ok = abs(m - a) <= tol
        failed |= not ok
        print(f"  t={t:5.1f} d  model {m:8.3f}  exact {a:8.3f}  diff {m - a:+.4f}  {'ok' if ok else 'FAIL'}")
    h, tt = at(d["Tank_Temperature:mass"], tend), at(d[T], tend)
    rel = abs(h - RC * 1000 * tt) / (RC * 1000 * tt)
    ok = rel < 1e-4
    failed |= not ok
    print(f"  heat content {h:.1f} MJ vs rho*c_p*V*T {RC * 1000 * tt:.1f} MJ  {'ok' if ok else 'FAIL'}")

print("FAILED" if failed else "all checks passed")
sys.exit(1 if failed else 0)
