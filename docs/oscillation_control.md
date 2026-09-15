# Oscillation control

*Solver feature notes — detection, remedies, measured behaviour, and the
defects found while building it.*

## 1. The problem

A converged step can still be wrong. When the time step is coarse relative to
the fastest process in the model — a sorption reaction with a large rate
constant, for instance — the scheme can oscillate: the state rises and falls on
alternate steps instead of advancing. Every step converges, no error is raised,
the run reports success, and the results contain excursions that cannot be
physical. In the mixed-media column models this shows up as negative aqueous
concentrations at the outflow ports.

Nothing in the solver noticed this before. Newton convergence is a statement
about the residual at one step; it says nothing about whether the sequence of
steps is tracking the solution. Oscillation control adds that check.

It is **off by default**. Some models genuinely oscillate, and only the modeller
can say whether an alternation is a numerical artefact or the answer.

## 2. Detection

After each accepted step the solver gathers the state vector that was actually
solved for — every variable in `solvevariableorder`, then, if the model has
constituents, the transport unknowns:

```cpp
std::vector<double> System::GatherSolvedState(const Expression::timing &tmg)
```

The transport unknowns are addressed by the quantity name `"mass"`, exactly as
`OneStepSolve` does. This matters: passing a `solvevariableorder` entry here
asks for `"<constituent>:Storage"`, which does not exist. (See §7.1.)

Checking the *state* rather than the outputs is deliberate. The outputs are a
much larger set, most of it derived, and they are uniformized to `dt0` when the
run ends — so an oscillation at the step scale may not even be visible in them.
The solved state is the smallest vector that can show the problem.

`CountOscillatingStates(tol)` keeps the last four accepted state vectors and
applies the same criterion as `TimeSeries::wiggle_sl`, one variable at a time:

```cpp
const double scale = (fabs(c1[i]) + fabs(c2[i]) + fabs(c3[i]) + fabs(c4[i])) / 4.0
                   + tol / 100.0;
const double d1 = (c1[i] - c2[i]) / scale;
const double d2 = (c2[i] - c3[i]) / scale;
const double d3 = (c3[i] - c4[i]) / scale;
const bool all_small   = fabs(d1) < tol && fabs(d2) < tol && fabs(d3) < tol;
const bool alternating = (d1 * d2 < 0) && (d2 * d3 < 0);
if (!all_small && alternating) count++;
```

Both conditions are needed. The alternating-sign pattern alone is satisfied by
round-off in any variable sitting at a steady value; the magnitude alone is
satisfied by any rapidly changing but perfectly healthy solution. A variable is
flagged only when it reverses direction twice in succession *and* the movement
is a significant fraction of its own magnitude. The steps are equal in a
four-step window, so the increments stand in for slopes.

The `tol / 100.0` term in `scale` keeps a variable that is identically zero from
dividing by zero, and sets the amplitude below which nothing is ever flagged.

There is a **four-step cooldown** after any intervention, and the history is
cleared when the state jumps backwards, so a single episode cannot be counted
repeatedly while the solver is still recovering from it.

## 3. Remedies

When at least one variable is flagged and the intervention budget is not spent,
one of two things happens.

### `rewind` (default)

Calls `ResetBasedOnRestorePoint`, which:

- restores the whole model state from the most recent restore point,
- restarts at **one fifth** of the step that was in force there,
- imposes a ceiling on the step at that value,
- and **knocks the recorded samples out of the output** past the restore point,
  for both all-outputs and observations.

The last point is what distinguishes rewind: the oscillation is removed from the
delivered results rather than merely stopped. Simply slowing down from the
current point would leave the wobble already written in the output.

The cost is that everything since the last restore point is recomputed, which is
why `restore_interval` matters so much to this remedy (§6).

### `reduce`

Keeps the current state and continues with `dt_base / 5`, holding the ceiling
there. Much cheaper — nothing is recomputed — but anything already written
stays. Choose it when run time matters more than the recorded results, or when
the oscillation is mild enough that you only want to stop it spreading.

### The ceiling, and why it exists

Both remedies set `SolverTempVars.dt_ceiling`. Without it neither one does
anything: `dt_base` recovers by roughly `1/0.75` per successful step and is back
where it started within about ten steps, at which point the oscillation returns.
The reduction on its own buys nothing. This was one of the more expensive
lessons of building the feature (§7.3).

The ceiling is applied at the top of the main loop:

```cpp
if (SolverTempVars.dt_ceiling > 0)
    SolverTempVars.dt_base = min(SolverTempVars.dt_base, SolverTempVars.dt_ceiling);
```

### Relaxation

Holding the step down for the remainder of a long run usually costs far more
than the oscillation episode itself, which is typically transient — the solver
passes the awkward point and the rest of the run is well behaved. So after
`oscillation_relax_after` consecutive untroubled steps the ceiling **doubles**,
one unit of intervention budget is refunded, and the count restarts. Once the
ceiling reaches `dt0 × max_timestep_increase_factor` it is dropped entirely and
the step is unconstrained again. A recurrence simply re-imposes it.

Relaxation is applied to a ceiling imposed by **any** rewind, not only an
oscillation rewind — a rewind after repeated Newton failure gets the same
treatment, for the same reason.

Set `oscillation_relax_after` to 0 to hold the ceiling for the rest of the run.

### Giving up

If the budget is exhausted, or the restore point has been used
`restore_point_max_uses` times, the solver stops intervening and records:

> *oscillation in N state variable(s) could not be removed by rewinding and
> refining … The results contain oscillation; treat them with caution.*

It deliberately does not grind the step down further. Past a point, a very small
step stops Newton iterating at all (§4) and the cure is worse than the disease.
The message is a signal to change `restore_interval`, `restore_point_max_uses`,
or the model — not to let the solver keep trying.

At the end of the run a one-line summary is written:

```
oscillation control: 12 intervention(s) (reduce), 30 relaxation(s); final dt = 0.0439309
```

## 4. Related: the null-solution guard

While building this a second silent failure came to light, and it is worth
recording here because the two are easily confused.

Newton can stop solving without failing. The iteration is guarded by an
**absolute** residual test — `err > 1e-12` — so when the initial residual starts
below that floor the loop is skipped entirely and the step is accepted
unchanged. That is correct for a system genuinely at rest. It is fatal when mass
is crossing the boundary: every step is accepted unchanged, the run reports
success, and the returned field is identically zero.

The guard triggers on the **mechanism**, not the symptom. Counting zeros would
only catch it much later, and would also flag legitimately empty models. Instead
the solver watches for Newton being skipped (`nr_below_absolute_floor`
increasing) while the transport inflow loading is non-zero. After 20 such
occurrences it raises a fail reason and stops:

> *the Newton iteration is being skipped — the initial residual is below the
> absolute floor (1e-12) while mass is entering the domain … The time step is
> too small for the scale of this problem: raise `initial_time_step`, or rescale
> the state variables.*

Note the interaction: driving the step down far enough — which an unbounded
oscillation response would do — walks straight into this. That is the concrete
reason the give-up path exists.

Note also that `nr_tolerance` is an **absolute** tolerance. Setting it very
small does not make the solve more accurate below the 1e-12 floor; it makes the
run slow and, in a badly scaled model, can put you in this regime.

## 5. Settings

All are on the **Solver Settings** page. GUI labels first, script names in
parentheses.

| GUI label | Script name | Default |
|---|---|---|
| Detect and suppress solution oscillation | `oscillation_control` | No |
| Oscillation detection tolerance | `oscillation_tolerance` | 0.01 |
| Response to detected oscillation | `oscillation_remedy` | rewind |
| Clean steps before relaxing the time-step ceiling | `oscillation_relax_after` | 40 |
| Maximum oscillation interventions | `oscillation_max_reductions` | 4 |
| Time steps between restore points | `restore_interval` | 200 |
| Maximum reuses of one restore point | `restore_point_max_uses` | 2 |

`restore_interval` and `restore_point_max_uses` are not oscillation settings as
such — they govern the restore-point machinery the solver has always had, used
after repeated Newton failure too — but they were exposed for this feature
because the `rewind` remedy is unusable without control over them.

A script line looks like:

```
setvalue; object=system, quantity=oscillation_control, value=Yes
setvalue; object=system, quantity=oscillation_remedy, value=reduce
setvalue; object=system, quantity=oscillation_tolerance, value=0.01
```

## 6. Measured behaviour

Benchmark: the eight-column Cu breakthrough model from the column study
(8 columns × 2 ports, two-site sorption, three amendments), calibrated
parameters, `initial_time_step = 0.001`, `maximum_time_allowed = 1200 s`, on a
Ryzen 7 5700G. Exit 4 means the run completed (the 8 recorded errors are a
pre-existing property lookup unrelated to the solver); exit 5 means the solve
failed.

### Remedies

| Configuration | Wall | Interventions | Relaxations | Final `dt` | Negatives | Outcome |
|---|---|---|---|---|---|---|
| control off | 94 s | — | — | — | 6536 | completes; oscillation present |
| `reduce` | 336 s | 12 | 30 | 0.0439 | 3752 | completes |
| `rewind`, interval 200, max uses 2 | 1206 s | 23 | 111 | 0.000597 | — | **fails** — exceeded the 1200 s limit |
| `rewind`, interval 25, max uses 4 | 494 s | 11 | 36 | 0.05 | **940** | completes |

*Negatives* counts negative values across every `:concentration` column of the
written output — a direct symptom of the oscillation, since none of these
quantities can be negative physically.

On that measure the two remedies are not equivalent: `rewind` removes 86% of the
negatives, `reduce` 43%. That gap is by design. `reduce` stops the oscillation
spreading but leaves in the output everything already written; `rewind` knocks
those samples out. If the delivered results are what matter, this is the number
to look at, and it is worth the extra 158 s.

The failing case is the instructive one. With restore points 200 steps apart,
each rewind discards up to 200 steps of work and the solver spends the run
recomputing; worse, it repeatedly returns to the *same distant* snapshot, cannot
get past the trouble from there, and grinds the step down to 6e-4. Making
snapshots 25 steps apart — cheaper to reach, closer to the problem, and taken
often enough that each new one is fresh — turns the same model from a failure
into a 494 s success that ends at the largest step of any configuration.

**If `rewind` reports that it gave up, reduce `restore_interval` before anything
else.**

### Detection tolerance

Same model, remedy `reduce`, varying `oscillation_tolerance` (walls include some
machine-load variation; the `reduce` row above and the 0.01 row here are the
same configuration):

| `oscillation_tolerance` | Wall | Interventions | Relaxations | Final `dt` | Negatives |
|---|---|---|---|---|---|
| 1e-9 | 1205 s | 48 | 108 | 0.000214 | — (**failed**) |
| 0.003 | 523 s | 19 | 46 | 0.0369 | 2686 |
| 0.01 (default) | 400 s | 12 | 30 | 0.0390 | 3825 |
| 0.03 | 173 s | 5 | 13 | 0.0349 | 7256 |
| 0.10 | 114 s | 0 | — | guard never fires | — |
| 5.0 | 85 s | 0 | — | guard never fires | 6536 |

The two extremes bracket the useful range and each fails in its own way. At
1e-9 the magnitude gate is effectively disabled, every sign alternation counts,
and the solver intervenes 48 times, grinding the step to 2e-4 until the run
exceeds `maximum_time_allowed` and fails outright — the same end state as
`rewind` with sparse restore points, reached from the opposite direction. At 5.0
nothing is ever flagged and the output is identical to `off`, negative for
negative (6536 either way): proof that the guard is genuinely inert there rather
than quietly doing something small.

Detection responds monotonically: a tighter tolerance flags more variables, so
there are more interventions and the run costs more, roughly linearly. Above
about 0.05 the magnitude gate is wide enough that this model's oscillations no
longer qualify, the guard is inert, and the run costs the same as `off`. The
default of 0.01 sits where the guard is active without being hair-trigger.

**Do not set this very small in the hope of catching more.** A tolerance below
roughly 1e-3 stops discriminating between oscillation and round-off, and the
resulting stream of interventions drives the step down until the run is slower
than the problem it is solving.

The negatives column carries a warning. It improves as the tolerance tightens,
but at 0.03 it is *worse than no control at all* (7256 against 6536). This is
consistent with how `reduce` works rather than surprising: it does not remove
anything already written, it only changes the trajectory from the point of
intervention on, and a few late, widely spaced interventions can leave the run
stepping through the difficult region on a different and no better path. It is a
concrete argument for `rewind` when the output matters, and against treating a
lax tolerance with `reduce` as a cheap partial fix. (The 0.01 row here and the
`reduce` row above are nominally the same configuration; they differ slightly —
3825 against 3752, final `dt` 0.0390 against 0.0439 — because the two model
files were generated separately and are not identical in every solver setting.)

## 7. Development notes

Four defects were found and fixed during development. They are recorded because
each is easy to reintroduce.

**7.1 Wrong state vector.** The first version gathered
`GetStateVariables(solvevariableorder[i], …)` for the transport unknowns too,
which asks for `"<constituent>:Storage"` — a quantity that does not exist. The
transport unknowns must be addressed as `"mass"`, as `OneStepSolve` does.

**7.2 Empty setting values destroying defaults.** `SetSystemSettings()` replays
*every* registered setting, including ones the model never mentions, whose
stored value is the empty string. `atof("")` is 0, so an unmentioned
`oscillation_tolerance` silently became 0 — and a zero tolerance makes every
sign alternation count as oscillation, since `all_small` can never be true.
Every numeric setting in this group is now guarded:

```cpp
if (!aquiutils::trim(val).empty())
    SolverSettings.oscillation_tolerance = aquiutils::atof(val);
```

This defect invalidated an entire round of A/B testing before it was found; any
conclusion drawn from a run whose settings were being zeroed is worthless.

**7.3 Step regrowth undoing every intervention.** Reducing `dt` without also
setting a ceiling accomplishes nothing, because `dt_base` climbs back within
about ten successful steps. This is the reason `dt_ceiling` exists.

**7.4 Budget counter doubling as event counter.** Relaxation refunds a unit of
intervention budget. A single counter used both to enforce the budget and to
report activity therefore read zero in the end-of-run summary on runs that had
intervened a dozen times. They are now separate: `osc_reductions` is the
refundable budget, `osc_events` counts interventions and is never refunded.

A fifth, caught late: generalizing the ceiling logic into
`ResetBasedOnRestorePoint` removed it from the `reduce` path, which does not go
through that function. The `reduce` branch now sets `dt_ceiling` explicitly, and
the comment there says why.

## 8. Limitations

- The detector reports *that* a variable is oscillating, not why. It does not
  attribute a cause and cannot tell a numerical artefact from a model that
  oscillates for real. That judgement stays with the modeller, which is why the
  feature is off by default.
- The check runs on every accepted step and costs O(number of state variables).
  Measured overhead on the column models, guard active but not intervening, is
  about 1.35×. Runs where it intervenes cost considerably more (§6) — that is
  the remedy, not the detector.
- Four accepted states are needed before anything can be flagged, and the
  history is cleared after every intervention, so there is an unavoidable
  detection latency of a few steps.
- `rewind` can only rewind as far as the most recent restore point. An
  oscillation that begins immediately after a snapshot and is only detected
  several steps later cannot be fully removed from the output.
- Only oscillation in the *solved state* is detected. A derived output that
  oscillates while its state variables do not will not be caught.
