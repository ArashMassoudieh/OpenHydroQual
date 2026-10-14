# Headless parameter estimation for OpenHydroQual

Three console programs run calibration and uncertainty analysis on an OpenHydroQual model
without the GUI, so you can run them on a server, a cluster node, or in a script:

| program | method | answers | GUI equivalent |
|---|---|---|---|
| [`OHQ-GA`](../OHQ-GA/README.md) | genetic algorithm | "which parameter values fit best?" (global search) | **Optimize** |
| [`OHQ-LM`](../OHQ-LM/README.md) | Levenberg-Marquardt | "where exactly is the optimum, and how well is each parameter determined?" (local, fast) | **Levenberg-Marquardt** |
| [`OHQ-MCMC`](../OHQ-MCMC/README.md) | Markov chain Monte Carlo | "what is the full posterior distribution of the parameters and predictions?" | **Inverse Run** |

All three read an ordinary `.ohq` model. The model's `parameter` entries say what to estimate
and within which bounds; its `observation` entries define the likelihood. Nothing in the
programs is specific to a model.

This file covers what the three have in common: building them, preparing a model, and running
with a **codegen kernel** (a compiled copy of the model that runs much faster than the
interpreter). Each program's own README is a step-by-step guide for that method.

---

## Contents

1. [Which method to use](#1-which-method-to-use)
2. [Build the programs](#2-build-the-programs)
3. [Prepare the model](#3-prepare-the-model)
4. [Run with the interpreter](#4-run-with-the-interpreter)
5. [Run with a codegen kernel](#5-run-with-a-codegen-kernel)
6. [Keep the kernel in sync with the model](#6-keep-the-kernel-in-sync-with-the-model)
7. [Common output files](#7-common-output-files)
8. [Common problems](#8-common-problems)

---

## 1. Which method to use

The usual sequence is **GA, then LM, then (if you need full uncertainty) MCMC**:

```
OHQ-GA    finds the region of the best fit, starting from nothing but the parameter bounds
   |      copy the GA's best values into the model's parameter "value=" entries
OHQ-LM    converges precisely from there and reports standard errors and correlations
   |      copy LM's estimates into the model; use its standard errors to size the MCMC steps
OHQ-MCMC  samples the posterior around the optimum and produces prediction intervals
```

| situation | use |
|---|---|
| no idea where the parameters lie; possibly several local optima | GA |
| a reasonable starting point; you want a precise optimum quickly | LM |
| you need standard errors and parameter correlations | LM (linearized) or MCMC (full) |
| you need prediction intervals for the model outputs | MCMC |
| you are optimizing a design or operating objective with no observed data | GA with objective functions (see [OHQ-GA](../OHQ-GA/README.md#objective-functions-instead-of-observations)) |

All three minimize or sample the **same negative log-likelihood**, so their results are directly
comparable.

To copy results into the model between steps, use `set_parameter_values.py` in this folder. It
reads `GA_output.txt` (best individual of the last generation), `LM_covariance.txt` (estimates) or
`posterior_percentiles.txt` (posterior medians) and writes the values into the `value=` entries of
a copy of the model:

```bash
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq model_ga.ohq --from ga_dir/GA_output.txt
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq --from lm_dir/LM_covariance.txt --show
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq model_new.ohq p_K=12.5 p_n=0.3
```

## 2. Build the programs

Dependencies (Ubuntu):

```bash
sudo apt-get install build-essential cmake qt6-base-dev libarmadillo-dev liblapack-dev \
                     libblas-dev libgsl-dev libopenblas-dev libsuperlu-dev
```

Each program builds the same way from its own folder. For example, for OHQ-GA:

```bash
cd terminal/OHQ-GA
mkdir -p build && cd build
qmake6 ../OHQ-GA.pro
nice make -j4
```

The result is `terminal/OHQ-GA/build/OHQ-GA`. Do the same in `terminal/OHQ-LM` and
`terminal/OHQ-MCMC`.

* `OHQ` (the OpenHydroQual checkout) defaults to the repository the project file is in, and
  `JSONCPP` to its vendored copy, so an in-tree build needs no arguments. To build against another
  checkout: `qmake6 ../OHQ-GA.pro OHQ=/path/to/OpenHydroQual`.
* The programs compile the model library's sources directly. **After pulling changes to
  `aquifolium/`, rebuild them** (`make` again); an old binary keeps running the old model code.
* Use a modest `-j`. Building with every core at once can exhaust memory on a desktop.

The code generator, needed only for the kernel path (§5), builds with CMake from the repository
root:

```bash
cmake -S codegen -B codegen/build -DCMAKE_BUILD_TYPE=Release
cmake --build codegen/build -j4
```

The result is `codegen/build/ohq_generate`.

## 3. Prepare the model

The model is an ordinary `.ohq` script; you can build it in the GUI and save it. Before
calibrating, check these points.

### 3.1 Parameters

Each estimated quantity needs a `parameter` with bounds, a starting value and a prior, and a
`setasparameter` line that binds it to one or more model quantities:

```
create parameter;type=Parameter,name=p_K,low=0.01,high=100,value=1,prior_distribution=log-normal
setasparameter; object=Aquifer, quantity=hydraulic_conductivity, parametername=p_K
```

| field | meaning |
|---|---|
| `low`, `high` | bounds. GA and LM never leave them. MCMC rejects proposals outside them only for `uniform` priors (see below). |
| `value` | starting value. **LM starts here**, and MCMC chains start here, so update it after a GA run. GA ignores it. |
| `prior_distribution` | `log-normal`, `normal` or `uniform` (see below) |

How each prior is used:

| prior | GA and LM search in | MCMC prior density |
|---|---|---|
| `log-normal` | log10 space | log-normal with geometric mean `sqrt(low*high)` and log standard deviation `ln(high/low)/4`, so `low` and `high` are about 2 standard deviations out |
| `normal` | linear space | normal with mean `(low+high)/2` and standard deviation `(high-low)/4` |
| `uniform` | linear space | flat on `[low, high]`, zero outside (set it in the script; the GUI menu offers only the first two) |

Use `log-normal` for anything positive that can span orders of magnitude (conductivities, rate
constants, error standard deviations).

**Naming rule:** a parameter must not have the same name as the block, link, reaction parameter
or observation it drives. Prefix it, for example `p_k_f` driving the reaction parameter `k_f`.
Two objects with the same name make the binding ambiguous.

A parameter can drive a block, link, source, reaction parameter, a composite's property (which
reaches its members through the composite's mappings), or an observation's
`error_standard_deviation` (see §3.2).

### 3.2 Observations

Each `observation` compares one modeled expression with a measured time series:

```
create observation;type=Observation,name=Q_outlet,object=Outlet link,expression=flow,error_structure=log-normal,error_standard_deviation=0.2,observed_data=/abs/path/Q_outlet.csv
```

| field | meaning |
|---|---|
| `object`, `expression` | what is modeled: an expression evaluated on a block or link (for example `flow`, `depth`, `NO3:concentration`) |
| `observed_data` | two-column CSV, time and value, in model time |
| `error_structure` | `normal` compares values; `log-normal` compares their logarithms (for strictly positive quantities spanning orders of magnitude). Zero or negative values cannot be used with `log-normal`. |
| `error_standard_deviation` | sigma of the residuals (of the log residuals for `log-normal`). It can be fixed, or estimated by binding a parameter to it. |
| `comparison_method` | `Least Squared` (default), `Weighted Least Squared` (downweights older data), `EMC` (event mean concentrations, see `docs/emc_calibration.tex`), or `Similarity` (not usable with LM) |
| `error_autocorrelation`, `error_correlation_time` | `AR1` with a correlation time accounts for serially correlated residuals (for example high-frequency sensor data). Default `none`. |
| `likelihood_scale` | a cruder correction for correlated residuals: divides the effective number of points. Do not combine it with `AR1`. |

The negative log-likelihood summed over all observations is what GA and LM minimize and MCMC
samples. With correlated residuals and no correction (no `AR1`, `likelihood_scale` 1), every point
counts as independent: estimates are still reasonable, but stated uncertainties come out too
narrow.

### 3.3 Settings

Calibration settings are written as system values, the same way the GUI saves them:

```
setvalue; object=system, quantity=maxpop, value=40
```

Writing them to the settings object by name (`object=Optimizer`, `object=MCMC`, `object=LM`) also
works. The settings for each method are listed in its README. Solver settings (time step,
tolerances, `maximum_time_allowed`) are written the same way and apply to every method.

### 3.4 File paths

A relative file name in the model (`observed_data`, forcing time series) is looked up first in the
**working folder** you give the program, then in the **current directory** you run it from. The
folder of the `.ohq` file is not searched. If you run from one place with the working folder in
another, relative files are not found, and **a missing observation file does not stop the run**:
that observation contributes nothing, and the run calibrates against nothing.

Safe options:

* use absolute paths in the model, or
* leave out the working folder (it defaults to the model's folder), or
* copy the data into the working folder.

Always check the start-up output (§8): it should list each observation with a non-zero number of
points.

The templates (`resources/`) are found from the path compiled into the program (the checkout it
was built from). Override it with the `OHQ_RESOURCES` environment variable.

### 3.5 Check the model first

Run the model once in the GUI (or with `OpenHydroQual-Console`) at the starting values. It
should load without errors and give sensible results. Then run `OHQ-LM model.ohq
--verify-residuals`. It does one forward run and prints, per observation, the number of compared
points and the negative log-likelihood. This is the quickest way to confirm that the data are
found and the likelihood is what you expect, whichever method you then use.

## 4. Run with the interpreter

```bash
OHQ-GA   model.ohq [working_folder] [options]
OHQ-LM   model.ohq [working_folder] [options]
OHQ-MCMC model.ohq [working_folder] [options]
```

* `working_folder` defaults to the folder containing the model, and is created if needed. All
  outputs go there. Use a new folder for each run you want to keep.
* With no options the forward model is solved by the interpreter, the same code the GUI uses.
* Command-line options override the model's settings; settings not given on the command line keep
  their values from the model.

Options shared by GA and MCMC (they change the interpreter's solver; the kernel ignores them):

| option | effect |
|---|---|
| `--continue`, `-c` | GA: seed the population from the last generation in `GA_output.txt`. MCMC: resume the chain file. |
| `--dependency-jacobian` | build the transport Jacobian from the dependency graph (exact; faster for many blocks) |
| `--sparse` | solve the Newton step with a sparse LU (SuperLU) |
| `--verify-jacobian` | diagnostic: build the Jacobian both ways and log the difference (slow) |
| `--kernel <library>` | use a codegen kernel (§5) |

LM's options are in its README.

## 5. Run with a codegen kernel

`ohq_generate` translates a model into C++ with every equation written out, and compiles it into
a shared library. Passing that library with `--kernel` replaces the forward solve; everything else
(parameters, likelihood, the GA/LM/MCMC algorithm and the output files) stays the same. A kernel
is typically 10 to 40 times faster, which matters because a calibration runs the model hundreds
to hundreds of thousands of times.

The steps are: **generate, build, check, run.**

### 5.1 Generate

```bash
codegen/build/ohq_generate model.ohq /path/to/OpenHydroQual/resources gen_model MyModel Storage --project shared
```

| argument | meaning |
|---|---|
| `model.ohq` | the model to compile |
| `resources` | the repository's `resources/` folder |
| `gen_model` | output folder: a self-contained CMake project |
| `MyModel` | C++ class name; also the library name (`libMyModel.so`) |
| `Storage` | the state variable (almost always `Storage`) |
| `--project shared` | a shared library with the `ohq_kernel_*` interface that `--kernel` loads. (`exe` builds a standalone forward-model program; `lib` a static library for your own C++.) |

`ohq_generate` resolves the model's relative paths from **the model's folder** and bakes the
forcing time series into the kernel. If the generator stops with `estimated parameter '<name>' ...
Refusing to emit a kernel that would silently ignore it`, that parameter's `setasparameter`
binding reaches nothing in the model; fix the binding.

### 5.2 Build

```bash
cmake -S gen_model -B gen_model/build -DCMAKE_BUILD_TYPE=Release
cmake --build gen_model/build -j4
```

The result is `gen_model/build/libMyModel.so` (`.dylib` on macOS, `.dll` on Windows) and a small
`MyModel_example` program that runs the model once. Always build **Release**; a Debug kernel is
about ten times slower. Large models take a few minutes to compile.

### 5.3 Check that the kernel reproduces the interpreter

Do this once for every new model, before calibrating with the kernel:

```bash
OHQ-LM model.ohq check --verify-residuals
OHQ-LM model.ohq check --verify-residuals --kernel $PWD/gen_model/build/libMyModel.so
```

Compare the two outputs: each observation should have the **same number of points** (`n = ...`)
and a `CalcMisfit()` value within about 1%. If they disagree, calibrate with the interpreter and
report the model; a calibration on a kernel that does not reproduce the interpreter finds the
optimum of a different model.

### 5.4 Run

```bash
OHQ-GA   model.ohq work_dir --kernel /abs/path/gen_model/build/libMyModel.so
OHQ-LM   model.ohq work_dir --kernel /abs/path/gen_model/build/libMyModel.so
OHQ-MCMC model.ohq work_dir --kernel /abs/path/gen_model/build/libMyModel.so
```

Pass the library file itself, preferably as an absolute path (a relative path is resolved from the
current directory). At start-up the program checks the kernel against the model and prints:

```
Forward model  : generated kernel /abs/path/gen_model/build/libMyModel.so
Kernel class    : MyModel
Kernel verified : 9 parameters, 6 observations, names match the model; all 9 kernel-owned parameters move it.
```

* **names match the model**: every parameter and observation in the model has a counterpart in the
  kernel with the same name (they are matched by name, so their order may differ).
* **parameters move it**: the program simulates the first 5% of the period at the starting values
  and once per parameter moved to the far side of its range, and refuses any parameter that does
  not change the result. This runs on one core and can take a minute on a large model.

Parameters bound to an observation's `error_standard_deviation` are handled by the program, not the
kernel, and are skipped in this check.

A line about a **missing symbol** is a warning, not an error, and the run continues. It means the
kernel was generated by an older `ohq_generate`; regenerate it. In particular, without
`ohq_kernel_step_to` the per-run time limit (`maximum_time_allowed`) does not work and a slow
parameter set can hang the run.

### 5.5 Drive the kernel from your own program

`gen_model/MyModel_api.h` is a plain C interface (`MyModel_create`, `MyModel_set_parameter`,
`MyModel_apply_parameters`, `MyModel_initialize`, `MyModel_run_to`, `MyModel_observation_at`, ...).
Use it for scenario or sensitivity runs at a calibrated parameter set. Set the parameters and call
`apply_parameters` **before** `initialize`, because initial storages that depend on parameters are
computed at that moment. See [OHQ-GA §5.4](../OHQ-GA/README.md#54-use-the-kernel-from-your-own-program) for an
example.

## 6. Keep the kernel in sync with the model

The kernel holds the model's structure, equations, fixed values, initial conditions, solver
settings and forcing. The programs read everything else from the `.ohq` every time they start.

| change | new kernel? |
|---|---|
| blocks, links, sources, reactions, composites, or their fixed property values | **yes** |
| a template in `resources/` that the model uses (even with an unchanged `.ohq`) | **yes** |
| initial conditions, simulation start or end time | **yes** |
| solver settings (time step, tolerances, ...) | **yes** |
| forcing time series (contents or file) | **yes** |
| adding, removing or renaming parameters or observations | **yes** (the program refuses the old kernel) |
| a parameter's `low`, `high`, `value`, prior | no |
| observation data files (contents), `error_standard_deviation`, error model | no |
| GA, LM and MCMC settings, `maximum_time_allowed` | no |

The start-up check compares only **names** of parameters and observations. A change to the physics
that keeps those names is **not detected**, and the calibration would fit the old model. The safe
rule: **regenerate and rebuild the kernel whenever the model file or a template it uses changes.**
It takes seconds for small models; wire it into the script that launches the run (see the
complete script in each program's README).

For anyone changing the code:

* After editing `codegen/runtime/`, run `codegen/tools/embed_runtime.py` before rebuilding
  `ohq_generate`; the runtime is embedded in the generator.
* `ohq_kernel.h` (in this folder) is the consumer half of the interface that
  `codegen/src/CodeGenerator.cpp` emits. Change one, change the other, and bump
  `ohq_kernel_abi_version`.

## 7. Common output files

Every program finishes with a forward run at its final parameter set (GA: best individual; LM:
estimate; MCMC: posterior median) and writes to the working folder:

| file | content |
|---|---|
| `output.txt` | all model outputs of that final run (name from the model's `alloutputfile`) |
| `observedoutput.txt` | the modeled value of every observation over time (name from `observed_outputfile`) |
| `fit_measures.txt` | MSE, R² and NSE per observation (for `log-normal` observations, of the logarithms) |
| `mapped_modeled_results.txt` | modeled values at the observation times; with the observation files this is all you need to plot modeled against measured |
| `objective_function_values.txt` | value of each objective function |
| `errors.txt` | errors and warnings recorded during the run |
| `state.json` | the model at the end of the final run |

The method-specific files are described in each README.

## 8. Common problems

| symptom | cause and fix |
|---|---|
| `No parameters have been defined - nothing to estimate.` | the model has no `parameter` entries, or the templates failed to load (check the `Templates :` line at start-up and the `loadtemplate` paths) |
| `There are errors in the values assigned to some variables:` followed by a list | the model has invalid values; fix them in the GUI first |
| an observation with `n = 0`, an objective of exactly 0, or `... was not found!` | the observation file was not found: see §3.4 |
| `The model does not define an Optimizer settings object` (or `MCMC`) | the model was not saved with a settings section; add the settings (§3.3) or save it again from the GUI |
| kernel: `count mismatch` or `name mismatch` | the kernel was built from another version of the model: regenerate it |
| kernel: `Parameter ... does NOT change its output ... Refusing to calibrate against it` | the parameter has no effect on the model (wrong binding, or a process that never occurs in the simulated period); fix the binding or remove the parameter |
| `Killed` with no other message | out of memory; reduce the number of threads |
| GA or MCMC much slower than expected | the default thread counts are in the model settings (`numthreads`, `number_of_threads`); check them |
