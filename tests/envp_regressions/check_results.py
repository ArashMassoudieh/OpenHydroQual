#!/usr/bin/env python3
"""Check the envp_regressions runs (build_models.py) against closed-form solutions.

  asr_particle, fixed_head  CSTR filling with inflow at 100 g/m3, tau = 2 d:
                            C = 100*(1 - exp(-t/2))
  sorbed_reactor            batch, D -> S first order (k = 0.5/d), volume = Storage:
                            D = 100*exp(-k t),  S = 100 - D
Also fails when a console log contains "does not exist".
Exit code 1 on any failure.
"""
import math, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
G = os.path.join(HERE, "generated")


def read(fname):
    lines = open(os.path.join(G, fname), encoding="utf8").read().splitlines()
    head = [h.strip() for h in lines[0].split(",")]
    data = {n: ([], []) for n in head[1::2]}
    for ln in lines[1:]:
        p = ln.split(",")
        for i, n in enumerate(head[1::2]):
            try:
                data[n][0].append(float(p[2 * i])); data[n][1].append(float(p[2 * i + 1]))
            except (ValueError, IndexError):
                pass
    return data


def at(s, t):
    ts, vs = s
    for i in range(1, len(ts)):
        if ts[i] >= t:
            f = (t - ts[i - 1]) / (ts[i] - ts[i - 1])
            return vs[i - 1] + f * (vs[i] - vs[i - 1])
    return vs[-1]


cstr = lambda t: 100 * (1 - math.exp(-t / 2))
CASES = {
    "asr_particle": [("Aeration_X:concentration", cstr)],
    "fixed_head": [("Tank_C:concentration", cstr)],
    "sorbed_reactor": [("Batch_D:concentration", lambda t: 100 * math.exp(-0.5 * t)),
                       ("Batch_S:concentration", lambda t: 100 - 100 * math.exp(-0.5 * t))],
}
failed = False
for name, series in CASES.items():
    print(name)
    log = open(os.path.join(G, name + "_console.log"), encoding="utf8", errors="replace").read()
    if "does not exist" in log or "no property called" in log:
        failed = True
        print("  FAIL: missing-property messages in", name + "_console.log")
    try:
        d = read(name + "_output.txt")
    except OSError:
        failed = True; print("  FAIL: no output"); continue
    for col, exact in series:
        for t in (0.5, 1, 2, 5):
            m, a = at(d[col], t), exact(t)
            ok = abs(m - a) <= 0.02 * 100 * 0.05   # 0.1 g/m3, implicit steps of 0.01 d
            failed |= not ok
            print(f"  {col:28s} t={t:4.1f}  model {m:8.3f}  exact {a:8.3f}  {'ok' if ok else 'FAIL'}")
print("FAILED" if failed else "all checks passed")
sys.exit(1 if failed else 0)
