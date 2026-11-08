#!/usr/bin/env python3
"""Check the batch sorption test runs against their closed-form solutions.

With X = k_ads*C - k_des*S (the net adsorption rate), every case reduces to
dX/dt = -lambda*X, so C relaxes exponentially to its equilibrium value:
    C(t) = C_eq + (C0 - C_eq) * exp(-lambda*t)
                      lambda               C_eq
  immobile_chem   k_ads + k_des          k_des*C0/(k_ads + k_des)
  sorbed_chem     k_ads + n*k_des        n*C0/(n + k_ads/k_des)
  sorbed_desorp   k_ads/n + k_des        n*C0/(n + k_ads/k_des)
All three are built so that C_eq = C0/R with R = 1 + rho_b*K_d/n = 3.

Checks for each correct case:
  1. mass conservation: Cw:mass + Cs:mass stays at C0*n*V (relative drift < 1e-4;
     the output file carries about 6 significant figures)
  2. the reported masses are the concentrations times the right volume
     (Storage for Cw and an immobile Cs, the total volume for a sorbed Cs; < 1e-4,
     the templates divide by Storage + 1e-5)
  3. C(t) follows the closed form (max error < 0.5 % of C0; the time step is 0.001 day)
  4. equilibrium: C -> C0/R and the sorbed mass per mass of solid -> K_d*C
porosity_stoich must FAIL check 1 (it is the reason the stoichiometric
constant has to stay 1 rather than n/rho_b).
Usage: python check_results.py   (reads generated/*_output.txt)
"""
import math, os, sys
import numpy as np

N, RHO, KD, KDES, C0, V = 0.4, 1600.0, 5e-4, 1.0, 10.0, 1.0
R = 1 + RHO * KD / N
HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "generated")

def load(name):
    L = open(os.path.join(HERE, name + "_output.txt")).read().splitlines()
    h = [s.strip() for s in L[0].split(",")]
    d = np.array([[float(s) if s.strip() else np.nan for s in l.split(",")[:len(h)]] for l in L[1:] if l.strip()])
    return {h[i]: (d[:, i - 1], d[:, i]) for i in range(1, len(h), 2)}

def series(c, q):
    t, v = c["Cell_" + q]; ok = ~np.isnan(t); return t[ok], v[ok]

cases = {
    "immobile_chem": dict(kads=KDES * RHO * KD / N, lam=lambda ka: ka + KDES, Sbasis="water"),
    "sorbed_chem":   dict(kads=KDES * RHO * KD,     lam=lambda ka: ka + N * KDES, Sbasis="total"),
    "sorbed_desorp": dict(kads=KDES * RHO * KD,     lam=lambda ka: ka / N + KDES, Sbasis="total"),
}
fails = 0
def report(ok, msg):
    global fails
    print(("PASS " if ok else "FAIL ") + msg); fails += (not ok)

for name, cfg in cases.items():
    c = load(name); ka = cfg["kads"]
    t, C = series(c, "Cw:concentration"); _, S = series(c, "Cs:concentration")
    _, MC = series(c, "Cw:mass"); _, MS = series(c, "Cs:mass"); _, ST = series(c, "Storage")
    M0 = C0 * N * V; tot = MC + MS
    drift = np.max(np.abs(tot - M0)) / M0
    report(drift < 1e-4, f"{name}: total mass conserved (max drift {drift:.1e})")
    sv = ST if cfg["Sbasis"] == "water" else V
    e1 = np.max(np.abs(MC - C * ST)) / M0; e2 = np.max(np.abs(MS - S * sv)) / M0
    report(e1 < 1e-4 and e2 < 1e-4, f"{name}: masses = concentration x volume (C per water, S per {cfg['Sbasis']} volume; max error {max(e1,e2):.1e})")
    Ceq = C0 / R
    Cex = Ceq + (C0 - Ceq) * np.exp(-cfg["lam"](ka) * t)
    err = np.max(np.abs(C - Cex)) / C0
    report(err < 5e-3, f"{name}: C(t) matches the closed form, lambda = {cfg['lam'](ka):.2f}/day (max error {100*err:.2f} % of C0)")
    Send = S[-1]; s_per_solid = (N * Send if cfg["Sbasis"] == "water" else Send) / RHO
    report(abs(C[-1] - Ceq) / Ceq < 1e-4 and abs(s_per_solid - KD * C[-1]) / (KD * C[-1]) < 1e-4,
           f"{name}: equilibrium C = {C[-1]:.4f} (C0/R = {Ceq:.4f}), sorbed per mass of solid = {s_per_solid:.4e} g/kg (K_d*C = {KD*C[-1]:.4e})")

c = load("porosity_stoich")
_, MC = series(c, "Cw:mass"); _, MS = series(c, "Cs:mass"); M0 = C0 * N * V
drift = np.max(np.abs(MC + MS - M0)) / M0
report(drift > 0.01, f"porosity_stoich: mass balance violated as expected with stoichiometric constant n/rho_b (drift {100*drift:.1f} %)")
print("ALL CHECKS PASSED" if fails == 0 else f"{fails} CHECK(S) FAILED")
sys.exit(1 if fails else 0)
