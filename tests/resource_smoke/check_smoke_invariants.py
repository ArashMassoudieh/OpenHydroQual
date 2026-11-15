#!/usr/bin/env python3
"""Type-specific invariant checks for run_smoke_test.sh -- confirms each
experimental connector/parameter actually behaves as designed (not just
"solved without error," which could pass even if a type or parameter
silently fell back to default behavior and was never actually exercised).
"""
import math
import sys
from pathlib import Path

import numpy as np
import pandas as pd

HERE = Path(__file__).resolve().parent
GEN = HERE / "generated"

# Minimum |recharge_flow| (m^3/day) that counts as "genuinely nonzero" for
# this basin scale -- observed magnitudes here are ~1.9e5 m^3/day, so this
# is set far below that (not a tight physical bound, just large enough to
# reject "identically zero" and float noise, which is what this guards
# against).
MIN_NONZERO_FLOW = 1.0


def col(path, qty):
    with open(path) as f:
        header = f.readline().strip().split(",")
    fields = [h.strip() for h in header]
    n = len(fields) // 2
    m = {fields[2 * i + 1]: 2 * i + 1 for i in range(n)}
    return m[qty]


def load(path, qty):
    c = col(path, qty)
    with open(path) as f:
        header = f.readline().strip().split(",")
    fields = [h.strip() for h in header]
    n = len(fields) // 2
    t_col = 0
    df = pd.read_csv(path, header=None, skiprows=1, usecols=[t_col, c], dtype=float)
    return df[t_col].values, df[c].values


def main():
    ok = True

    # Riparian_Urban_HRU_OneWayRecharge: recharge_flow must never be
    # negative (the upward branch is removed, flow = 0 instead). This
    # fixture is built with wetter-than-default soil and a lowered water
    # table specifically so the gradient favors downward flow for this
    # window (see build_smoke_models.py's VARIANTS comment) -- an earlier
    # version used the default initial conditions, under which the
    # gradient is upward-favoring the whole time and recharge_flow is
    # identically 0, which only proved "never negative" trivially and never
    # exercised the permitted downward branch at all.
    path = GEN / "smoke_oneway_recharge_OHQ_output.txt"
    _, recharge = load(path, "Subbasin_9__recharge_flow")
    if recharge.min() < -1e-9:
        print(f"FAIL: oneway recharge_flow went negative (min={recharge.min():.6g}); "
              f"the upward branch should be exactly 0.")
        ok = False
    else:
        print(f"PASS: oneway recharge_flow min={recharge.min():.6g} (>= 0, as designed)")
    if recharge.max() < MIN_NONZERO_FLOW:
        print(f"FAIL: oneway recharge_flow never exceeded {MIN_NONZERO_FLOW} "
              f"(max={recharge.max():.6g}) -- the permitted downward branch was never "
              f"actually exercised by this fixture.")
        ok = False
    else:
        print(f"PASS: oneway recharge_flow's permitted downward branch is genuinely "
              f"exercised (max={recharge.max():.6g} >= {MIN_NONZERO_FLOW})")

    # Riparian_Urban_HRU_SymmetricKRecharge: K_eff must stay below the raw,
    # unreduced K_sat (1 m/day here) at all times -- if it ever reaches 1,
    # the upward branch is using full saturated conductivity like the
    # default type, not the symmetric kr(Se)-reduced conductivity.
    # NOTE: this only confirms the connector is "bidirectional-capable" by
    # construction (its flow expression has no direction-dependent branch
    # at all, unlike the default type or the oneway type) -- it does NOT
    # demonstrate both directions occurring. In this specific 10-day
    # window recharge_flow is observed to be exclusively negative (upward);
    # the downward branch is never exercised here. Do not report this as
    # "bidirectional behavior demonstrated."
    path = GEN / "smoke_symmetric_k_recharge_OHQ_output.txt"
    _, k_eff = load(path, "Subbasin_9__recharge_K_eff")
    _, recharge_sym = load(path, "Subbasin_9__recharge_flow")
    if k_eff.max() >= 1.0 - 1e-9:
        print(f"FAIL: symmetric_K recharge_K_eff reached {k_eff.max():.6g} "
              f"(>= raw K_sat=1); expected it to stay reduced by kr(Se) always.")
        ok = False
    else:
        print(f"PASS: symmetric_K recharge_K_eff max={k_eff.max():.6g} (< 1, as designed; "
              f"bidirectional-CAPABLE by construction -- not a demonstration both "
              f"directions occurred)")
    sign_note = (
        "both signs observed" if recharge_sym.min() < 0 and recharge_sym.max() > 0
        else "exclusively non-negative (downward only) in this window" if recharge_sym.min() >= 0
        else "exclusively non-positive (upward only) in this window"
    )
    print(f"INFO: symmetric_K recharge_flow in this 10-day smoke window: {sign_note} "
          f"(min={recharge_sym.min():.6g}, max={recharge_sym.max():.6g})")
    max_abs_flow = float(np.max(np.abs(recharge_sym)))
    if max_abs_flow < MIN_NONZERO_FLOW:
        print(f"FAIL: symmetric_K recharge_flow is identically ~0 "
              f"(max|flow|={max_abs_flow:.6g} < {MIN_NONZERO_FLOW}) -- the connector is "
              f"not actually exchanging any water, which would also make the K_eff<1 "
              f"check above vacuous.")
        ok = False
    else:
        print(f"PASS: symmetric_K recharge_flow is genuinely nonzero "
              f"(max|flow|={max_abs_flow:.6g} >= {MIN_NONZERO_FLOW})")

    # hillslope_width_scale / hillslope_length_scale: flow through
    # hillslope_to_near_stream (UC_groundwater_link) is proportional to
    # width/length (see unconfined_groundwater.json's UC_groundwater_link
    # flow expression). Setting both scales to the SAME value leaves that
    # ratio -- and therefore flow -- unchanged and proves nothing. Three
    # variants isolate each factor: a 1/1 baseline, width_scale=0.5 alone
    # (flow should decrease, roughly halve), length_scale=2.0 alone (flow
    # should also decrease, roughly halve, from the denominator).
    t_base, flow_base = load(GEN / "smoke_default_baseline_OHQ_output.txt",
                              "Subbasin_9__hillslope_to_near_stream_flow")
    t_width, flow_width = load(GEN / "smoke_default_width_scale_half_OHQ_output.txt",
                                "Subbasin_9__hillslope_to_near_stream_flow")
    t_length, flow_length = load(GEN / "smoke_default_length_scale_double_OHQ_output.txt",
                                  "Subbasin_9__hillslope_to_near_stream_flow")

    base_integral = float(np.trapz(np.abs(flow_base), t_base))
    width_integral = float(np.trapz(np.abs(flow_width), t_width))
    length_integral = float(np.trapz(np.abs(flow_length), t_length))

    if base_integral <= 0:
        print(f"FAIL: baseline hillslope_to_near_stream_flow integral is {base_integral:.6g} "
              f"(expected nonzero flow to compare against)")
        ok = False
    else:
        width_ratio = width_integral / base_integral
        length_ratio = length_integral / base_integral
        print(f"INFO: hillslope_to_near_stream |flow| integral -- baseline={base_integral:.6g}, "
              f"width_scale=0.5 alone={width_integral:.6g} (ratio={width_ratio:.3f}), "
              f"length_scale=2.0 alone={length_integral:.6g} (ratio={length_ratio:.3f})")
        # Analytically exact for this linear formula (flow ~ width/length):
        # ratio should be 0.5, not merely "somewhere below 0.9". A tolerance
        # that only rejects ratios >= 0.9 would let a seriously wrong wiring
        # (e.g. ratio=0.8, or the two parameters swapped/scaled incorrectly)
        # through silently.
        EXPECTED_RATIO = 0.5
        RATIO_TOL = 0.05
        if not math.isclose(width_ratio, EXPECTED_RATIO, abs_tol=RATIO_TOL):
            print(f"FAIL: hillslope_width_scale=0.5 gave ratio={width_ratio:.3f}, "
                  f"expected {EXPECTED_RATIO} +/- {RATIO_TOL} -- "
                  f"the multiplier may not be wired to hillslope_to_near_stream#width.")
            ok = False
        else:
            print(f"PASS: hillslope_width_scale=0.5 reduced flow independently of length_scale "
                  f"(ratio={width_ratio:.3f}, expected {EXPECTED_RATIO})")
        if not math.isclose(length_ratio, EXPECTED_RATIO, abs_tol=RATIO_TOL):
            print(f"FAIL: hillslope_length_scale=2.0 gave ratio={length_ratio:.3f}, "
                  f"expected {EXPECTED_RATIO} +/- {RATIO_TOL} -- "
                  f"the multiplier may not be wired to hillslope_to_near_stream#length.")
            ok = False
        else:
            print(f"PASS: hillslope_length_scale=2.0 reduced flow independently of width_scale "
                  f"(ratio={length_ratio:.3f}, expected {EXPECTED_RATIO})")

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
