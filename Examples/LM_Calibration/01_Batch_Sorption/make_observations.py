#!/usr/bin/env python3
"""Regenerate obs_Cu_aqueous.csv and obs_Cu_sorbed.csv for this case.

Runs truth.ohq forward, samples both concentrations at 60 log-spaced times
from 0.02 to 20, and multiplies each value by exp(0.05*z), z ~ N(0,1)
(5 % log-normal noise, matching error_standard_deviation = 0.05 in
calibration.ohq). The seed is fixed, so the data are reproducible.

Usage: OHQ_CONSOLE=/path/to/OHQTerminal python make_observations.py
"""
import os, subprocess, sys
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
console = os.environ.get("OHQ_CONSOLE")
if not console:
    sys.exit("Set OHQ_CONSOLE to an OpenHydroQual console.")
subprocess.run([console, "truth.ohq", "-q", "-o", "truth_output.txt"], cwd=HERE, check=True,
               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
L = open(os.path.join(HERE, "truth_output.txt")).read().splitlines()
h = [s.strip() for s in L[0].split(",")]
d = np.array([[float(s) if s.strip() else np.nan for s in l.split(",")[:len(h)]] for l in L[1:] if l.strip()])
def series(name):
    i = h.index(name); t, v = d[:, i - 1], d[:, i]; ok = ~np.isnan(t); return t[ok], v[ok]
times = np.geomspace(0.02, 20, 60)
rng = np.random.default_rng(20261108)
for obs, col in (("obs_Cu_aqueous.csv", "Groundwater cell (1)_Cu_aq:concentration"),
                 ("obs_Cu_sorbed.csv", "Groundwater cell (1)_Cu_s:concentration")):
    t, v = series(col)
    vals = np.interp(times, t, v) * np.exp(0.05 * rng.standard_normal(times.size))
    with open(os.path.join(HERE, obs), "w", newline="\n") as f:
        for ti, vi in zip(times, vals):
            f.write(f"{ti:.6g},{vi:.8g}\n")
    print(obs, "first", f"{vals[0]:.4g}", "last", f"{vals[-1]:.4g}")
os.remove(os.path.join(HERE, "truth_output.txt"))
