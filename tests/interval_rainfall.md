# Native precipitation interval regression

Native precipitation records are **start day, end day, total depth**. They are
not point rates. The loader previously converted each bin to its midpoint rate
and interpolated between midpoints. A 2 mm pulse in unequal-width bins delivered
3 mm; rain was also spread outside the storm. This fix intentionally changes
that erroneous forcing behavior; it is not a calibration or a backwards
compatibility mode for reproducing the old error. Frozen outputs stay untouched.

`IntervalRainfall.h` constructs bounded step-rate knots (transition width at most
floating-point spacing) and integrates over the actual solver step. `Quan` uses
that average for native precipitation only. It preserves ordinary time-series
interpolation. Native precipitation is excluded from optional point resampling;
otherwise resampling would erase its interval edges. Source receiver-area and
SI-unit conversion paths are unchanged. No public class layout was enlarged.

The code generator uses the same interval construction and step averaging,
including its runtime `setPrecipitation` API; native series are not point-resampled
while being baked. Embedded runtime changes are scoped to the new helper: the
repository's pre-existing unrelated makeUniform header/embedded drift was not
silently synchronized. No unrelated output-resampling behavior changes here.

Validation rejects nonfinite/negative depths, invalid widths, overlapping or
out-of-order bins, incomplete numeric tokens and extra columns. Gaps mean zero
rain; a single bin has bounded support. Programmatic malformed CPrecipitation
arrays throw invalid_argument; the file loader reports an input error and the
kernel injection API returns false without deleting the previous forcing.
The separate getflow(A, dt) point-sampler API is unchanged; native loading and
programmatic CPrecipitation injection use getflow(A).

## Tests

```
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation \
  -Iaquifolium/include -Icodegen/runtime tests/interval_rainfall.cpp \
  -o /tmp/interval_rainfall_test
/tmp/interval_rainfall_test
```

The pure C++ test compares interpreter helper and codegen runtime across unequal
bins, gaps, single intervals, steps crossing multiple bins and rejected domains.
It passed plain and ASan/UBSan builds. LeakSanitizer cannot run in the sandbox's
ptrace environment; the address/undefined checks passed with detect_leaks=0.

A fresh full source-pinned interpreter build and actual code-generator build were
made in /tmp. `tests/generated_interval_rainfall.cpp` compiles against a generated
IntervalRainModel.h and its exported runtime. Baked and runtime-injected variants
both finish at12m³ (initial10 + rain2), matching the real interpreter. Malformed
kernel injection preserves the old forcing. This is real generation/compilation/
execution, not just a test of a hand-written generated-model imitation.

Additional source-pinned TimeSeries singleton/general interpolation regression
passes. Native-loader checks: three valid rainfall encodings, seven rejection
cases, including a missing start token. The small real solver uses0.007-day
steps (10.08minutes), deliberately straddling the pulse edges. Exactly2 or3m³ is
received as specified, storage change matches, and fully dry intervals stay dry.
Three prior constant-rain/area-swap/loss ledgers still pass.

GIStoOHQ's saved scripts run all four full generated historical formulations with
both equal and unequal interval encodings. All eight rainfall-volume and whole-
domain ledgers pass using nr_block_tolerance=1e-8 in isolated copies. All receive
47337.6m³; the largest residual is about6.92e-6m³ against the retained0.0573376m³
criterion. This does NOT establish all default numerical tolerances, arbitrary
long-storm/ET/limiting behavior, per-HRU conservation, research skill or HMS solver
acceptance. Historical model area parameter sum remains23.6688km²; no replacement
with29.2km² or the new HydroPINN gauge area was made.

Saved evidence and binaries:
/home/arash/Dropbox/HydroPINN paper/audits/ohq_interval_rainfall_fix_20261119/.
Earlier failing event receipts are preserved in ohq_generated_event_20261119.
The final missing-token parser guard is a separate hash-bound Precipitation.o
variant of the frozen build; three valid +seven invalid native tests use that
final variant. The eight full event runs use the same physics/interval code with
the preceding parser version; only malformed empty-start-row rejection differs.
Original console main/user files and installed binaries are preserved. No
research training, observations, reserved targets or manuscript edits.

## Broader compatibility follow-up (19 November2026)

The broader review found TWO regressions in d7a45cb; both are fixed in the
follow-up, rather than treating the original narrow checks as comprehensive:

1. Numeric validation used aquiutils::trim (spaces only), rejecting valid CRLF
   rainfall. Local token trimming now accepts standard CSV whitespace without
   relaxing full numeric consumption or overlap/negative-depth rejection.
   The original 2010 plant rainfall file is accepted byte-for-byte; no input
   normalization or resource change was used to make the test pass.
2. Generated precipitation averaging used solver.dt(), which is the adaptive
   BASE timestep. A clamp can take a smaller APPLIED timestep. The new accessor
   appliedDt() exposes the attempted step already used in the finite-volume
   residual. Only native-rain averaging switches to it; the public base-step
   API/adaptation policy stays unchanged. A saved closed-catchment counterexample
   previously receives0.2m³ before the storm and now matches the independent
   volume solution at EVERY accepted step (maximum error2.63e-11m³).

Actual generator compilation/execution across eight broader example families:
6parity passes (tank, parallel links, rule reservoir, snowmelt, HRU and Penman
plant-transpiration), using separate0.5%/block tolerances rather than allowing
an artificial fixed-head storage to dominate the error scale. Urban and crop
kernels have missing intermediate-symbol compile errors; rebuilding with the
pre-d7a45cb generator reproduces the exact same symbol sets. These are existing
codegen limitations, not passing or newly introduced failures. Their interpreter
examples solve. Snowmelt parity's pre-fix failure is preserved; it passes after
using appliedDt, as does the pre-change midpoint-generator baseline.

13bounded interpreter example copies pass after the CRLF correction.9outputs
are byte-identical before/after; the4rain-affected examples change as expected.
Actual nonzero ET/uptake is checked in5families, not inferred from a successful
exit. Four analytical heat/mass-transport tests pass in both builds and all4
output tables are byte-identical. Five existing groundwater-resource invariant
cases pass in both builds (including nonzero downward and upward flux cases).
Existing committed fixture models were copied; their nonportable generator
which refers to external research data was NEVER run.

12independent interval-forcing cases cover unit selection, uniformization on/off,
small/large steps, growth/clamping, and a nonzero calendar epoch. Existing XString
unit coefficients are float32: the millimetre case's9.5e-8m³ rounding is recorded
separately and unchanged, not disguised as an interval-volume error. Original
strict-unit failed attempt remains saved; the revised oracle accounts for the
existing coefficient precision without loosening interval integration tolerance.
10C++ native-file checks additionally cover CRLF, padded tokens, blank lines and
comments.628GIStoOHQ tests pass with4existing warnings. No full GUI/installed
application/all-model guarantee or research-skill claim follows from this audit.

All originals/previous failed evidence retained under the broad audit:
/home/arash/Dropbox/HydroPINN paper/audits/ohq_broad_compatibility_20261119/.
The first adaptive fixture did not expose the bug; the stronger forcing/end-time
clamp regression does. No fitted parameters, research training, reserved targets,
original manuscript, installed binary or user console-main changes.
