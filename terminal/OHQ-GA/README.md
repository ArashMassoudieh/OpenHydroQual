# OHQ-GA: genetic-algorithm calibration from the command line

`OHQ-GA` does what the GUI's **Optimize** button does, without the GUI. It reads an `.ohq` model,
searches the model's `parameter` entries within their bounds, scores every parameter set against
the model's `observation` entries (or its objective functions), and writes the population of every
generation.

The GA is a **global** search: it needs only the parameter bounds, not a good starting point. Use
it first, then refine its result with [OHQ-LM](../OHQ-LM/README.md) and, if you need full
uncertainty, [OHQ-MCMC](../OHQ-MCMC/README.md).

The forward model runs one of two ways:

* **interpreter** (default): the same code the GUI uses. Nothing to prepare.
* **codegen kernel** (`--kernel`): a compiled copy of the model, typically 10 to 40 times faster.
  For a calibration that would take more than a few minutes, this is worth the extra steps.

Building, model preparation and the kernel workflow are shared with the other programs and
described in [`../OHQ-Common/README.md`](../OHQ-Common/README.md). This file is the GA guide.

---

## Contents

1. [Quick start](#1-quick-start)
2. [GA settings](#2-ga-settings)
3. [Objective functions instead of observations](#objective-functions-instead-of-observations)
4. [Run with the interpreter](#4-run-with-the-interpreter)
5. [Run with a codegen kernel](#5-run-with-a-codegen-kernel)
6. [Monitor the run](#6-monitor-the-run)
7. [Results](#7-results)
8. [Next step: refine with LM](#8-next-step-refine-with-lm)
9. [Continue a run](#9-continue-a-run)
10. [Memory and run time](#10-memory-and-run-time)
11. [Troubleshooting](#11-troubleshooting)
12. [A complete script](#12-a-complete-script)

---

## 1. Quick start

```bash
# build once (see ../OHQ-Common/README.md for dependencies)
cd terminal/OHQ-GA && mkdir -p build && cd build && qmake6 ../OHQ-GA.pro && nice make -j4

# check that the model loads and its observations are found
terminal/OHQ-LM/build/OHQ-LM model.ohq --verify-residuals

# calibrate
terminal/OHQ-GA/build/OHQ-GA model.ohq work_dir
```

Before the first run, check the points in
[OHQ-Common §3](../OHQ-Common/README.md#3-prepare-the-model): parameters with sensible bounds and
priors, observations whose data files are found, and absolute paths (or no working folder) so the
data are found from the working folder.

## 2. GA settings

GA settings are system values in the model, as the GUI saves them:

```
setvalue; object=system, quantity=maxpop, value=40
setvalue; object=system, quantity=ngen, value=40
setvalue; object=system, quantity=numthreads, value=8
```

| quantity | meaning | default | advice |
|---|---|---|---|
| `maxpop` | population size | 40 | 20 to 40 times the number of parameters is generous; 40 is enough for a handful |
| `ngen` | number of generations | 40 | look at how the best objective evolves (§6) and stop adding generations when it no longer improves |
| `numthreads` | individuals solved in parallel | 8 | number of physical cores; sets the memory use (§10) |
| `pcross` | crossover probability | 1 | |
| `pmute` | mutation probability | 0.02 | |
| `shakescale` | initial perturbation scale | 0.05 | |
| `shakescalered` | factor applied to `shakescale` each generation | 0.75 | |
| `outputfile` | population log | `GA_output.txt` | keep `GA_output.txt`: it is the file `--continue` reads |
| `continue_based_on_filename` | seed the first generation from a previous GA output file (GUI equivalent of `--continue`) | empty | |
| `maximum_time_allowed` | wall-clock limit per forward run, seconds | 86400 | set it to a few times the normal run time (§5.3) |

Parameters with a `log-normal` prior are searched in log10 space, others in linear space. Every
individual stays within `[low, high]`. The parameters' `value=` entries are not used by the GA.

The GA minimizes the **negative log-likelihood** summed over all observations (lower is better).
A run that fails to solve, or exceeds `maximum_time_allowed`, scores 1e18 and is dropped.

<a id="objective-functions-instead-of-observations"></a>
## 3. Objective functions instead of observations

When the model defines **objective functions**, the GA minimizes their weighted sum in place of the
observation likelihood. Use this to optimize a design or operating rule with no measured data, for
example to minimize a peak flow or an exceedance volume:

```
create objectivefunction;type=Objective_Function,name=peak,object=Outlet,expression=flow,method=maximum,weight=1
```

| `method` | value of the objective |
|---|---|
| `integrate` | integral of the expression over the simulation |
| `value` | value at the end of the simulation |
| `maximum` | maximum over the simulation |
| `variance` | variance over the simulation |
| `exceedance` | the value exceeded with probability `exceedance_probability` |
| `tail_expectation` | mean of the values above that exceedance quantile (sensitive to how large the extremes are, not only how long they last) |

If a model has both, the GA uses the objective functions and ignores the observations. LM and MCMC
always use the observations.

## 4. Run with the interpreter

```bash
OHQ-GA model.ohq [work_dir] [options]
```

* `work_dir` defaults to the model's folder and is created if needed. Use a new folder for every
  run you want to keep; a new run overwrites the files of the previous one.
* Options: `--continue` (§9), and the interpreter solver options `--dependency-jacobian`,
  `--sparse` and `--verify-jacobian` (see [OHQ-Common §4](../OHQ-Common/README.md#4-run-with-the-interpreter)).

The program prints the settings it read and then one report per generation:

```
Model file     : model.ohq
Working folder : work_dir/
Templates      : /path/to/OpenHydroQual/resources/
Forward model  : interpreter

Generations : 4
Population  : 16
Parameters  : 2

Running GA ...
[gen    1/4]  best objective 5.207965e+03  failed 0/16  shakescale 0.05
        params:  p_k_f=5.374e+00  p_KD=1.465e+02
[gen    2/4]  best objective 9.396558e+02  failed 0/16  shakescale 0.05
        params:  p_k_f=5.529e+00  p_KD=3.801e+02
...
```

After the last generation it prints the best parameter set and makes a final forward run with it.

## 5. Run with a codegen kernel

The full workflow is in [OHQ-Common §5](../OHQ-Common/README.md#5-run-with-a-codegen-kernel). In
short:

```bash
OHQ=/path/to/OpenHydroQual
$OHQ/codegen/build/ohq_generate model.ohq $OHQ/resources gen_model MyModel Storage --project shared
cmake -S gen_model -B gen_model/build -DCMAKE_BUILD_TYPE=Release
cmake --build gen_model/build -j4
```

### 5.1 Check the kernel

Compare the kernel with the interpreter at the starting values. The number of points per
observation must be the same, and the misfit should agree within about 1%:

```bash
$OHQ/terminal/OHQ-LM/build/OHQ-LM model.ohq check --verify-residuals
$OHQ/terminal/OHQ-LM/build/OHQ-LM model.ohq check --verify-residuals --kernel $PWD/gen_model/build/libMyModel.so
```

If they disagree, calibrate with the interpreter.

### 5.2 Run

```bash
OHQ-GA model.ohq work_dir --kernel /abs/path/gen_model/build/libMyModel.so
```

At start-up the program verifies the kernel against the model:

```
Forward model  : generated kernel /abs/path/gen_model/build/libMyModel.so
Kernel class    : MyModel
Kernel verified : 9 parameters, 6 observations, names match the model; all 9 kernel-owned parameters move it.
```

Before the GA starts, each parameter is moved to the far side of its range on a short run to make
sure it changes the result. This uses one core and can take a minute; the parallel runs start
after it. The solver options `--sparse` and `--dependency-jacobian` do not apply to a kernel: it
uses the solver settings it was generated with.

**Regenerate the kernel whenever the model or a template it uses changes**
([OHQ-Common §6](../OHQ-Common/README.md#6-keep-the-kernel-in-sync-with-the-model)). The start-up
check only compares names, so a kernel built from an older version of the physics is not detected.

### 5.3 Time one forward run

`MyModel_example` runs the whole simulation once at the starting values:

```bash
time gen_model/build/MyModel_example > /dev/null
```

This gives the run time of one forward run, which sets the GA's run time (§10) and a sensible
`maximum_time_allowed`.

### 5.4 Use the kernel from your own program

The generated header `gen_model/MyModel_api.h` is a C interface for scenario or sensitivity runs
with a calibrated parameter set:

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
Set the parameters and call `apply_parameters` **before** `initialize`, because initial storages
that depend on parameters are computed at that moment.

## 6. Monitor the run

The generation reports on the screen show the best objective so far and how many individuals
failed. For more detail, in another terminal:

```bash
tail -f work_dir/detail_GA.txt
```

`detail_GA.txt` has a line when each individual starts (with its parameter values) and when it
finishes:

```
gen 3 | ind 17  | START  p_K=12.5, p_n=0.31
gen 3 | ind 17  | DONE   objective= 5.291e+04  wall_s=412    solver_s=405    solved=yes
```

`wall_s` is the run time of that individual, `solved=no` a failed or timed-out run. Individuals
run in parallel, so lines of different individuals interleave.

`GA_output.txt` starts with a description of its columns, then has one block per generation:

```
Generation: 3
ID, p_K, p_n, neg_log_likelihood, Fitness, Rank, Q_MSE, Q_R2, Q_NSE, ...
```

with one row per individual: parameter values (in model units, already converted back from log10),
the objective `neg_log_likelihood` (**lower is better**), a rank-based selection weight, the rank
(1 = best), and MSE, R² and NSE for every observation. After the last generation the file ends with
a `Final Enhancements` section.

The best individual of every generation, with only the Python standard library:

```python
import csv, io, re

text = open("work_dir/GA_output.txt").read().split("Final Enhancements")[0]
parts = re.split(r"^Generation: (\d+)\n", text, flags=re.M)
for gen, body in zip(parts[1::2], parts[2::2]):
    rows = list(csv.DictReader(io.StringIO(body), skipinitialspace=True))
    rows = [r for r in rows if float(r["neg_log_likelihood"]) < 1e17]   # drop failed runs
    best = min(rows, key=lambda r: float(r["neg_log_likelihood"]))
    print(gen, best["neg_log_likelihood"], best)
```

## 7. Results

After the last generation the program makes a final forward run with the best parameter set and
writes to `work_dir`:

| file | content |
|---|---|
| `GA_output.txt` | the population of every generation (the calibration history) |
| `detail_GA.txt` | start and end of every forward run, with run times |
| `outputs.txt`, `observedoutputs.txt` | outputs of the best individual, as found during the search |
| `output.txt`, `observedoutput.txt`, `fit_measures.txt`, `mapped_modeled_results.txt`, `objective_function_values.txt`, `errors.txt`, `state.json` | the final forward run (see [OHQ-Common §7](../OHQ-Common/README.md#7-common-output-files)) |

The best parameter set is also printed at the end of the screen output.

## 8. Next step: refine with LM

The GA locates the region of the best fit; LM converges precisely from there and reports standard
errors. LM starts from the parameters' `value=` entries, so write the GA's best values into a copy
of the model:

```bash
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq model_ga.ohq --from work_dir/GA_output.txt
terminal/OHQ-LM/build/OHQ-LM model_ga.ohq lm_dir
```

`set_parameter_values.py` takes the best individual of the last generation and never overwrites
its input. See [OHQ-LM](../OHQ-LM/README.md).

## 9. Continue a run

```bash
OHQ-GA model.ohq work_dir --continue            # add --kernel ... if the first run used one
```

`--continue` reads `work_dir/GA_output.txt`, seeds the first generation from the individuals of its
**last** generation, and appends the new generations to the same file.

* The model's `outputfile` must be `GA_output.txt`.
* The parameter names in the file must match the model's parameters, in the same order; otherwise
  the program stops. Adding or removing a parameter therefore means a fresh start.
* The GA does not resume in the middle of a generation. It restarts the generation count and the
  perturbation scale, and recomputes the objective of the seeded individuals. A continued run on a
  modified model is therefore valid, but it is a new calibration seeded with old parameter sets.

Continuing helps when the best objective was still improving at the end of the run. If it has not
changed for many generations, more generations rarely help; check the fit instead (§11), or move on
to LM.

## 10. Memory and run time

**Run time.** One generation takes about
`ceil(maxpop / numthreads) × (run time of the slowest individual in each batch)`. Parameter sets
near the edges of their ranges can be several times slower than the starting values, so estimate
with the slower runs in `detail_GA.txt`. The whole run is `ngen` generations plus the final run.

**Memory.** Each thread solves one individual at a time on its own copy of the model; the copy is
made just before the solve and released after it (only the best individual is kept). The total is
about

```
(numthreads + 1) × (model copy + observation series)
```

and does not depend on `maxpop`. A copy holds all blocks, links, sources and their time series; an
observation series takes about `2 × observations × (simulation length / initial_time_step) × 16
bytes`. To estimate the per-copy size, watch the program's memory (`ps -o rss -p <pid>`) during the
first generation and divide by `numthreads + 1`.

To reduce memory, reduce `numthreads`, clip forcing files to the simulated period (a copy loads
every series the model references, whatever its length), or use a larger `initial_time_step` for
long periods. If the system runs out of memory it kills the largest process, often the GA: the
terminal shows `Killed`, and `grep -i "out of memory" /var/log/syslog` shows which process was
chosen.

**Threads.** More threads than physical cores rarely helps. On CPUs with performance and
efficiency cores, runs on the slower cores set the pace of each batch.

## 11. Troubleshooting

Problems common to all three programs are listed in
[OHQ-Common §8](../OHQ-Common/README.md#8-common-problems).

| symptom | cause and fix |
|---|---|
| `WARNING: individual ... scored a fitness of exactly 0`, best objective 0 | no observation data were found (check the paths, [OHQ-Common §3.4](../OHQ-Common/README.md#34-file-paths)), or the model has no observations and no objective functions |
| a generation never finishes; a `START` without `DONE` in `detail_GA.txt` | a very slow parameter set; set `maximum_time_allowed` (§2) |
| many `solved=no` | parameter ranges reach values where the model cannot be solved; narrow them |
| `Killed` shortly after `Running GA ...` | out of memory (§10); reduce `numthreads` |
| `--continue` stops with a parameter mismatch | the model's parameters changed since that run; start fresh |
| the best objective stops improving after a few generations | the GA has converged for this model. Check the fit of the best run: systematic misfit points to the model structure, not the calibration. Then refine with LM (§8). |
| one CPU busy for the first minute, the rest idle (kernel runs) | the parameter check before the GA (§5.2); the parallel runs start after it |

## 12. A complete script

Regenerating the kernel every time guarantees that the GA calibrates the model you think it does:

```bash
#!/usr/bin/env bash
set -euo pipefail
OHQ=/path/to/OpenHydroQual
MODEL=model.ohq            # the model to calibrate
NAME=MyModel               # kernel class and library name
GEN=gen_$NAME              # generated project folder
WORK=calib/ga_run1         # GA working folder (a new folder per run)

"$OHQ/codegen/build/ohq_generate" "$MODEL" "$OHQ/resources" "$GEN" "$NAME" Storage --project shared
cmake -S "$GEN" -B "$GEN/build" -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$GEN/build" -j4 > /dev/null
KERNEL=$(realpath "$GEN/build/lib$NAME.so")

# the kernel must reproduce the interpreter: compare the n and CalcMisfit lines of these two
"$OHQ/terminal/OHQ-LM/build/OHQ-LM" "$MODEL" "$WORK/check" --verify-residuals | grep "n = \|CalcMisfit"
"$OHQ/terminal/OHQ-LM/build/OHQ-LM" "$MODEL" "$WORK/check" --verify-residuals --kernel "$KERNEL" | grep "n = \|CalcMisfit"

"$OHQ/terminal/OHQ-GA/build/OHQ-GA" "$MODEL" "$WORK" --kernel "$KERNEL"
python3 "$OHQ/terminal/OHQ-Common/set_parameter_values.py" "$MODEL" "${MODEL%.ohq}_ga.ohq" --from "$WORK/GA_output.txt"
```

Without codegen, drop the generate, build and check lines and the `--kernel` option. Run the script
in the foreground, or in `tmux` or `screen` on a remote machine, to see the start-up checks and the
generation reports as they happen.
