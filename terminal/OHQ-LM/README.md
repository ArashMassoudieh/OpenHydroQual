# OHQ-LM: Levenberg-Marquardt calibration from the command line

`OHQ-LM` minimizes the same negative log-likelihood as [OHQ-GA](../OHQ-GA/README.md), but by
Gauss-Newton steps on the vector of residuals. It needs roughly one model run per parameter per
iteration, and usually converges in 5 to 15 iterations, so it is far cheaper than a GA.

It also reports what the GA cannot. At the solution, `(J'J)^-1` approximates the inverse Hessian of
the likelihood, so the run ends with a **standard error and 95% interval for every parameter and
the correlation matrix between them**.

LM is **local**: it starts from each parameter's `value=` in the model and converges to the nearest
optimum. Run the GA first when you do not have a good starting point.

The forward model runs with the interpreter (default) or a codegen kernel (`--kernel`). Building,
model preparation and the kernel workflow are shared with the other programs and described in
[`../OHQ-Common/README.md`](../OHQ-Common/README.md). This file is the LM guide.

---

## Contents

1. [Quick start](#1-quick-start)
2. [Check the likelihood first](#2-check-the-likelihood-first)
3. [The starting point](#3-the-starting-point)
4. [Settings and options](#4-settings-and-options)
5. [Run with the interpreter](#5-run-with-the-interpreter)
6. [Run with a codegen kernel](#6-run-with-a-codegen-kernel)
7. [Results](#7-results)
8. [Estimated error standard deviations](#8-estimated-error-standard-deviations)
9. [Next step: MCMC](#9-next-step-mcmc)
10. [Troubleshooting](#10-troubleshooting)
11. [A complete script](#11-a-complete-script)
12. [Worked examples](#12-worked-examples)

---

## 1. Quick start

```bash
# build once (see ../OHQ-Common/README.md for dependencies)
cd terminal/OHQ-LM && mkdir -p build && cd build && qmake6 ../OHQ-LM.pro && nice make -j4

# check the likelihood: one forward run
terminal/OHQ-LM/build/OHQ-LM model.ohq --verify-residuals

# calibrate from the parameters' value= entries
terminal/OHQ-LM/build/OHQ-LM model.ohq lm_dir --threads 4
```

After a GA run, start LM from the GA's best parameter set:

```bash
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq model_ga.ohq --from ga_dir/GA_output.txt
terminal/OHQ-LM/build/OHQ-LM model_ga.ohq lm_dir
```

## 2. Check the likelihood first

```bash
OHQ-LM model.ohq [check_dir] --verify-residuals
```

This makes one forward run at the starting values and prints, per observation, the number of
compared points, sigma and the negative log-likelihood, then checks that the residual vector LM
works on reproduces the likelihood exactly:

```
Verifying the residual decomposition against CalcMisfit() ...
  observations      : 2
  residuals         : 120
    Cu_aqueous: n = 60, sigma = 0.05, effective N = 60, -logL = 200124
    Cu_sorbed: n = 60, sigma = 0.05, effective N = 60, -logL = 8349.32
  CalcMisfit()      : 208473.75694645988
  from residuals    : 208473.75694645988
  from 0.5*||r||^2  : 208473.75694645985
  absolute error    : 0
  relative error    : 0
MATCH: the residual vector reproduces the misfit to rounding.
```

Check two things:

* **every observation has `n` greater than 0.** `n = 0` means its data file was not found
  ([OHQ-Common §3.4](../OHQ-Common/README.md#34-file-paths)). The check still says `MATCH`,
  because nothing matches nothing.
* **`MATCH`.** A mismatch means LM would optimize something other than the likelihood; report it.

The comparison methods `Least Squared`, `Weighted Least Squared` and `EMC` can be used with LM.
`Similarity` cannot (it has no sum-of-squares form), and LM refuses a model that uses it.

This check is cheap and is also the quickest way to confirm that a model loads and finds its data,
whichever method you then use.

## 3. The starting point

LM starts from each parameter's `value=` entry in the model, and prints them before it starts:

```
Starting parameter values (LM is local; these are where it starts):
  p_k_f = 1.5
  p_KD = 120
```

From a poor starting point on a problem with several optima, LM finds whichever optimum is nearest.
That is why the GA comes first:

```
OHQ-GA   model.ohq ga_dir                      # finds the region of the best fit
set_parameter_values.py model.ohq model_ga.ohq --from ga_dir/GA_output.txt
OHQ-LM   model_ga.ohq lm_dir                   # converges there; quantifies the uncertainty
```

On a well-behaved problem LM can also start from a rough guess (the worked examples in §12 start
3 to 4 times away from the truth).

Parameters with a `log-normal` prior are searched in log10 space, others in linear space. Every
step stays within `[low, high]`; a parameter may end on a bound, in which case its standard error
is not meaningful.

## 4. Settings and options

Settings are system values in the model (`setvalue; object=system, quantity=..., value=...`), or
values of the `LM` object (`setvalue; object=LM, ...`). A command-line option overrides the model's
setting; settings without an option on the command line keep the model's value.

| setting | option | meaning | default |
|---|---|---|---|
| `max_iterations` | `--iterations <n>` | maximum number of iterations | 50 |
| `lambda0` | `--lambda0 <x>` | initial Marquardt damping | 0.01 |
| `fd_step` | `--fd-step <x>` | finite-difference step, as a fraction of each parameter's range (of its log10 range for log-normal parameters) | 0.001 |
| `central_differences` | `--central` | central differences: twice the runs per iteration, less bias | No |
| `profile_sigma` | `--no-profile-sigma` | estimate error standard deviations analytically (§8) | Yes |
| `lm_numthreads` | `--threads <n>` | model runs in parallel when building the Jacobian | 8 |
| `tol_objective` | | stop when an iteration improves the objective by less than this, relative | 1e-8 |
| `max_rejections` | | stop after this many rejected steps (each with more damping) in one iteration | 10 |
| `lm_outputfile` | | iteration log | `LM_output.txt` |
| `covariancefile` | | covariance and standard errors | `LM_covariance.txt` |
| `lambda_up`, `lambda_down` | | damping factor after a rejected / accepted step | 3, 3 |
| `lambda_max`, `tol_step`, `tol_gradient` | | further stopping criteria | 1e12, 1e-8, 1e-8 |

Other options:

| option | effect |
|---|---|
| `--verify-residuals` | check the likelihood (§2) and exit |
| `--kernel <library>` | use a codegen kernel (§6) |

`lm_numthreads` above the number of parameters (twice that with `--central`) does not help: each
iteration has that many independent runs.

**The finite-difference step.** The model is solved with an adaptive time step, so the objective is
only piecewise smooth. A perturbation smaller than the solver's own error measures numerical noise,
not a gradient. The default (0.001 of the range) clears that with typical solver settings. If a
calibration stops at the first iteration with no step accepted, try a larger `--fd-step` (0.01) or
a tighter solver tolerance (`nr_tolerance`) before concluding the model is at an optimum.

## 5. Run with the interpreter

```bash
OHQ-LM model.ohq [lm_dir] [options]
```

`lm_dir` defaults to the model's folder and is created if needed. The program prints its settings
and the starting values, then one line per iteration:

```
Max iterations : 40
Parameters     : 2
Differences    : forward, step 0.001 of range
Profiled sigma : yes

Running Levenberg-Marquardt ...
[LM] Starting from a negative log-likelihood of 208474 with 120 residuals and 2 free parameters.
[LM] iteration 1: -logL = 2455.07, lambda = 0.00333333
[LM] iteration 2: -logL = 135.546, lambda = 0.00111111
[LM] iteration 3: -logL = 133.252, lambda = 0.00037037
[LM] Converged at iteration 4: no step reduced the objective in 10 attempts at increasing damping, so this is a local minimum to within the finite-difference resolution
[LM] Finished after 4 iteration(s) and 24 model solves: ... Final -logL = 133.252.

Estimates:
  p_k_f = 5.22277
  p_KD = 522.728

Standard errors and the parameter correlation matrix are in lm_dir/LM_covariance.txt
```

It then makes a final forward run at the estimates and writes the usual outputs.

## 6. Run with a codegen kernel

The full workflow is in [OHQ-Common §5](../OHQ-Common/README.md#5-run-with-a-codegen-kernel):

```bash
OHQ=/path/to/OpenHydroQual
$OHQ/codegen/build/ohq_generate model.ohq $OHQ/resources gen_model MyModel Storage --project shared
cmake -S gen_model -B gen_model/build -DCMAKE_BUILD_TYPE=Release
cmake --build gen_model/build -j4

# the kernel must reproduce the interpreter: same n per observation, CalcMisfit within ~1%
OHQ-LM model.ohq check --verify-residuals
OHQ-LM model.ohq check --verify-residuals --kernel $PWD/gen_model/build/libMyModel.so

OHQ-LM model.ohq lm_dir --kernel $PWD/gen_model/build/libMyModel.so
```

`--verify-residuals` is the parity check for every method, not only LM: run it with and without
`--kernel` on every new model. If the two disagree, calibrate with the interpreter.

Because LM needs few model runs, the kernel matters less here than for GA or MCMC. It pays off
for slow models, or when you run LM many times (for example from several starting points).

## 7. Results

| file | content |
|---|---|
| `LM_output.txt` | one row per iteration: negative log-likelihood, damping, `accept`/`reject`, parameter values |
| `LM_covariance.txt` | estimates, standard errors, 95% intervals and the correlation matrix |
| `outputs.txt`, `observedoutputs.txt` | outputs at the estimates, as found during the search |
| `output.txt`, `observedoutput.txt`, `fit_measures.txt`, `mapped_modeled_results.txt`, `objective_function_values.txt`, `errors.txt`, `state.json` | the final forward run (see [OHQ-Common §7](../OHQ-Common/README.md#7-common-output-files)) |

`LM_covariance.txt`:

```
# estimate and standard error
parameter               , space   , estimate      , std_error     , ci95_low      , ci95_high
p_k_f                   , log10   ,  5.222773     ,  0.0008202308 ,  5.203475     ,  5.242142
p_KD                    , log10   ,  522.728      ,  0.005652981  ,  509.5607     ,  536.2356

# correlation matrix
                        , p_k_f     , p_KD
p_k_f                   ,  1.0000   , -0.0183
p_KD                    , -0.0183   ,  1.0000
```

* `space` is the space the parameter was searched in. For `log10` parameters, `std_error` is the
  standard error of log10(value); `estimate` and the 95% interval are in model units.
* The 95% interval is the linearized one: estimate ± 1.96 standard errors (in the search space).
  It is reliable when the likelihood is close to quadratic around the optimum. For strongly
  non-linear problems, MCMC gives the full distribution.
* A **correlation near ±1** means the data cannot tell the two parameters apart: they trade off
  against each other. Consider fixing one of them.
* If the covariance could not be computed (for example when a parameter does not affect the
  residuals), the program says so and no standard errors are written.

## 8. Estimated error standard deviations

When a parameter is bound to an observation's `error_standard_deviation`:

```
create parameter;type=Parameter,name=sigma_Q,low=0.001,high=2,value=0.1,prior_distribution=log-normal
setasparameter; object=Q_outlet, quantity=error_standard_deviation, parametername=sigma_Q
```

LM sets it to its analytic maximum-likelihood value at every iteration and leaves it out of the
search (it is "profiled"). This is exact and costs nothing. The GA converges to the same value.
`--no-profile-sigma` keeps it in the search, which exists to measure the difference, not because it
helps.

Likewise, a parameter bound to an observation's `error_correlation_time` (`AR1` error model) is
profiled by a one-dimensional search at each iteration, with the sigmas re-profiled jointly.

## 9. Next step: MCMC

LM's estimates are a good place to start MCMC chains, and its standard errors tell you how large
the MCMC steps should be (see [OHQ-MCMC §4](../OHQ-MCMC/README.md#4-step-size-the-setting-that-matters-most)):

```bash
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq model_lm.ohq --from lm_dir/LM_covariance.txt
terminal/OHQ-MCMC/build/OHQ-MCMC model_lm.ohq mcmc_dir
```

## 10. Troubleshooting

Problems common to all three programs are listed in
[OHQ-Common §8](../OHQ-Common/README.md#8-common-problems).

| symptom | cause and fix |
|---|---|
| `LM did not run: ...` | read the reason. Usually an observation uses `Similarity`, or the model failed to solve at the starting values. |
| stops at iteration 1 with no step accepted | the finite-difference step is lost in solver noise (§4): try `--fd-step 0.01` or a tighter `nr_tolerance` |
| converges to a poor fit | a local optimum: start from the GA's best set (§3) |
| a parameter ends on its bound | the optimum is outside the range, or the parameter is poorly determined; widen the range or fix the parameter. Its standard error is not meaningful. |
| huge standard errors, correlations near ±1 | the data do not determine those parameters independently; fix one, or add data that separate them |
| `No covariance was produced` | `J'J` is singular: a parameter has no effect on the residuals (check its binding) |
| result differs between interpreter and kernel | the kernel does not reproduce the interpreter on this model (§6); use the interpreter |

## 11. A complete script

```bash
#!/usr/bin/env bash
set -euo pipefail
OHQ=/path/to/OpenHydroQual
MODEL=model_ga.ohq         # the model, with value= set from the GA
WORK=calib/lm_run1
USE_KERNEL=yes             # "no" to use the interpreter

ARGS=()
if [ "$USE_KERNEL" = yes ]; then
    NAME=MyModel; GEN=gen_$NAME
    "$OHQ/codegen/build/ohq_generate" "$MODEL" "$OHQ/resources" "$GEN" "$NAME" Storage --project shared
    cmake -S "$GEN" -B "$GEN/build" -DCMAKE_BUILD_TYPE=Release > /dev/null
    cmake --build "$GEN/build" -j4 > /dev/null
    ARGS=(--kernel "$(realpath "$GEN/build/lib$NAME.so")")
    # the kernel must reproduce the interpreter: compare these two
    "$OHQ/terminal/OHQ-LM/build/OHQ-LM" "$MODEL" "$WORK/check" --verify-residuals | grep "n = \|CalcMisfit"
fi
"$OHQ/terminal/OHQ-LM/build/OHQ-LM" "$MODEL" "$WORK/check" --verify-residuals "${ARGS[@]}" | grep "n = \|CalcMisfit\|MATCH"

"$OHQ/terminal/OHQ-LM/build/OHQ-LM" "$MODEL" "$WORK" "${ARGS[@]}"
python3 "$OHQ/terminal/OHQ-Common/set_parameter_values.py" "$MODEL" "${MODEL%.ohq}_lm.ohq" --from "$WORK/LM_covariance.txt"
```

## 12. Worked examples

`Examples/LM_Calibration/` has three synthetic cases (batch sorption, a Monod reactor, a pond with
a weir) with data manufactured from known parameter values, so you can see whether LM recovers the
truth and whether its intervals cover it. Each runs in seconds to a minute with the interpreter:

```bash
cd Examples/LM_Calibration/01_Batch_Sorption
OHQ-LM calibration.ohq . --threads 4
```

Run them from their own folder: their data and template paths are relative. See
`Examples/LM_Calibration/README.md` and `docs/lm_calibration.tex` for the method.
