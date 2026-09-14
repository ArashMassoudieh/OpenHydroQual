# OpenHydroQual — Known Issues / Revisit Log

This file records defects and parity gaps found while building the **codegen**
(model→C++ compiler, in `codegen/`) and validating it against the interpreter
(`OpenHydroQual-Console` / `System::Solve`). Each entry has enough detail to pick
up cold later.

---

## ISSUE 1 — Interpreter accepts unphysical **negative storage** (outflow-limiting gap)

**Status:** open, deferred (2026-09-13). Do NOT fix yet — logged for later.
**Affects:** both the interpreter and the codegen (codegen deliberately mirrors
the interpreter's logic, so it shares the flaw).

### Symptom
On `AZ12-140/outputs_2D/ohq_grid/run25m/AZ12-140_grid_25m.ohq` (1061 blocks,
2084 links, rainfall-driven), the interpreter leaves some catchment cells at
**negative storage** at t=3, e.g. block `C6_24 = -19.61`. That is ~ -1.5× the
cell's depression-storage capacity (area·depression = 625·0.02 = 12.5) — a real,
unphysical negative volume, not a rounding artifact.

Trajectory (interpreter output.txt): C6_24 fills to +118 at the storm peak
(t≈0.8), then over-drains to **-19.8 at t≈1.09** as it dumps into channel
`Ch_0225` (which feeds the `Skyline` box culvert), then **freezes at ≈ -19.6**
for the rest of the run (no further rain/gradient to recover it).

### Root cause
`System::OneStepSolve` switching loop, `aquifolium/src/System.cpp` ~lines 2462-2500:

```cpp
if (X[i] < -1e-13 && !blocks[i].GetLimitedOutflow() && OutFlowCanOccur(i,variable)) {
    ... SetLimitedOutFlow(i, true); switchvartonegpos = true; ...   // clamp & re-solve
} else if (X[i] >= 1 && blocks[i].GetLimitedOutflow()) { ... }      // un-limit
else if (X[i] < 0) {
    blocks[i].SetOutflowLimitFactor(0, ...);   // <-- only sets a factor; does NOT
}                                              //     re-solve or fail -> ACCEPTS X<0
```

Limiting only engages when `X[i] < -1e-13` **AND `OutFlowCanOccur(i)` is true**.
A cell can over-drain to negative *within a converged step*; by the time the loop
checks, the channel has drained and gradients are ~0, so `OutFlowCanOccur`
returns **false**. The block then hits the final `else if (X[i] < 0)` branch,
which merely sets its outflow factor to 0, does not set `switchvartonegpos`, and
does not return false — so the loop exits and `OneStepSolve` returns `true`,
**accepting the negative storage**. (The outer `Solve` loop is fine: it does not
advance time on a failed step; this is a converged-but-unphysical step.)

`OutFlowCanOccur` (System.cpp:2927) only inspects *current* flow signs (own
inflow<0, links-from flow>0, links-to flow<0); it does not see the entry-time
over-drain that already happened.

The over-drain itself comes from the overland flow forms in
`resources/rainfall_runoff.json`:
- `distributed_catchment_to_stream_link.flow` uses
  `_sqt(hydraulic_gradient) * (_pos(_max(depth.s, head.e - elevation.s) - depression_storage.s))^1.667`
  — the `_max(..., head.e-elevation.s)` ties drainage to the **channel** head and
  `_sqt` is a signed sqrt (reverse flow), so a fast channel drawdown (culvert
  draining) can pull the cell below zero.

### Why it's hard to reproduce small
Isolated 1-cell (`Overland_Neg.ohq`) and 3-cell+culvert (`MultiCell_Neg.ohq`)
models both stay ≥0 (min storage 0.0) — the limiter works there. It only fails in
the full grid's stiff multi-cell drainage. The interpreter log shows Newton
hitting the iteration limit at exactly the failure times:
```
at 1.00208: Number of iterations exceeded the maximum threshold, dt = 0.05
at 1.18739: Number of iterations exceeded the maximum threshold, dt = 0.0421399
```
(models in `/home/arash/Projects/OHQ_codegen_results/models/`.)

### codegen behaves the SAME (by design), but diverged here
`codegen/runtime/ohq_massbalance.h` (~lines 143-150) mirrors the switching loop
exactly, including the negative-acceptance branch. On the AZ grid, codegen gave
`C6_24 = +12.6` (physical) while the interpreter gave `-19.6`. This is **NOT** a
policy difference — the residual is nonlinear with multiple roots in this region,
and the two solvers take different Newton trajectories (interpreter:
analytical/dense-or-sparse Jacobian, tol 1e-3; codegen: colored-FD Jacobian +
ILU0-BiCGSTAB), landing in different basins. Same acceptance policy, different
path → different root.

### Proposed fix (for BOTH, later)
Do not accept a converged step that leaves a non-rigid block with `X[i] < 0`
that cannot be limited. Reject the step (return false) so the outer loop cuts
`dt` and retries; a smaller `dt` reduces the per-step over-drain until the cell
lands ≥0 or `OutFlowCanOccur` stays true and the normal limiter engages. Add a
dt-floor last-resort force-limit (`SetLimitedOutFlow` pinning storage to
`past*landtozero≈0`) so genuinely stuck models don't abort. Apply the identical
change in `codegen/runtime/ohq_massbalance.h`.

**Blast radius / verification:** this changes step acceptance for every model
(incl. GA/MCMC calibration); needs a regression pass over `Examples/`. The only
known trigger (AZ grid) has a ~16 min interpreter run, so verification is slow —
build a faithful multi-cell+culvert reproduction first.

---

## ISSUE 2 — codegen vs interpreter parity summary (AZ grid)

- **991 / 1061 blocks match to < 1e-6**; total-mass difference 0.75%.
- The remaining ~2% is concentrated at the drainage terminus (`Outfall`,
  `EdgeSink`) and the culvert-feeding channels (`Ch_0225`, `Ch_0195`, `Ch_0380`)
  and a handful of edge catchment cells — all downstream consequences of ISSUE 1
  (the negative-storage root divergence) and the solver-path difference.
- Small models are exact / near-exact: `Channel_Test`, `Culvert_Test` = 0 error;
  `Catchment_Test` = 1.4e-6; `Draining_Tank`, `Reservoir_Rule`, `Fixed_Head_Test`
  = machine precision.

## ISSUE 3 — Performance: dense vs sparse; symbolic Jacobian
- Codegen now uses colored-FD sparse Jacobian + ILU0-BiCGSTAB (dense fallback),
  enabled for n≥100. AZ grid: **62.9 s** single-thread (was >15 min dense / timing
  out); interpreter **1000.8 s** wall (multi-core, from run25m.log) → ~16×.
- Remaining optimization: **symbolic/AD Jacobian** emitted from the expression
  trees (exact, cheaper than colored FD). Also **dead-code elimination** (emit
  only per-iteration quantities reachable from the residual).

## ISSUE 4 — codegen feature status (roadmap)
- **Transport Phase A — advection**: DONE (PFR_tracer, reactors match ~1e-6).
- **Transport Phase B — reactions + stoichiometry**: DONE (implemented). Rate
  expressions resolve bare constituent names to `Pos(<block>:concentration)` and
  reaction parameters to their computed `value` (via `RxnParameter::CalcVal`);
  the block reaction term `Σ_r rate_r·stoich_{r,j}·Storage` is folded into the
  transport `inflowOwn`. Validated on a first-order decay (`Decay_Test.ohq`):
  generated reproduces the decay but lands ~2.7% below the interpreter at t=3
  (interp 2155.9 vs gen 2098.6). Investigate later — likely dt-path / stiffness
  (advection alone matches ~6e-4; the reaction term adds sensitivity). Not chased
  further per the "small discrepancy is ok" call.
- **Solver settings** are now emitted from the model (`GetSolverSettings`) into
  both the mass-balance and transport solvers (tolerance, iteration bounds, dt
  grow/shrink = 1/reduction & reduction, dt max/min factors, landtozero) so the
  generated solver adapts identically to the interpreter rather than using
  codegen defaults. Flow models still match at machine precision after this.
- Multi-constituent link coupling (`mass[b*nC+j]`) is in place but only exercised
  at nC=1 so far (Wet_pond with DOM/O2/NH3/NOx would exercise nC=4 + reaction
  parameters with Arrhenius temperature terms — next validation target).

---

## ISSUE 5 — Codegen fixes ALREADY applied during this investigation (current baseline)

These are done and validated; listed so the "current state" is unambiguous when
revisiting. All in `OpenHydroQual/codegen/`.

1. **Source-type quantities (Precipitation/Evapotranspiration).** A `source`
   quantity references a `create source` object valued as `coefficient *
   timeseries * rate` (`Source::GetValue`). Codegen now bakes each *unique*
   source's timeseries once (shared across all objects that reference it — one
   rain gauge → 1024 cells) and expands a source reference to
   `(coeff) * ts_src_<name>.interpol(t) * (rate)`, with `coeff`/`rate` translated
   in the *referencing object's* context (so `coefficient=area` → that block's
   area). Before this, precipitation was emitted as an empty series → 0 rain.
   Code: `CodeGenerator.cpp` resolveValue source branch + source-baking pre-pass;
   helper `srcHandle()`.
2. **Non-finite series points skipped** when baking (the precip series had NaN
   header points at t=0). Code: bake loops in `CodeGenerator.cpp`.
3. **dt clamped to forcing breakpoints** — the solver clamps `dt` so a step never
   crosses a time-series sample time, matching the interpreter's
   `GetMinimumNextTimeStepSize`. Without this the adaptive `dt` stepped over the
   rainfall spike (0.613 at t=0.5) and lost ~45% of runoff volume. Code:
   `ohq_massbalance.h` `breakpoints` + clamp in `step()`; generator emits
   `setBreakpoints(...)` in `initialize()`.
4. **Sparse Jacobian** — `ohq_sparse.h` (CSR + ILU(0)-preconditioned BiCGSTAB,
   dense fallback); `ohq_massbalance.h` builds sparsity from block adjacency,
   fills with **graph-colored finite differences** (~15-20 residual evals vs n),
   solves sparsely. Enabled for n≥100.
5. **Per-iteration emission ordering** — blocks before links, each topologically
   sorted; `_ups`/`_bkw` args recorded as depending on source+dest blocks in the
   analyzer (their bare args look self-located otherwise). Fixes
   use-before-declaration for channel `area = _max(_bkw(head;area),0)` etc.
6. **Rules** (`Quan::_type::rule`) emitted as first-match `if/else if` chains;
   `"unit"` pseudo-rule entries (zero-operator conditions) skipped.
7. **Initial values** evaluated numerically via `Object::CalculateInitialValues`
   at generation time and emitted as literals (safe when an initial expression
   references non-constant quantities).
8. **2-arg `_ups`/`_bkw`** link forms emitted as upstream/downstream selection
   ternaries (`ExpressionEmitter`).

## How to reproduce / verify

Generator + runtime live in `OpenHydroQual/codegen/`. Build once:
```
cmake -S codegen -B codegen/build -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake
cmake --build codegen/build -j
```
Parity for a model (interpreter vs generated, in one process):
```
codegen/tests/run_parity.sh <model.ohq> <ClassName>
```
Generate a standalone library for a model:
```
codegen/build/ohq_generate <model.ohq> <resources_dir> <out_dir> <ClassName>
```
AZ grid benchmark: generate `AZ140Model.h`, compile against `codegen/runtime`,
run to t=3; compare `state(i)` to the interpreter's `output.txt` last row
(`*_Storage` columns). Interpreter run:
```
terminal/TOpenHydroQual/OpenHydroQual-Console <model.ohq> -r resources -o out.txt
```

## Reproduction assets
`/home/arash/Projects/OHQ_codegen_results/` — test models (`models/`), parity
outputs and the grid comparison (`out/`). The codegen lives in
`OpenHydroQual/codegen/` (see its `README.md`).

---

## ISSUE 6 — `addtemplate` resolves file names relative to the process CWD first

**Status:** open (interpreter), 2026-09-13. Not changed per "do not touch the interpreter".

`Command.cpp:203-211` (`addtemplate`, same for `loadtemplate` at 185-191): it
tries `AppendQuanTemplate(assignments["filename"])` with the bare name — i.e.
relative to the **current working directory of the process** — and only if that
fails falls back to `DefaultTemplatePath() + filename`. A stale
`mass_transfer.json` (Jul 2025) sitting at the repo root therefore silently
replaced `resources/mass_transfer.json` whenever a tool was launched from the
repo root: older `AgeTracker` (`inflow_loading = inflow_concentration*inflow`,
guard `1e-05`) instead of the current one (`+Evapotranspiration*concentration`,
guard `1e-4`). Cost half a day of "parity" debugging. Proposed fix: resolve
relative to the model's working folder, then resources — never the process cwd;
or at least log which file was loaded. Meanwhile: run tools from the model's
folder; delete/rename the root `mass_transfer.json`.

## ISSUE 7 — Transport Newton fails repeatedly at large dt (Wetland)

**Status:** open (interpreter), 2026-09-13.

On the Wetland forward model with hourly forcing and the deployment settings
(`max_timestep_increase_factor=50` → dt up to 0.5 d) the interpreter logs 365
`Number of iterations exceeded the maximum threshold, state_variable: 1` events
(the constituent/AgeTracker solve), each followed by a dt cut. The generated
solver, same equations, never fails. Capping dt at 0.02 d removes all events and
the two solvers then agree to 0.13% RMS on storages / 0.2% on tracer mass. Worth
a look at the transport Newton (Jacobian/line search) — it costs the interpreter
~30% of its run time here and makes its trajectory step-path dependent.

## ISSUE 8 — dt clamp follows precipitation only; hourly ET inputs are stepped over on dry days

**Status:** open (interpreter), 2026-09-13. Affects every deployment that uses a
Penman-type ET source with sub-daily inputs.

`System::Solve` sets `alltimeseries = GetTimeSeries(true)` (onlyprecip = true,
System.cpp:1315), so `GetMinimumNextTimeStepSize` (interpol_D over those series)
limits dt only around precipitation changes. Between rain events dt grows to
`dt0 * max_timestep_increase_factor` (0.5 d in the Wetland deployments) and the
hourly `Temperature / R_h / wind_speed / solar_radiation` series of the
`Evapotranspiration_Penmam (S)` source are sampled **twice a day** (at the step
times, 00:00 and 12:00 UTC here). On a dry day the interpreter's ET is then a
straight line between two night/evening values (~2 m³/day) where the resolved
Penman rate peaks at ~20 m³/day at 13:00 local (see
`OpenHydroTwin/deployments/Wetland_truth_codegen/benchmark/results_obs60/`,
"Evaporation" observation: rms error 0.16 vs 3e-3 once dt is capped at 0.02 d).
The interpreter usually *does* resolve the forcing in practice — not by design,
but because its Newton rarely converges below `NR_niteration_lower`, so
`dt_base` seldom grows; a faster solver exposes the gap immediately.

Proposed fix: register every loaded time series in `alltimeseries`
(`GetTimeSeries(false)`), or at least the sources' inputs. The codegen runtime
already clamps on all forcing series (`addClampSeries`, `interpol_D` port).

## Codegen fixes landed 2026-09-13 (baseline update to ISSUE 4/5)

- **G1 parameters as runtime inputs** and **G2 observation emission** (roadmap
  Phase 1): `setasparameter` bindings become `params_[i]` assigned inside
  `buildConstants()` (blocks, links and source values such as
  `solar_scale_fact`), `setParameter(i,v)` + `applyParameters()` recompute every
  derived constant; each Observation's expression is emitted in its object's
  context, evaluated at the accepted state and recorded every step
  (`observationSeries(i)`), initial point included. Gate:
  `codegen/tests/run_parity_obs.sh` (perturbs all parameters, MCMC-style on both
  sides). On the 60-day Wetland model with dt capped: every observation agrees
  to <= 3e-3 rms, final storages 7e-5, 78x faster in-process.
- **dt policy** now mirrors the interpreter's Solve loop (applied step =
  max(min(dt_base, interpol_D), dt0/timestepminfactor); dt_base adapts/shrinks
  separately; `assign_D`/`interpol_D` ported) — but clamping on ALL forcing
  series (ISSUE 8). Deployment-settings Wetland: 17.5k steps, 0.99 s (31x).

- **Transport integrated over the grown dt** (root cause of the 2.7% Decay_Test
  gap in ISSUE 4 — now 2.9e-3): generated `step()` now passes `t_new - t_prev`
  to the transport phase.
- Division and `^` now mirror `Expression::oprt` exactly (`a/(b+1e-23)`,
  `pow(Pos(a),b)`) — `ohq::div`, `ohq::powr`.
- Penman-ET-type sources (internal expression graph), flow-phase geometry in the
  transport phase, sources referenced inside constituent expressions; series
  baked as static tables (compile time 10 min → 2 s).
- Wetland benchmark:
  `OpenHydroTwin/deployments/Wetland_truth_codegen/benchmark/README.md`.
