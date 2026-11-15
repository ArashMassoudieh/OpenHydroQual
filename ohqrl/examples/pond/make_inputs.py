"""
Inflow record for the pond example: hourly runoff from a 2-ha impervious
catchment (runoff coefficient 0.9) under the hourly rain record of NOAA HPD
station USC00180700, Beltsville, MD (data/beltsville_hourly.npz), 1979-2025.
Model time is days since 1900-01-01; flows in m3/day. Missing hours are dry.

    python ohqrl/examples/pond/make_inputs.py
"""
import os

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
d = np.load(os.path.join(HERE, "data", "beltsville_hourly.npz"))
t = d["hour_since_1900"] / 24.0
q = 0.9 * np.nan_to_num(d["rain_m"]) * 20_000.0 * 24.0
keep = (t >= 28854) & (t < 46022)          # 1979-01-01 .. 2026-01-01
np.savetxt(os.path.join(HERE, "inflow.txt"), np.column_stack([t[keep], q[keep]]),
           fmt=["%.6f", "%.4f"], delimiter=", ")
print(f"wrote {keep.sum()} hourly values")
