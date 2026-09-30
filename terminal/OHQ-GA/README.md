# OHQ-GA — genetic-algorithm calibration from the command line

`OHQ-GA` does what the GUI's **Optimize** button does, without Qt Widgets: it reads an `.ohq`
model, varies the model's `parameter` entries within their bounds, scores each parameter set
against the model's `observation` entries, and writes the population of every generation. It can
run the forward model two ways:

* **interpreter** (default) — `System::Solve()`, the same code path as the GUI;
* **codegen kernel** (`--kernel`) — a shared library compiled from the model by `ohq_generate`,
  typically 10–20× faster. For any calibration longer than a few minutes this is the way to run.

This file is a step-by-step guide to calibrating with the kernel. The shared reference for both
headless runners (OHQ-GA and OHQ-MCMC), including the kernel ABI, is
[`../OHQ-Common/README.md`](../OHQ-Common/README.md).

---

## Contents

1. [Build the tools (once)](#1-build-the-tools-once)
2. [Prepare the model](#2-prepare-the-model)
3. [Generate the kernel](#3-generate-the-kernel)
4. [Build the kernel](#4-build-the-kernel)
5. [Check the kernel with one forward run (optional)](#5-check-the-kernel-with-one-forward-run-optional)
6. [Run the GA](#6-run-the-ga)
7. [Monitor the run](#7-monitor-the-run)
8. [Results](#8-results)
9. [Continue a run](#9-continue-a-run)
10. [What needs a new kernel, and what does not](#10-what-needs-a-new-kernel-and-what-does-not)
11. [Memory and run time](#11-memory-and-run-time)
12. [Troubleshooting](#12-troubleshooting)
13. [A complete script](#13-a-complete-script)

---

## 1. Build the tools (once)

Dependencies (Ubuntu):

```bash
sudo apt-get install build-essential cmake qt6-base-dev libarmadillo-dev liblapack-dev \
                     libblas-dev libgsl-dev libopenblas-dev libsuperlu-dev
```

**The runner** (from this folder):

```bash
mkdir -p build && cd build
qmake6 ../OHQ-GA.pro
make -j$(nproc)
```

This produces `build/OHQ-GA`. It compiles the aquifolium sources directly, so after any change to
an `aquifolium/include/` header, rebuild it (`make` again).

**The code generator** (from the repository root):

```bash
cmake -S codegen -B codegen/build -DCMAKE_BUILD_TYPE=Release
cmake --build codegen/build -j$(nproc)
```

This produces `codegen/build/ohq_generate`.

## 2. Prepare the model

The model is an ordinary `.ohq` script. Before generating a kernel, check:

1. **Parameters.** Each `create parameter` line defines a calibration parameter with `low`, `high`,
   a start `value` and a `prior_distribution` (`normal` or `log-normal`; log-normal parameters are
   sampled in log10 space). Bind it to model quantities with `setasparameter`:

   ```
   create parameter;type=Parameter,name=p_K,low=0.01,high=100,value=1,prior_distribution=log-normal
   setasparameter; object=Aquifer, quantity=hydraulic_conductivity, parametername=p_K
   ```

   The target may be a block, a link, a source or a **composite** (for example an `Urban_HRU`
   property such as `impervious_multiplier`); a composite property reaches its members through the
   composite's mappings, including member values and initial storages computed from it.

2. **Observations.** Each `create observation` names the modeled expression and the file with
   measured data (`observed_data`), the `error_structure` (`normal` or `log-normal`) and the
   `error_standard_deviation`. The GA minimizes the negative log-likelihood summed over all
   observations.

3. **File paths.** Relative paths in the model (forcing and observation files) are resolved
   against the GA **working folder**, not against the folder of the `.ohq` file. Use absolute paths,
   or put the data where the working folder will find them. (A model whose observation files are
   not found runs normally and reports an objective of 0.)

4. **GA settings** are system values in the model (`setvalue; object=system, quantity=..., value=...`):

   | quantity | meaning | typical |
   |---|---|---|
   | `maxpop` | population size | 40 |
   | `ngen` | number of generations (the GA runs generations 0 … ngen) | 20 |
   | `numthreads` | individuals evaluated in parallel | number of physical cores, memory permitting |
   | `pcross`, `pmute` | crossover and mutation probability | 1, 0.02 |
   | `shakescale`, `shakescalered` | initial perturbation scale and its reduction factor per generation | 0.05, 0.75 |
   | `outputfile` | population log; **use `GA_output.txt`** — it is the file `--continue` reads | `GA_output.txt` |
   | `maximum_time_allowed` | wall-clock cap per forward run, seconds (see §6) | a few × the normal run time |

5. **Solver settings** (`initial_time_step`, tolerances, …) are also system values. Most of them are
   compiled into the kernel (§10), so settle them before generating it.

Run the model once with the interpreter (the GUI or `OpenHydroQual-Console`) to make sure it loads
without errors and gives sensible results before you calibrate it.

## 3. Generate the kernel

```bash
codegen/build/ohq_generate model.ohq resources gen_model MyModel Storage --project shared
```

| argument | meaning |
|---|---|
| `model.ohq` | the model to compile |
| `resources` | the repository's `resources/` folder (the templates the model loads) |
| `gen_model` | output folder: a self-contained CMake project |
| `MyModel` | C++ class name; also the library name (`libMyModel.so`) and the prefix of its symbols |
| `Storage` | the state quantity to integrate (almost always `Storage`) |
| `--project shared` | emit a shared library with the `ohq_kernel_*` interface that `--kernel` loads |

The generator stops with an error if a calibration parameter would not reach any value in the
generated code (`estimated parameter '<name>' ... Refusing to emit a kernel that would silently
ignore it`). Fix the `setasparameter` binding; do not work around it.

## 4. Build the kernel

```bash
cmake -S gen_model -B gen_model/build -DCMAKE_BUILD_TYPE=Release
cmake --build gen_model/build -j$(nproc)
```

Result: `gen_model/build/libMyModel.so` (and a small `MyModel_example` program). Always build
**Release**; a Debug kernel is about ten times slower. Large models (thousands of state variables)
take a few minutes to compile.

## 5. Check the kernel with one forward run (optional)

`MyModel_example` runs the whole simulation once with the start values of the parameters:

```bash
time gen_model/build/MyModel_example > /dev/null
```

This tells you the run time of one forward run, which sets the GA's run time (§11) and a sensible
`maximum_time_allowed`. The generated header `gen_model/MyModel_api.h` documents the C interface
(`create`, `set_parameter`, `apply_parameters`, `initialize`, `run_to`, `observation_at`, …) if you
want to drive the kernel from your own program — for sensitivity runs or scenario tests with a
calibrated parameter set, for example:

```cpp
#include <string>
#include "MyModel_api.h"
auto* h = MyModel_create();
for (int i = 0; i < MyModel_n_parameters(); ++i)                 // set by name
    if (std::string(MyModel_parameter_name(i)) == "p_K") MyModel_set_parameter(h, i, 12.5);
MyModel_apply_parameters(h);                                      // propagate to the model
MyModel_initialize(h);                                            // initial state (after parameters)
MyModel_run_to(h, MyModel_simulation_end());
for (int i = 0; i < MyModel_n_observations(); ++i)                // modeled series of each observation
    for (int k = 0; k < MyModel_observation_count(h, i); ++k) { double t, v; MyModel_observation_at(h, i, k, &t, &v); }
MyModel_destroy(h);
```

Compile with `g++ -O2 -I gen_model mydriver.cpp -L gen_model/build -lMyModel -Wl,-rpath,$PWD/gen_model/build`.
Set the parameters and call `apply_parameters` **before** `initialize`: initial storages that
depend on parameters are computed from the parameter values at that moment.

## 6. Run the GA

```bash
terminal/OHQ-GA/build/OHQ-GA model.ohq work_dir --kernel /abs/path/to/gen_model/build/libMyModel.so
```

* `work_dir` is created if needed; all outputs go there. Use a new folder for every run you want to
  keep.
* Pass the path of the `.so` itself, preferably absolute.
* Other options: `--continue` (§9); `--sparse` and `--dependency-jacobian` apply to the interpreter
  only — the kernel uses the solver settings it was generated with.

At startup the runner prints what it checked:

```
Forward model  : generated kernel /abs/path/to/gen_model/build/libMyModel.so
Kernel class    : MyModel
Kernel verified : 9 parameters, 6 observations, names match the model; all 9 kernel-owned parameters move it.

Generations : 20
Population  : 40
Parameters  : 9

Running GA ...
```

* **names match the model** — the kernel was generated from a model with the same parameters and
  observations, in the same order.
* **all … parameters move it** — before the GA starts, the runner simulates the first 5 % of the
  period once with the start values and once per parameter moved to the far side of its range, and
  refuses any parameter that does not change the result. This runs on one core and takes a minute
  or so; the parallel runs start afterwards.

Each forward run is limited to `maximum_time_allowed` seconds of wall-clock time. A run that exceeds
it is scored as a failed solve (objective 1e18) and dropped. Without a sensible limit, one parameter
set that makes the solver crawl holds up the whole generation, because a generation ends only when
its slowest individual finishes.

## 7. Monitor the run

In another terminal:

```bash
tail -f work_dir/detail_GA.txt
```

`detail_GA.txt` has a line when each individual starts (with its parameter values) and when it
finishes:

```
gen 3 | ind 17  | DONE   objective= 5.291e+04  wall_s=412    solver_s=405    solved=yes
```

`wall_s` is the run time of that individual, `solver_s` the part spent in the solver, `solved=no` a
failed or timed-out run. `GA_output.txt` gets one block per completed generation:

```
Generation: 3
ID, p_K, ..., neg_log_likelihood, Fitness, Rank, Q_site_MSE, Q_site_R2, Q_site_NSE, ...
```

with one row per individual: parameter values (already converted back from log space), the
objective `neg_log_likelihood` (**lower is better**), a rank-based selection weight, and MSE, R² and
NSE for every observation (for `log-normal` observations these are computed on the logarithms).
The best individual of every generation:

```python
import io, re, pandas as pd
blocks = re.split(r"^Generation: (\d+)\n", open("work_dir/GA_output.txt").read(), flags=re.M)
for g, body in zip(blocks[1::2], blocks[2::2]):
    df = pd.read_csv(io.StringIO(body), skipinitialspace=True)
    df = df[pd.to_numeric(df.neg_log_likelihood, errors="coerce") < 1e17]      # drop failed runs
    print(g, df.sort_values("neg_log_likelihood").iloc[0].to_dict())
```

## 8. Results

After the last generation the runner makes a final forward run with the best parameter set and
writes to `work_dir`:

| file | content |
|---|---|
| `GA_output.txt` | the population of every generation (the calibration history) |
| `detail_GA.txt` | start and end of every forward run, with run times |
| `fit_measures.txt` | MSE, R², NSE per observation for the best parameter set |
| `mapped_modeled_results.txt` | modeled values of every observation at the observation times |
| `outputs.txt`, `observedoutputs.txt`, `errors.txt`, `state.json` | outputs of the final run |

`mapped_modeled_results.txt` together with the observation files is enough to plot modeled against
measured values. For full time series with the best parameters (not only at the observation times),
drive the kernel directly (§5).

## 9. Continue a run

```bash
terminal/OHQ-GA/build/OHQ-GA model.ohq work_dir --kernel /abs/path/libMyModel.so --continue
```

`--continue` reads `work_dir/GA_output.txt`, seeds the initial population from the individuals of
its **last** generation, and appends the new generations to the same file. Notes:

* The model's `outputfile` must be `GA_output.txt`.
* The parameter names in the file's header must match the model's parameters, in the same order;
  otherwise the runner stops. Adding or removing a parameter therefore means a fresh start (or a
  seed file edited to the new parameter list).
* The GA does not resume in the middle of a generation; it restarts the generation count and the
  perturbation scale, and re-evaluates the seeded individuals.
* The seeded objective values are recomputed, so a continued run on a modified model is valid —
  but it is a new calibration seeded with old parameter sets, not a continuation of the old one.

Continuing helps when the best objective was still improving at the end of the run. If it has not
changed for many generations, more generations of the same model rarely help; look at the fit
instead (§12).

## 10. What needs a new kernel, and what does not

The kernel contains the model's structure and equations; the runner reads the rest of the model
file every time it starts.

| change | new kernel? |
|---|---|
| blocks, links, sources, composites, their fixed property values | **yes** |
| a template file in `resources/` used by the model (even with an unchanged model file) | **yes** |
| initial conditions, simulation start and end time | **yes** |
| solver settings (`initial_time_step`, tolerances, …) | **yes** |
| adding, removing, renaming or reordering parameters or observations | **yes** (the runner refuses the old kernel) |
| parameter `low`, `high`, start `value`, prior | no |
| observation data files (contents), error standard deviations | no |
| GA settings: `maxpop`, `ngen`, `numthreads`, `pcross`, `pmute`, `shakescale`, `outputfile` | no |
| `maximum_time_allowed` | no |

The startup check (§6) compares only the names and counts of parameters and observations. A change
to the physics that leaves those names alone is **not** detected, and the GA would calibrate the old
model. The safe rule: **regenerate the kernel whenever the model file or a template it uses
changes** — see the script in §13.

## 11. Memory and run time

**Run time.** One generation takes about
`ceil(maxpop / numthreads) × (run time of the slowest individual in each batch)`. Parameter sets
near the edges of their ranges can be several times slower than the start values, so estimate with
the slower runs from `detail_GA.txt`, not the fastest. The whole run is `ngen + 1` generations.

**Memory.** Each thread holds one forward run. Besides the model state, a run stores the modeled
value of every observation on a uniform grid with spacing `initial_time_step`, twice (in the kernel
and in the runner). The memory per thread is therefore roughly

```
2 × number of observations × (simulation length / initial_time_step) × 16 bytes  + model state
```

— about 1.3 GB per thread for 6 observations over 7 years at `initial_time_step = 0.0002` day, but a
few hundred MB for 2 years at 0.01 day. Multiply by `numthreads` and leave room for everything else
on the machine: if the operating system runs out of memory it kills the largest or least protected
process — often the GA (the terminal shows `Killed`; `grep -i "out of memory" /var/log/syslog` shows
which process was chosen). Reduce `numthreads`, shorten the calibration period, or close other large
programs (a GIS with large rasters open is a common culprit).

**Threads.** More threads than physical cores rarely helps. On CPUs with performance and efficiency
cores, runs on the slower cores set the pace of each batch.

## 12. Troubleshooting

| symptom | cause and fix |
|---|---|
| `Kernel ... count mismatch` or `name mismatch at index i` | the kernel was built from a different version of the model — regenerate it (§3–4) |
| `Parameter ... does NOT change its output ... Refusing to calibrate against it` | the parameter has no effect on the model over the probe period (wrong binding, or a process that never occurs); fix the binding or remove the parameter |
| objective exactly 0 for every individual | observation files not found — check the paths (§2.3) |
| a generation never finishes; one `START` without `DONE` in `detail_GA.txt` | a very slow parameter set; set `maximum_time_allowed` (§6) |
| `Killed` shortly after `Running GA ...` | out of memory (§11) |
| many `solved=no` | parameter ranges reach values where the model cannot be solved; narrow them |
| `--continue` stops with a parameter mismatch | the model's parameters changed since that run; start fresh |
| the best objective stops improving after a few generations | the GA has converged for this model; check the fit of the best run before running more generations — systematic misfit points to the model structure, not to the calibration |
| one CPU busy for the first minute, the rest idle | the parameter check before the GA (§6); the parallel runs start after it |

## 13. A complete script

Regenerating the kernel every time guarantees that the GA calibrates the model you think it does:

```bash
#!/usr/bin/env bash
set -euo pipefail
OHQ=/path/to/OpenHydroQual
MODEL=model.ohq            # the model to calibrate
NAME=MyModel               # kernel class / library name
GEN=gen_$NAME              # generated project folder
WORK=calib/run1            # GA working folder (a new folder per run)

"$OHQ/codegen/build/ohq_generate" "$MODEL" "$OHQ/resources" "$GEN" "$NAME" Storage --project shared
cmake -S "$GEN" -B "$GEN/build" -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$GEN/build" -j"$(nproc)" > /dev/null
"$OHQ/terminal/OHQ-GA/build/OHQ-GA" "$MODEL" "$WORK" --kernel "$(realpath "$GEN/build/lib$NAME.so")"
```

Run it in the foreground (or in `tmux`/`screen` on a remote machine) to see the startup checks and
the generation log as they happen.
