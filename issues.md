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

**Status:** fixed 2026-09-25 — `alltimeseries = GetTimeSeries(false)`; also
`System::GetTimeSeries` assigned D to the wrong series under `onlyprecip`, and
`assign_D`/`interpol_D` (interpreter and kernel alike) measured the distance to
the first sample of the NEXT value, floored at one spacing, so a step from a dry
spell landed on the first wet sample and applied its rate over the whole step.
D is now the time to the next sample at which the series starts to change.
Opened 2026-09-13. Not changed per "do not touch the interpreter".

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

**Status:** fixed 2026-09-25 — `alltimeseries = GetTimeSeries(false)`; also
`System::GetTimeSeries` assigned D to the wrong series under `onlyprecip`, and
`assign_D`/`interpol_D` (interpreter and kernel alike) measured the distance to
the first sample of the NEXT value, floored at one spacing, so a step from a dry
spell landed on the first wet sample and applied its rate over the whole step.
D is now the time to the next sample at which the series starts to change.
Opened 2026-09-13.

On the Wetland forward model with hourly forcing and the deployment settings
(`max_timestep_increase_factor=50` → dt up to 0.5 d) the interpreter logs 365
`Number of iterations exceeded the maximum threshold, state_variable: 1` events
(the constituent/AgeTracker solve), each followed by a dt cut. The generated
solver, same equations, never fails. Capping dt at 0.02 d removes all events and
the two solvers then agree to 0.13% RMS on storages / 0.2% on tracer mass. Worth
a look at the transport Newton (Jacobian/line search) — it costs the interpreter
~30% of its run time here and makes its trajectory step-path dependent.

## ISSUE 8 — dt clamp follows precipitation only; hourly ET inputs are stepped over on dry days

**Status:** fixed 2026-09-25 — `alltimeseries = GetTimeSeries(false)`; also
`System::GetTimeSeries` assigned D to the wrong series under `onlyprecip`, and
`assign_D`/`interpol_D` (interpreter and kernel alike) measured the distance to
the first sample of the NEXT value, floored at one spacing, so a step from a dry
spell landed on the first wet sample and applied its rate over the whole step.
D is now the time to the next sample at which the series starts to change.
Opened 2026-09-13. Affects every deployment that uses a
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

---

## ISSUE 9 — Link expressions reference an unresolvable bare `dispersivity`, silently 0

**Status:** open (found 2026-09-14 during codegen↔interpreter parity work).
**Affects:** the interpreter. Cosmetic in effect, but it produces the
"*N* error(s) during the solve" banner on every transport model and masks real
errors.

### Symptom
Running the 8-column study model (`ColumnStudy/CodegenBench8/col8.ohq`) always
reports `*** 8 error(s) during the solve:` with no further detail. The run is
otherwise correct.

### Cause
`Object::GetVal(s, ...)` (aquifolium/src/Object.cpp:99) resolves a bare name on
a link through a fallback chain, and when every branch misses it appends
`property '<s>' does not exist in '<object>'` (code 1002) and returns **0**.

`Cu_aq:diffusive_masstransfer` on a link evaluates two different terms (seen
with the `OHQ_TERMLOG` trace added during this work):

```
TERMLOG-ENTRY parameter='dispersivity'       W='Flow1-1' quan=(nil)          <- misses, returns 0 + error
TERMLOG       parameter='Cu_aq:dispersivity' W='Flow1-1' out=0.0485256       <- resolves via the constituent
```

The qualified form resolves (Object.cpp:138: a `<constituent>:<property>` name
is routed to the constituent object). The bare form does not: the links carry no
`dispersivity` and no `Cu_aq:dispersivity` quantity — only the three expression
quantities `Cu_aq:advective_masstransfer`, `Cu_aq:diffusive_masstransfer`,
`Cu_aq:masstransfer`.

### Why it matters
The result is numerically right — the qualified term supplies the value — but
every transport model emits spurious errors, so a genuine error is easy to miss.
Worth deciding whether the bare term should be dropped at copy time or resolved
the same way as the qualified one.

---

## ISSUE 10 — `optimize_lambda`: the adopted half-step is discarded at loop exit

**Status:** open, behaviour question (found 2026-09-14).
**Affects:** the interpreter (codegen now reproduces it deliberately).

### What happens
In `System::OneStepSolve` (System.cpp:2457-2492) each iteration runs:

1. `ComputeNewtonStep`: `X -= dx` (X is now the full damped step), `X1 = X + 0.5*dx`
2. `F1 = GetResiduals(X1)`  — loads X1 into the model
3. `F  = GetResiduals(X)`   — loads the **full step** back in
4. `AdjustNRCoefficient` may set `X = X1` (System.cpp:6467) — **local variable only**

So the model's committed state is always the full step. When the loop exits
immediately after the half-step was adopted, the half-step is silently
discarded and never reaches the state.

### Why it matters
This is not rare. In the transport phase the third loop escape,
`dx_norm/X_norm < 1e-10` (System.cpp:2377), fires after **one** iteration on
every step of the column model, because the sorbed mass dominates the state
vector and dwarfs the aqueous correction (measured `dx_norm/X_norm ~ 7e-13`).
So on every step `optimize_lambda` decides the half-step is better, and every
step then commits the full step anyway. The accepted increment is exactly 2x the
one the damping selected.

This may well be intended (the convergence test is evaluated on the full step's
residual, so committing that is self-consistent) — but if the half-step is meant
to be adopted, it must be written back through `SetStateVariables` before the
loop exits. Worth a decision either way; it changes results.

### Note on the escape itself
Because that escape is scale-dependent, `nr_tolerance` has **no effect** on the
transport phase of a sorption-dominated model — it always takes exactly one
iteration. That is worth knowing when tuning accuracy.

---

## ISSUE 11 — Oscillation control is a discrete decision, so the two codes can diverge by one event

**Status:** open, revisit later (logged 2026-09-14 when oscillation control was
ported to the codegen).
**Affects:** codegen↔interpreter parity, not correctness of either.

### Context
The codegen now implements the interpreter's oscillation control:
`CountOscillatingStates` (System.cpp:1012) over the gathered state
(flow states then transport masses, `GatherSolvedState`), the rewind path
(`ResetBasedOnRestorePoint`, 5130) including the output knockout, the
`reduce`-only path, the `dt_ceiling` hold, the relax-after-N-clean-steps
doubling, the reduction budget and the 4-step cooldown. With
`oscillation_control = No` the generated code is **bit-identical** to before the
port, so the machinery is genuinely inert when off.

### Symptom
On the 8-column model (`ColumnStudy/CodegenBench8/attrib/osc.ohq`,
remedy = `reduce`, 50x dt ceiling) with control ON:

| | steps | oscillation events | ceiling relaxations |
|---|---|---|---|
| interpreter | ~2070 | 3 | — |
| codegen | 1869 | 2 | 4 |

Breakthrough agreement loosens from **-0.010%** median (control off) to
**-0.116%** median, range -1.66%..+0.25%, worst |diff|/scale 3.9e-2.

### Why
The detector fires on a threshold test over a four-deep state history. The two
codes agree to ~1e-7 per step, but that is enough to put one of them on the
other side of the threshold at a marginal step. One extra (or missing) event
changes dt for the following hundreds of steps, so the trajectories separate —
the disagreement is not in the physics but in *when* the control triggers.

### Re-measured 2026-09-14 (afternoon), current code
Re-run end-to-end with today's fixes in place (ISSUE 14 reaction-parameter
precedence, ISSUE 17 parameter binding, ISSUE 18 X_norm, ISSUE 19 copy safety,
`/` and `^` parity). Interpreter re-run too; note it must be given `-w .` or the
model's relative `observed_outputfile` resolves under `attrib/attrib/` and the
run produces nothing (same doubled-path class as ISSUE 15).

| | interpreter | codegen | speed-up | steps (interp / codegen) |
|---|---:|---:|---:|---|
| control OFF (`def`) | 23.5 s | 0.45 s | **52.6x** | 142 / **141** |
| control ON (`osc`)  | 202.7 s | 4.86 s | **41.8x** | **2069** / **1869** |

Median signed relative difference over t>3 d across the 8 bottom ports
(`analyze.py`): **-0.05%** control off, **-0.51%** control on.

### Accuracy against the converged reference — the more useful framing
`analyze.py osc def ref` (reference = `ref.ohq`, `max_timestep_increase_factor=1`):

| variant | codegen - interp | interp - ref | codegen - ref |
|---|---:|---:|---:|
| `ref`  (ceiling 1)             | **-0.00%** | +0.00% | -0.00% |
| `def`  (ceiling 50, control off) | -0.05% | **-23.90%** | -24.11% |
| `osc`  (ceiling 50, control on)  | -0.51% | **-7.93%**  | -8.47%  |

Two things follow, and they matter more than the parity percentages above:

1. **At the converged dt the two codes are the same code**: codegen vs
   interpreter is **-0.00%** on `ref`. So the -0.05% / -0.51% at ceiling 50 are
   dt-path artefacts of two adaptive controllers, not a physics difference.
2. **Oscillation control is buying real accuracy, not just stability.** At the
   same ceiling it moves the breakthrough from **-23.9%** to **-7.9%** of the
   converged answer, on BOTH sides. The earlier note here ("at a restrained dt
   ceiling the oscillations do not occur at all, which is the cheaper remedy")
   understates the cost of the restrained ceiling: ceiling 50 without control is
   24% wrong.

So the real trade is accuracy vs time, and the kernel changes it: the converged
`ref` run costs the interpreter **>1200 s** -- it actually aborted on this
model's own `maximum_time_allowed = 1200` (exit 5) when re-run under load --
against **61 s** for the generated solver. The kernel can afford the converged
solution the interpreter cannot. (The `ref` interpreter series used above is
therefore the earlier successful run; the interpreter is behaviourally unchanged
today, only env-gated logging was added, so the comparison is valid.)

**The qualitative conclusion stands**: oscillation control loosens
codegen-interpreter agreement by roughly an order of magnitude (-0.05% ->
-0.51%) and costs ~9-11x runtime on both sides, for a model where a restrained
dt ceiling avoids the oscillations entirely.

Two cross-checks worth keeping:
- BOTH step counts with control on reproduce the original entry exactly --
  interpreter **2069** (logged as ~2070) and codegen **1869** -- so nothing in
  today's fixes moved this model's trajectory on either side, and the original
  table is reproducible. (Interpreter steps counted with `OHQ_ITERLOG=1`.)
- With control off the two codes now agree to **one step out of 142**, which is
  the strongest parity evidence yet for the ported dt policy and Newton.

The absolute percentages are looser than the original entry (-0.010% / -0.116%).
The original `def` artifacts on disk turned out to predate the ISSUE 14/17 fixes
(the codegen side was from 10:34), so the two sets are not directly comparable;
these numbers are the ones measured with a single, current code state on both
sides. Timings vary +-30% with machine load (parallel builds were running).

Also confirmed: **ISSUE 18 (the X_norm==0 guard) never applied to this model** —
`rho_*` bulk densities start at 1402.5-1650, so the transport state norm was
never zero. And ISSUE 17's binding fix does not move a *forward* run at fixed
parameter values; it matters when a GA/MCMC drives them.

### What to decide later
Whether bit-parity under oscillation control is worth pursuing at all. Options:
make the detector hysteretic (require N consecutive detections), or accept that
control-on runs agree only to ~0.1% and document it. Note the control is
expensive on both sides — ~9x the runtime (see below) — and at a restrained dt
ceiling (`max_timestep_increase_factor` <= 10) the oscillations it exists to
suppress do not occur at all, which is the cheaper remedy for this model.

### Cost measured (8 columns, 4.5 d)
| | control OFF | control ON | ratio |
|---|---|---|---|
| interpreter | 12.7 s | 113.8 s | 9.0x |
| codegen | 0.25 s | 2.15 s | 8.6x |
| codegen speed-up | 51x | 53x | |

---

## ISSUE 12 — `System::CalcMisfit` writes into `fit_measures` without checking its size

**Status:** open, low priority (found 2026-09-14 while adding the `--kernel`
back end to OHQ-GA / OHQ-MCMC).
**Affects:** the interpreter, but only reachable from a host that scores a model
without going through `System::Solve`.

### Symptom
Segfault inside `System::CalcMisfit()`.

### Cause
`CalcMisfit` (System.cpp:3684) writes `fit_measures[3i]`, `[3i+1]`, `[3i+2]` for
every observation, but the vector is sized only in `System::InitializeSolver`
(1306), which `Solve()` calls. Any path that computes the objective without
having solved through `Solve()` — such as an alternative forward-model back end
— writes past the end of an empty `std::vector`.

### Workaround in place
`ohq::KernelSystem::Solve` resizes `fit_measures` itself before handing control
to the objective. Nothing in the library was changed.

### Suggested fix
Either resize defensively at the top of `CalcMisfit`, or make it use
`.at()`/`resize()` rather than assuming a prior `InitializeSolver`.

---

## ISSUE 13 — Three small additions to `System.h` for the codegen kernel back end

**Status:** informational (2026-09-14). No behaviour change; recorded so they
are not mistaken for accidental edits.

- `SetSolutionFailed(bool)` — the getter existed but `SolverTempVars` is
  private, so an external back end could not report a failed solve.
- `RestoreInterval()`, `RestorePointMaxUses()` — const getters for two private
  members the codegen needs to emit into generated solver settings.

All three are one-line const/trivial accessors next to the existing ones.

---

## ISSUE 14 — Codegen resolved reaction parameters before an object's own quantity

**Status:** FIXED 2026-09-14 (codegen side).

`Object::GetVal` (Object.cpp:99) checks the object's own quantity first and only
then falls back to a constituent or a reaction parameter. The codegen reaction
resolver checked `system.reactionparameter(name)` first. For a name that exists
in both places the two diverge silently.

Found while giving each column its measured porosity: the model has a
`porosity` ReactionParameter *and* a `porosity` quantity on every block. While
both were bound to one estimated parameter the values agreed and the bug was
invisible; with per-column porosity the interpreter would read each block's
value and codegen the single global one. Fixed by reordering to match
`Object::GetVal`; the 8-column regression is bit-identical.

---

## ISSUE 15 — OHQ-MCMC `--continue` built a doubled path, silently restarting

**Status:** FIXED 2026-09-14 (runner side, `tools/OHQ-MCMC/main.cpp`).

`CMCMC::SetParameters` already resolves `samples_filename` against the output
path (MCMC.hpp:151-153), so `FileInformation.outputfilename` carries the folder.
The runner prepended the working folder again, producing
`<wf>/<wf>/mcmc.txt`. That path never exists, so `--continue` reported
"no chain file ... starting a fresh run" and **overwrote the chain** -- the exact
failure mode the flag exists to prevent, and silent apart from one line of
output. Now the folder is prepended only when `outputfilename` carries no path.

## ISSUE 16 — MCMC resume segfaults on a chain with too few recorded samples

**Status:** open, low priority (found 2026-09-14).

After fixing ISSUE 15, resuming from a chain of 205 samples works correctly
(appends, 205 -> 209). Resuming from a 2-line chain -- produced by a 40-sample
run at `record_interval = 7`, i.e. essentially no usable samples -- segfaults
inside the resume path. Not a concern for production runs (20,000 samples), but
a short smoke-test run cannot be resumed, and the failure is a crash rather than
a diagnostic. Worth a guard on the recorded-sample count before restarting.

---

## ISSUE 17 — Codegen baked estimated parameters as literals unless the target was a block, link or source

**Status:** FIXED 2026-09-14. This one produced a wrong scientific result before
it was caught, so the detail is worth keeping.

### Symptom
A GA driven through `--kernel` reported a converged optimum over 10 parameters
whose fit was 2x worse than the run it started from, and worse on all 16
observations. The GA's reported best (nll -870.8) did not reproduce when the
same parameters were run forward in the interpreter (-621.3).

### Cause
`CodeGenerator.cpp` resolved `setasparameter` bindings with an allow-list:

```cpp
if (ot == object_type::block || ot == object_type::link || ot == object_type::source) { ...bind... }
else  paramNotes.push_back("... not a model quantity; the host applies it");
```

Anything else was baked into the emitted code as a **literal**. In the column
model every sorption parameter is a `ReactionParameter` (`alpha_*`, `Kd_*`), so
all eight were frozen at their start values:

```cpp
Col1_1__rxn_ads_sand_ = div((1.04983 * pos(rho_sand)) * 0.121749, porosity) * pos(Cu_aq);
//                            ^ alpha_sand literal      ^ Kd_sand literal
```

`setParameter()` / `applyParameters()` could not move them. The GA was in effect
optimising two parameters (dispersivity, and the host-applied sigma) while
believing it had ten, and it drove dispersivity to 1.93e-4 m -- grid Peclet 132,
which is what produced the violent oscillation at the amended lower ports.

The failure was silent in three separate ways: the skipped binding was recorded
only as a **comment** in the generated header, the console note looked benign,
and the runner's `--kernel` verification checked parameter **names and counts**,
not whether the kernel responds to them.

### Fix
1. Binding is now resolved generically for any object type whose target is a
   `value`/`constant` quantity; a constituent property additionally binds the
   per-block/link copies. Observation sigma and objective-function weights are
   the only host-owned exceptions, and are recognised explicitly.
2. Anything still unbound raises, rather than emitting a note.
3. **Generation-time guard:** after the code is assembled, every estimated
   parameter must appear as `params_[i]` somewhere in it, or generation fails:
   *"Refusing to emit a kernel a calibration cannot drive."*

### Refinement 2026-09-14 — inert bindings are allowed, undrivable parameters are not
The first version of the guard refused ANY unbound forward-model target, which
blocked the 8-column model: `porosity_all` is bound to three things —
`porosity.base_value`, `Col1-1.porosity` (both live `value` quantities) and
`Col1-1.moisture_content`, which on a *Groundwater cell* is the **expression**
`Storage/(depth*area)`.

Setting an expression quantity is inert in the interpreter too:
`ApplyParameters` calls `SetVal`, which writes the Quan's `past` value, while
every `present` read recomputes it from the expression (`Quan::GetVal`). So the
kernel NOT binding it preserves parity rather than breaking it.

The rule is now:
- `value`/`constant` target -> bound live (`params_[i]`).
- `expression`/`balance`/`rule` target -> recorded as an explicit `// NOTE ...
  deliberately not bound` line in the generated header, not silently dropped.
- observation sigma / objective-function weight -> host-owned, as before.
- **A parameter whose bindings are ALL inert still raises** ("has no binding the
  kernel can drive"), so ISSUE 17's guarantee is intact: the kernel is never
  emitted for a calibration it cannot drive.

On the 8-column model this yields 10 live parameters + `error_std` (host-owned),
with 160 inert `moisture_content` notes.

### Lesson for the runners — DONE 2026-09-14 (tasks C3/C4)
Name/count verification is not enough. Before trusting a kernel for a
calibration, perturb each parameter and confirm the output moves.

`ohq::Kernel::self_test()` (`codegen/tools/ohq_kernel.h`) now does this, and
`VerifyKernelMatches` calls it, so **every `--kernel` run checks it at startup**:

```
Kernel verified : 11 parameters, 16 observations, names match the model;
                  all 10 kernel-owned parameters move it (host-owned skipped).
```

Parameters the kernel legitimately does not own (an observation's
`error_standard_deviation` -- G9, applied by `System::ApplyParameters`) are
skipped by object type, not by a name heuristic.

The check is **bitwise**, deliberately: the kernel is deterministic, so a
parameter it reads changes some bit of the output, while one it ignores
reproduces the baseline exactly. A magnitude threshold is the wrong tool here --
in this very model a live sorption parameter moves the answer by 5e-4 against a
state vector dominated by ~1e9 of constant bulk-density mass, i.e. 1e-12
relative, which any sane threshold would call dead. (That mistake was made and
caught while writing the test.)


## ISSUE 18 — codegen: Newton skipped whenever the state norm was zero (FIXED 2026-09-14)

**Status:** fixed the day it was found. Recorded because it silently produced
zeros rather than failing, and because it invalidates measurements taken while
it was present.

### Symptom
Every constituent mass in the Wetland model was identically zero for the whole
run — `HRT` (an observation reading `AgeTracker_1:concentration`) was 0 at all
5999 output rows, while storages were correct to 6e-5. No error, no failed step:
`runTo` reported success.

### Cause
`newtonInterp()` in BOTH `codegen/runtime/ohq_transport.h` and
`ohq_massbalance.h` opened with

```cpp
const double X_norm = norm(state);
...
if (X_norm <= 0.0) return true;      // "nothing to solve"
```

The interpreter has no such guard (System.cpp:2431): with `X_norm == 0` its loop
condition `dx_norm / X_norm > 1e-10` is `1/0 = +inf`, so it iterates normally.
The guard was meant to avoid a division by zero, but it bails out of the solve
entirely — and it **latches**: a state that starts at zero is never advanced, so
it stays zero, so the guard fires again on every subsequent step.

Anything that legitimately starts at zero is affected: an age tracer, a tracer
breakthrough experiment (concentration 0 before the front arrives), or a flow
model whose blocks all start dry (a rainfall-runoff grid at t=0).

### Fix
Removed both guards; the loop now mirrors the interpreter exactly. `1/0` is
`+inf` in IEEE, not UB, and the iteration is bounded by `max_iterations`.
Verified: Wetland Cell 6 tracer mass 0 -> 402, HRT matches the interpreter to
1.5e-3 rms, and the G1/G2 gate passes again.

### Scope — which models are affected
Only a model whose ENTIRE transport state starts at zero. `norm()` is taken over
the whole constituent vector, so one non-zero constituent keeps the guard shut.

- **Wetland: hit.** A single constituent (`AgeTracker`) starting at 0 *is* the
  whole vector.
- **8-column study (ISSUE 11): NOT hit — verified.** `Cu_aq` and `Cu_s_*` start
  at 0, but the `rho_*` bulk densities start at 1402.5-1650, so `X_norm` was
  never 0 and the solve always ran. The ISSUE 11 figures do not need re-measuring
  on account of this bug. (They were re-measured anyway on 2026-09-14 because
  ISSUE 14 and ISSUE 17 do touch that model.)
- Watch for: a rainfall-runoff grid whose blocks all start dry — that is the
  flow-phase equivalent and would be hit.

---

## ISSUE 19 — codegen: a copied kernel solved the ORIGINAL's model (FIXED 2026-09-14, G7)

**Status:** fixed. Never observed in a result because nothing copied a kernel
yet — but the whole point of G7 is that MCMC copies one per chain.

### Symptom (by construction)
`MassBalanceSolver` / `TransportSolver` stored `Model& m_`, and the dt clamp
stored raw `const TimeSeries*` into the model's own members. Therefore, for
`Kernel chain = base;`

- `chain.solver_.m_` still referenced **base**, so every `computeFluxes` /
  `precomputeStep` / `computeTransportFluxes` read base's `params_`, cached
  flow locals and series while writing chain's state. `chain.setParameter(...)`
  would have had no effect on the solve.
- `chain.solver_.clampSeries_[k]` still pointed at **base's** series, so the dt
  clamp read another object's forcing — and dangled if base died first.
- A `Model&` member also makes the class **non-copy-assignable**, so
  `chains[k] = base;` did not compile at all.

### Fix
`Model& m_` -> `Model* m_` plus `rebind()` / `model()` in both solvers, a
`clearClampSeries()`, and a generated `bindSelf()` that re-points the solvers and
re-registers the clamp series at `*this`. `initialize()` calls it, and the public
`step()` self-heals (`if (solver_.model() != this) bindSelf();`) so a plain copy
is safe with no host cooperation.

### Verified
`copy_assignable` 0 -> 1; a copy's solver and clamp series point at itself;
copy-then-perturb is **bit-identical** to a freshly-initialised kernel with the
same parameter; the source kernel is untouched.

### Note on cost (the original G7 premise)
The roadmap assumed copying was expensive. Measured: 1.34 MB of baked forcing,
**121 us per copy** on Wetland, against a ~0.25-2 s solve per sample — i.e. well
under 0.1%. Sharing the immutable forcing (shared_ptr + copy-on-write in
`setSeries`) is therefore NOT worth the complexity; correctness was the real
issue. Revisit only if a model appears whose forcing dwarfs its solve time.


## ISSUE 18 — a stale kernel missing `ohq_kernel_step_to` loads and silently degrades

`KernelABI::valid()` (terminal/OHQ-Common/ohq_kernel.h:80) checks only
`lib && create && run_to && observation_at`. A kernel generated before
`ohq_kernel_step_to` was added therefore loads successfully; the missing symbol
is reported to stdout but is not an error, and `VerifyKernelMatches` passes
because parameter and observation names still match.

The call site guards correctly (`if (budget > 0 && k.step_to)`, line 240), so
the runner falls back to `run_to` — which has no wall-clock deadline. That is
exactly the MCMC hang fixed earlier: a stiff proposal runs forever instead of
being rejected on `maximum_simulation_time`.

Reproduced 2026-09-14 running OHQ-GA against `Two-site s13-GA/kernel3/build/libS13.so`.

Fix: either add `step_to` to `valid()` and refuse to load without it, or gate on
`ohq_kernel_abi_version()` and reject kernels below the version that introduced
it. The version symbol is already loaded and currently unused for anything.

---

## ISSUE 20 — kernel ABI v1 -> v2, and the duplicate `ohq_kernel.h` (2026-09-14)

**Status:** resolved same day. Recorded because a stale `.so` is otherwise a
silent-wrong-answer risk.

### What happened
`codegen/tools/ohq_kernel.h` was written as the canonical ABI declaration, and
while doing so the generator's alias ABI was extended from 25 to 40 entry points
(it was missing G4 forcing injection, G5 state in/out, solver status and
state/mass readback -- everything the twin needs beyond a plain GA run).

That extension was made **without bumping `ohq_kernel_abi_version()`**, so a
library built earlier still reported v1 while lacking the new symbols: a host
that bound them would have called a null pointer.

### Fix
- ABI bumped to **v2** in the generator and in `OHQ_KERNEL_ABI_VERSION`.
  `Kernel::load()` refuses a mismatched library up front:
  `libCol8.so: kernel ABI v1, host expects v2` -- verified against a stale build.
- `terminal/OHQ-Common/ohq_kernel.h` (the host-side G3/G9 integration) had its
  own copy of the ABI struct and dlsym loader. It now `#include`s the canonical
  header and keeps only what needs the OHQ core: `KernelSystem`, plus
  `VerifyKernelMatches`. One declaration list instead of three (generator,
  codegen header, host header).

### Rule going forward
The generator emits the ABI; `codegen/tools/ohq_kernel.h` declares it. Changing
one means changing both **and** bumping the version in both. Anything that links
the OHQ core gets the ABI through the canonical header, never its own copy.

---

## ISSUE 21 — G8 model-coverage survey across the deployments (2026-09-14)

Every deployment family was put through the generator, and where a usable
simulation window exists, through the interpreter-vs-kernel parity gate.

### Coverage
All 15 models generate. The large ones compile:

| model | size | compile | parity (final storages) | speed-up |
|---|---|---|---|---|
| Bioretention | 11 blk / 10 lnk | OK | **4.99e-10** | 443x |
| Reservoir | 2 / 1 | OK | **4.55e-16** | 545x |
| StormwaterPond | 5 / 3 | OK | 1.14e-03 (loosest) | 61x |
| JM | 26 / 34 | OK | **5.57e-06** | 1082x |
| R_simple | 36 / 34 | OK | **8.27e-11** | 318x |
| R_LF | 192 / 366 | OK | **1.27e-11** (1% perturbation) | 397x |
| R | 532 / 1026 | OK (235 s, 64k-line header) | not run | -- |
| HQ | 391 / 1064 | OK (133 s) | not run | -- |
| VN | 450 / 881 | OK (180 s) | not run | -- |
| Wetland, 8-column | -- | OK | see ISSUE 11 and the Wetland benchmark | 31-52x |

### Three real defects found and fixed
1. **Unresolvable bare quantity emitted as an undeclared symbol.** HQ and VN
   drywell-to-soil links read a bare `pressure_head` that lives on the soil
   *block*, not on the link. `Object::GetVal` (Object.cpp:99) has **no** fallback
   to a link's endpoints: it logs error 1002 and returns **0**. The generator was
   emitting `pressure_head_ /*UNRESOLVED*/`, which does not compile. It now emits
   `0.0` -- the parity-preserving answer -- and warns once per (object, quantity)
   so the case cannot hide. 20 such terms in HQ, 15 in VN. Same family as ISSUE 9.
2. **Dangling capture in the resolver** (introduced while fixing 1, caught by HQ):
   `makeCtx`'s `cur` parameter is captured by reference and dangles after it
   returns; using it in the emitted-warning path gave `std::bad_alloc` on HQ while
   the *larger* R model was fine. Use `t` (the resolved target). This is the
   second time this exact trap has been hit in this file -- there is now a comment
   at the site.
3. **The parity harness passed runs that never happened.** `R_LF` and a
   mis-windowed `R_simple` reported a glowing PASS with 0 steps: every observation
   empty, every storage still at its initial value. `parity_obs` now reports
   **INCONCLUSIVE** (exit 4) when `tend <= tstart` or the interpreter failed.
   R_LF's interpreter fails at a 10% parameter perturbation but solves at 1%.

### Two broken model files found (not codegen defects)
- **`Wetland_BSh_AM.ohq`** loads with **7 of its 13 links**. It carries template
  blocks from two machines, and the second `loadtemplate` RESETS the template set
  while its block omits `groundwater.json` -- so `surface2groundwater_link` is
  undefined, the six soil links are never created, and
  `Soil_Hydraulic_Conductivity` calibrates nothing. The interpreter reports this
  and runs anyway.
- **`HQ.ohq`** declares `Ks_12` and `new_Van_alpha` as estimated parameters with
  **no `setasparameter` binding at all** -- they affect neither code.

The generator now prints the model's own build errors before generating, since a
kernel built from a half-loaded model is a kernel for a different model.

### Parameter-binding rule, final form
`value`/`constant` target -> live `params_[i]`. Derived (expression/balance/rule)
target, or no usable binding at all -> recorded as a `// NOTE` in the header plus
a stderr warning, because the interpreter cannot drive it either, so ignoring it
preserves parity. A parameter whose bindings are ALL to objects that failed to
load is fatal. ISSUE 17's guarantee is intact: the kernel is never emitted for a
calibration it could silently ignore.


## ISSUE 19 — `CMCMC::readfromfile` assumes the chain file holds every sample; it does not

`readfromfile` (MCMC.hpp:845) packs rows sequentially, `Params[jj]` with `jj++`,
so it requires row *j* of `mcmc.txt` to be global sample *j*. Chain identity is
`k mod number_of_chains` (every step derives `Params[k]` from
`Params[k-nchains]`), so that assumption is load-bearing.

The file written by the s13 run is subsampled: sample numbers run
21, 28, 35, ... — a stride of 7, all 2744 gaps identical, 2745 rows spanning
20000 samples. Resuming from it therefore assigns row *j* to chain `j mod 16`
when its true chain is `(21+7j) mod 16`. **All 2745 rows are mis-assigned.** A
`--continue` would splice 16 chains together arbitrarily and silently, and would
also resume the counter at 2745 rather than 20000, re-running most of the range.

Reproduced 2026-09-15 on `Two-site s13-GA/mcmc2/mcmc.txt`.

Two things to fix, and they are independent:
1. `readfromfile` should key off the `no.` column rather than row order, and
   should refuse a file whose sample numbers are not contiguous from 0.
2. The stride is deliberate: `record_interval` (MCMC.hpp:119 ->
   `MCMC_Settings.save_interval`), set to 7 in this model. So the writer and the
   reader simply disagree — the writer honours `record_interval`, the reader
   assumes it is 1. Either `readfromfile` must divide by it, or the resume path
   must refuse a file written with `record_interval > 1`.

Note also that a thinned file silently breaks R-hat for anyone who treats
consecutive rows as consecutive samples of one chain; chain identity is
`sample number mod nchains`, which with a stride coprime to `nchains` scatters
each chain uniformly through the rows.

Until both are fixed, extend a chain by re-running with a larger
`number_of_samples`, not by `--continue`.

---

## ISSUE 20 -- `setvalue; object=system` without a prior `loadtemplate` fails with an empty error message

A script that uses only `addtemplate` builds every block and link correctly, but
each `setvalue; object=system, quantity=..., value=...` line is rejected.  The
console then prints

```
*** 1 error(s) while building the model:
Error:
*** refusing to solve.
```

-- one error, no text.  The count is right and the message is empty, so there is
nothing to search for and nothing to act on.  Bisecting the script down to a
single line is the only way to find it; a two-line script consisting of

```
setvalue; object=system, quantity=simulation_start_time, value=0
setvalue; object=system, quantity=simulation_end_time, value=20
```

reproduces it, and prefixing `loadtemplate; filename = <resources>/main_components.json`
fixes it.  `loadtemplate` installs the system template; `addtemplate` only
appends to it, so with `addtemplate` alone the system object has no quantities
and every assignment to it misses.

Two separate defects:

1. the error carries no message.  Whatever raises it should say which quantity
   it could not set, on which object.
2. assigning to `object=system` when no system template is loaded is a
   configuration mistake that should be reported as such, once, rather than as
   N anonymous errors.

Every example under `Examples/` happens to start with a `loadtemplate` line, so
the failure never shows up there.  Scripts written by hand or emitted by a
generator routinely start with `addtemplate`.

---

## ISSUE 21 -- `initial_time_step` silently sets the resampling resolution of every time series

`System::InitializeSolver` calls `MakeTimeSeriesUniform(SimulationParameters.dt0)`
(System.cpp:1333), which replaces every time series on every block, link and
source with `make_uniform(dt0)`.  The number of points materialised is therefore
`record_span / dt0`, and `dt0` is a *solver* setting that a user picks for
stability, with no reason to think it controls memory.

A 155-day inflow record at the `dt0 = 1e-5` d that the transport models in this
project use expands a 141-point file to 1.6e7 points.  The run then hangs in
`TimeSeries::assign_D` before the first step: no output, no error, no progress
line, and `maximum_time_allowed` never fires because the solve has not started.
Bisecting the model does not find it either -- removing blocks does not help,
because the cost is per time series, not per block.

Measured on `flow_PA1.txt` (141 points, 155.08 d, 0.875967 m^3):

| dt0    | points     | volume   | error  |
|--------|------------|----------|--------|
| 1e-2   | 15,510     | 0.916330 | +4.61% |
| 1e-3   | 155,083    | 0.879775 | +0.43% |
| 1e-4   | 1,550,814  | 0.876319 | +0.04% |
| 1e-5   | 15,508,128 | 0.875972 | +0.00% |

So the resampling is also lossy, and one-sided: linear interpolation across a
step smears each edge into a wider trapezoid, which *adds* volume.  The user
pays memory and startup time for an operation that degrades the input.

What it buys is the fast path in `interpol` (TimeSeries.hpp:644): a uniform
series indexes by arithmetic, an unstructured one falls through to a **linear
scan** (TimeSeries.hpp:657) -- not a bisection.  That is worth having for a
large series, but it does not justify tying the resolution to `dt0`.

Suggested fixes, in order of preference:

1. give time-series resampling its own setting, defaulted to something sane, and
   leave `dt0` to the solver;
2. cap the point count (`make_uniform` could refuse to expand a series beyond,
   say, 1e6 points and raise instead of hanging);
3. replace the unstructured `interpol` fallback with a binary search, so an
   unmodified series costs `O(log n)` and resampling becomes optional.

Workaround: keep `dt0` no smaller than ~1e-3 d when any time series is attached,
and place the series' breakpoints on multiples of `dt0` -- then resampling
reproduces the file exactly (verified: 0.000% volume change).

---

## ISSUE 22 -- `Time-Dependent flow` hardcodes link length = 1 m and area = 1e-9, breaking dispersion and disabling diffusion entirely

`Constituent::diffusive_masstransfer` (main_components.json) is

```
(diffusion_coefficient*area + dispersivity*flow)/length * (concentration.s - concentration.e)
```

so both transverse terms are divided by the LINK's `length` and the diffusive
one is scaled by the LINK's `area`. The `Time-Dependent flow` link
(mass_transfer.json) defines both as fixed expressions the user cannot set:

```
length : {"type": "expression", "expression": "1",           "ask_user": "false"}
area   : {"type": "expression", "expression": "0.000000001", "ask_user": "false"}
```

Two independent consequences.

**1. Molecular diffusion can never contribute.** The diffusive term is
multiplied by 1e-9 m^2 regardless of the `diffusion_coefficient` the user sets
on the constituent. Setting a physically correct D changes nothing, silently.
A `soil_to_soil_link` carries the real area (0.00811 m^2 for the column study),
so the same constituent diffuses in one link type and not in the other.

**2. Dispersion is divided by 1 m instead of the real cell spacing.** For the
column study the cells are 0.0254 m apart, so the dispersive exchange in a
`Time-Dependent flow` chain is **39.4x too weak**. The parameter is still live
-- it is bound correctly (`setasparameter; object= Cu_aq, parametername=
dispersivity_all, quantity= dispersivity`) -- it just barely moves the answer.

Measured on the 8-column study (18 cells, 0.0254 m, PA1 lower port at 400 pore
volumes), varying dispersivity over 5e8:

| dispersivity | `Time-Dependent flow` | `soil_to_soil_link` |
|--------------|----------------------|---------------------|
| ~0           | 0.01976              | 0.01667             |
| 0.0485       | 0.01968  (-0.4%)     | 0.00457  (-73%)     |
| 0.5          | 0.01771  (-10%)      | 0.00099  (-94%)     |

The same nominal parameter has ~100x more leverage in the soil chain. Rebuilding
the identical column on `Soil` blocks and rescaling the calibrated dispersivity
by the length ratio (0.0485256 x 0.0254 = 0.00123) reproduces the
`Time-Dependent flow` result to within 6% at 800 PV -- confirming the length is
the whole discrepancy.

**Why this matters beyond one model.** A dispersivity calibrated against a
`Time-Dependent flow` chain absorbs the wrong length: the fitted 0.0485 m is
1.9x the cell spacing and 39x the physical value it stands for. Carrying that
number into any correctly-dimensioned model over-disperses by the same factor.
Any GA/MCMC estimate of dispersivity on such a chain is also weakly identified,
because the likelihood is nearly flat in it.

Suggested fix: make `length` and `area` real, user-settable properties of
`Time-Dependent flow` as they are on every other transport link, defaulting to
the connected blocks' geometry rather than to 1 and 1e-9. Existing models that
relied on the old behaviour will need their dispersivity rescaled by the link
length; that is a breaking change and should be called out, but leaving it is
worse -- the current defaults silently mean "no diffusion, and dispersion in
units of per-metre".

---

## ISSUE 23: a `setvalue` naming an unknown system property is silently discarded

`setvalue; object=system, quantity=covariance_proposal, value=Yes` had no
effect. The run proceeded with the covariance-adapted proposal switched off and
reported nothing -- no warning, no error, and the samples file looked normal.
The cost was a 20,000-sample MCMC whose whole purpose was to measure what that
setting does, and an A/B comparison that looked valid and was not: it compared
the diagonal sampler against itself.

**Cause.** The binary in use predated the setting. `covariance_proposal` was
added 2026-09-17 11:54; the study was driven by an `OHQ-MCMC` built 2026-09-14,
which has no such property. `System::SetSystemSettingsObjectProperties` did not
find it in any settings object, appended error 631 to a log nobody reads, and
returned false. The quantity was left empty -- not even its declared default of
"No" -- and `CMCMC<T>::SetProperty` never saw it.

| binary | knows `covariance_proposal` |
|---|---|
| `tools/OHQ-MCMC/build-release/OHQ-MCMC` (09-14) | no |
| `tools/OHQ-MCMC/build-quiet/OHQ-MCMC` (09-05)   | no |
| `terminal/OHQ-MCMC/OHQ-MCMC` (09-17 and later)  | yes |

Confirmed from the samples alone, independently of `state.json`:

- Proposal step sizes tracked `pertcoeff`, not the posterior standard deviation
  (CV of jump/pertcoeff 0.16 against CV of jump/posterior-sd 0.86) -- the
  signature of the diagonal branch. The known-diagonal run gave 0.14 / 0.84.
- Mean absolute cross-parameter correlation of accepted jumps was 0.033, against
  0.032 for the diagonal run, although the posterior carries a strong ridge
  (`p_alpha_sand` x `p_Kd_sand`, r = -0.88; covariance condition number ~3900).
- Both runs followed an identical acceptance-rate adaptation schedule
  (pertcoeff 0.27607 -> 0.08735 = 0.75^4 in each).

Note what is NOT wrong here, because two plausible explanations were checked and
rejected. The parser (MCMC.hpp:187) and `UpdateProposalCovariance` are both
correct. And the fault is not specific to `type: string` quantities: the
Yes/No setting `initial_purturbation`, declared identically, arrives intact.

**Fixed** in this tree:

1. `System::SetSystemSettingsObjectProperties` now prints
   `*** setvalue ignored: system has no property '<name>' (value '<v>' discarded)`
   on stdout instead of only appending to the error handler. Same class of
   defect as ISSUE 20 -- a `setvalue` that fails should never be silent.
2. `CMCMC<T>::Perform()` now echoes the sampler's actual configuration before
   sampling starts -- chains, samples, burn-in, and whether the proposal is
   COVARIANCE-ADAPTED or DIAGONAL. A run can no longer silently be something
   other than what the script asked for. The pre-existing
   `[covariance proposal refreshed at sample N]` message only appears once the
   feature is already working, so it could not have caught this.

Verified after rebuilding: the echo reports COVARIANCE-ADAPTED, no `setvalue
ignored` lines appear, and the covariance refreshes on schedule.

**Wider point.** The GA and MCMC drivers are separate executables from
`OpenHydroQual-Console`, and the console does not even link the MCMC objects.
Rebuilding one does not rebuild the others, and a stale copy of any of them
accepts a current script while quietly ignoring whatever is newer than itself.
A version or build stamp in each driver's banner would make that visible.
