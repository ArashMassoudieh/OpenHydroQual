# Culvert examples

Eleven models exercising the culvert templates in `resources/culvert.json` with an
open channel upstream and downstream of the structure and a dynamic inflow
hydrograph. Between them they cover unsubmerged and submerged inlet control,
outlet control under high tailwater, roadway overtopping, reverse flow,
multi-barrel box culverts, and constituent transport through the barrel.

## Running

```bash
./run_all.sh                 # runs all ten, reports exit codes
python3 verify.py            # regime and volume-balance report from the outputs
python3 make_tests.py        # regenerates every .ohq and time series
```

Inlet-control coefficients come from HDS-5 Table A.1; the outlet links apply the
`(dc+D)/2` hydraulic grade line rule, gated to the range where HDS-5 says it is
valid. T11 is the example that exercises that rule — the other ten are mostly
inlet-controlled, where it is correctly inert.

`run_all.sh` finds the console binary at
`terminal/TOpenHydroQual/OpenHydroQual-Console` and sets `OHQ_RESOURCES`; pass a
different binary as the first argument if yours lives elsewhere.

## Layout

Every model except T7 and T10 uses the same arrangement:

```
US_Channel --[Inlet]--> Barrel --[Outlet]--> DS_Channel --[Tailwater]--> Outfall
```

- `US_Channel`, `DS_Channel` — `Trapezoidal Channel Segment`, 3 m base, 2:1 sides, n = 0.035, 100 m
- `Barrel` — `Culvert Barrel (Circular)`, 1.0 m diameter, 20 m long, n = 0.013, 1% slope
- `Inlet` — `Channel_to_culvert (Circular)`: HDS-5 inlet control plus entrance loss, ke = 0.5
- `Outlet` — `Culvert_to_channel (Circular)`: barrel friction, exit loss (kx = 1.0), and the HDS-5 `(dc+D)/2` grade line rule
- `Outfall` — `fixed_head` or `time_variable_fixed_head`

Inlet-control coefficients are the HDS-5 Table A.1 row for **circular concrete,
square edge with headwall** (Chart 1 scale 1: K = 0.0098, M = 2.0, c = 0.0398,
Y = 0.67, Form 1). T5 uses **Chart 8 scale 1**, rectangular concrete box with
30–75° wingwall flares (K = 0.026, M = 1.0, c = 0.0347, Y = 0.81, Form 1). Both
are verified against the manual.

## The tests

| # | Model | Condition covered | Result |
|---|---|---|---|
| T1 | `T1_inlet_control` | Unsubmerged inlet control, free outfall | HW/D 0.75, submergence weight 0, inlet control 100% of steps |
| T2 | `T2_submerged_inlet` | Submerged inlet, barrel surcharges | HW/D 2.74, weight 1.00, 0.63 m surcharge above the crown |
| T3 | `T3_outlet_control_tailwater` | Rising tailwater drowns the culvert | Control shifts: inlet governs only 59% of steps, outlet control the rest |
| T4 | `T4_roadway_overtopping` | Extreme storm overtops the road | 0.45 m over a 2.3 m crest; **51% of the total volume goes over the road** |
| T5 | `T5_box_multibarrel` | Three-barrel box culvert | 1.5 × 1.2 m cells, 9.0 m³/s peak, HW/rise 0.99 |
| T6 | `T6_reverse_flow` | Downstream surge drives flow backwards | Flow reverses to −0.093 m³/s; barrel surcharges 1.15 m |
| T7 | `T7_lumped_vs_routed` | Single-link vs storage-routed barrel, same storm | Peaks agree to 0.05%, volumes to 0.01% |
| T8 | `T8_dry_to_dry` | Dry start, pulse inflow, full drain-down | Returns to dry, no negative storage |
| T9 | `T9_steep_slope` | 5% barrel slope | Inlet control throughout, barrel only 41% full |
| T10 | `T10_water_quality` | Conservative tracer through the culvert | 50 → 49.8 → 47.7 g/m³ across the structure |
| T11 | `T11_outlet_control_hgl` | Long rough barrel, firmly outlet-controlled | HDS-5 `(dc+D)/2` rule active on 57% of steps; tailwater raised 0.45 → 1.37 m |

## Volume balance

`verify.py` closes the balance across the whole structure
(inflow − outflow − storage change). Every model closes to better than 0.16%,
most below 0.08%. T6 and T11 need a smaller starting time step to get there; the
settings are in their `.ohq` files and the reason is commented in
`make_tests.py`. In both cases it was `initial_time_step`, not the Newton
tolerance, that mattered.

## Flow accounting

Where a roadway is present the two paths are reported separately, and `flow` is
always the total:

- single-link variants: `culvert_flow` + `overtopping_flow` = `flow`, plus
  `overtopping_fraction`
- routed layout: the barrel links and the `Roadway_Overtopping` link are
  separate objects, so each is reported on its own

## Input files

Time series are plain `time (day), value` CSVs written by `make_tests.py`:
`inflow_moderate.csv` (0.8 m³/s peak), `inflow_large.csv` (3.2), `inflow_extreme.csv`
(9.0), `inflow_small.csv` (0.1), `inflow_pulse.csv` (square pulse),
`tailwater_rise.csv` and `surge.csv` (boundary stage), `tracer_in.csv`
(inflow concentration).
