# OHQ-MCMC: Bayesian parameter estimation from the command line

`OHQ-MCMC` does what the GUI's **Inverse Run** does, without the GUI. It samples the posterior
distribution of the model's `parameter` entries given its `observation` entries, by Markov chain
Monte Carlo with several parallel chains. It then summarizes the posterior (percentiles and
marginal densities) and runs the model for a set of posterior samples to give prediction
intervals for every observation.

Use it after [OHQ-GA](../OHQ-GA/README.md) and [OHQ-LM](../OHQ-LM/README.md): MCMC explores the
neighbourhood of a good parameter set efficiently, but it is slow to find one from scratch. LM's
standard errors also tell you how large the MCMC steps should be (§4), which is the setting that
decides whether a run is useful.

The forward model runs with the interpreter (default) or a codegen kernel (`--kernel`). MCMC needs
thousands to hundreds of thousands of model runs, so the kernel matters most here. Building, model
preparation and the kernel workflow are shared with the other programs and described in
[`../OHQ-Common/README.md`](../OHQ-Common/README.md). This file is the MCMC guide.

---

## Contents

1. [Quick start](#1-quick-start)
2. [How the sampler works](#2-how-the-sampler-works)
3. [MCMC settings](#3-mcmc-settings)
4. [Step size: the setting that matters most](#4-step-size-the-setting-that-matters-most)
5. [Run with the interpreter](#5-run-with-the-interpreter)
6. [Run with a codegen kernel](#6-run-with-a-codegen-kernel)
7. [Monitor the run](#7-monitor-the-run)
8. [Results](#8-results)
9. [Has it converged?](#9-has-it-converged)
10. [Continue a run](#10-continue-a-run)
11. [Run time](#11-run-time)
12. [Troubleshooting](#12-troubleshooting)
13. [A complete script](#13-a-complete-script)
14. [Known issues](#14-known-issues)

---

## 1. Quick start

```bash
# build once (see ../OHQ-Common/README.md for dependencies)
cd terminal/OHQ-MCMC && mkdir -p build && cd build && qmake6 ../OHQ-MCMC.pro && nice make -j4

# start the chains at the LM estimates
python3 terminal/OHQ-Common/set_parameter_values.py model.ohq model_lm.ohq --from lm_dir/LM_covariance.txt

# set the step size (§4) and the thread count in model_lm.ohq, then
terminal/OHQ-MCMC/build/OHQ-MCMC model_lm.ohq mcmc_dir
```

## 2. How the sampler works

* `number_of_chains` chains run side by side. Each iteration proposes one new sample per chain and
  evaluates the proposals in parallel (`number_of_threads` at a time).
* `number_of_samples` is the **total** number of samples over all chains, including the burn-in.
  With 8 chains and 16000 samples, each chain takes 2000 steps.
* The first `number_of_burnout_samples` samples (also counted over all chains) are discarded from
  the posterior summaries.
* **Starting point.** All chains start at the parameters' `value=` entries. With
  `initial_purturbation = Yes`, chain 0 starts there and the others are scattered around it.
* **Proposals.** Each parameter is perturbed by a normal step (in log space for `log-normal`
  parameters). The step size starts from `initual_purturbation_factor` (§4) and adapts: every
  50 × `number_of_chains` samples, if the acceptance rate in that window was above
  `acceptance_rate` the step grows by 1/`purturbation_change_scale`, otherwise it shrinks by
  `purturbation_change_scale`.
* **Covariance-adapted proposals** (default on). After the burn-in, and once enough post-burn-in
  samples exist, proposals are drawn from the covariance of the samples so far, so the sampler can
  move along ridges where parameters trade off against each other. The covariance is refreshed
  every `covariance_update_interval` samples.
* **Prior.** The log posterior is the log-likelihood plus the log prior of each parameter
  ([OHQ-Common §3.1](../OHQ-Common/README.md#31-parameters)). A proposal outside a `uniform`
  prior's range is rejected without running the model. `normal` and `log-normal` priors are not
  truncated at `low` and `high`.
* A proposal whose model run fails, or exceeds `maximum_time_allowed`, is rejected.

## 3. MCMC settings

Settings are system values in the model, as the GUI saves them
(`setvalue; object=system, quantity=..., value=...`). Note the spellings: `purturbation` and
`initual` are the names the program reads.

| quantity | meaning | default | advice |
|---|---|---|---|
| `number_of_samples` | total samples over all chains, including burn-in | 1000 | thousands per chain; see §9 |
| `number_of_chains` | parallel chains | 8 | 4 to 16; more chains make the convergence check more reliable |
| `number_of_burnout_samples` | samples discarded at the start (over all chains) | 8 | a quarter to a half of `number_of_samples` |
| `number_of_threads` | proposals evaluated in parallel | **1** | set it to the number of physical cores (at most `number_of_chains`) |
| `initual_purturbation_factor` | initial step size, as a fraction of each parameter's range | 0.05 | **usually far too large**; see §4 |
| `initial_purturbation` | scatter the chains' starting points around `value=` | No | Yes when you want chains to start apart (§9) |
| `acceptance_rate` | target acceptance rate for the step adaptation | 0.15 | 0.15 to 0.3 |
| `purturbation_change_scale` | factor by which the step shrinks or grows at each adaptation | 0.75 | |
| `covariance_proposal` | covariance-adapted proposals (§2) | Yes | keep Yes |
| `covariance_min_samples` | post-burn-in samples needed before the first covariance | 500 | |
| `covariance_update_interval` | samples between covariance refreshes | 1000 | the first refresh happens at this sample number, so it must be well below `number_of_samples` |
| `covariance_scale` | step multiplier for covariance proposals, divided by sqrt(number of parameters) | 2.38 | rarely needs changing |
| `record_interval` | write every n-th sample to the chain file | 1 | |
| `samples_filename` | chain file | `mcmc.txt` | |
| `number_of_post_estimate_realizations` | model runs at random posterior samples, for the prediction intervals | 10 | 100 or more for stable intervals; 0 to skip |
| `add_noise_to_realizations` | add the observation error to the realizations | No | Yes for intervals that include measurement error |
| `continue_based_on_file_name` | resume from this chain file (GUI equivalent of `--continue`) | empty | |
| `maximum_time_allowed` | wall-clock limit per model run, seconds (a solver setting) | 86400 | a few times the normal run time |

## 4. Step size: the setting that matters most

The initial step for each parameter is `initual_purturbation_factor` times its range:

* `log-normal` parameter: step in ln(value) = factor × ln(high/low)
* `normal` or `uniform` parameter: step = factor × (high − low)

The default factor of 0.05 is sized for parameters that are only loosely determined by the data.
For a well-determined parameter it is many times larger than the width of the posterior, so
**almost every proposal is rejected**: the progress line shows `accept 0.000`, the chains do not
move, and the posterior percentiles collapse onto the starting value. The adaptation shrinks the
step by only 0.75 every 50 × `number_of_chains` samples, which is too slow to rescue a short run.

Size the factor from LM's standard errors (`LM_covariance.txt`):

```
factor ≈ (2.4 / sqrt(d)) × sigma / range
```

where `d` is the number of parameters, and for each parameter

| prior | `sigma` | `range` |
|---|---|---|
| `log-normal` | 2.303 × `std_error` (LM reports it in log10 units) | ln(high/low) |
| `normal`, `uniform` | `std_error` | high − low |

The same factor applies to all parameters, so use the **smallest** value over the parameters.
The covariance-adapted proposals then correct the step for each parameter once they start.

Example (`Examples/LM_Calibration/01_Batch_Sorption`, 2 log-normal parameters): LM gives a
`std_error` of 0.00082 for `p_k_f` (range 0.1 to 50) and 0.0057 for `p_KD` (range 10 to 5000).
For `p_k_f`: 1.7 × (2.303 × 0.00082) / ln(500) ≈ 0.0005, the smaller of the two.

| factor | acceptance | posterior 95% interval of `p_KD` |
|---|---|---|
| 0.05 (default) | 0.000 to 0.001 | 522 to 526: the chains never left the starting value |
| 0.0005 | 0.21 to 0.34 | 511 to 537, matching LM's 95% interval of 510 to 536 |

If you have no LM result, start with a small factor (0.001 to 0.005), watch the acceptance rate in
the first minutes (§7), and adjust.

## 5. Run with the interpreter

```bash
OHQ-MCMC model.ohq [mcmc_dir] [options]
```

* `mcmc_dir` defaults to the model's folder and is created if needed. Use a new folder for every
  run you want to keep.
* Options: `--continue` (§10), and the interpreter solver options `--dependency-jacobian`,
  `--sparse` and `--verify-jacobian` (see [OHQ-Common §4](../OHQ-Common/README.md#4-run-with-the-interpreter)).

The program prints the settings it read:

```
Samples   : 1600  (burn-in 400)
Chains    : 8   threads 4
Parameters: 2

Running MCMC (this writes the chain incrementally to mcmc_dir/mcmc.txt) ...
MCMC configuration:
  chains            : 8
  samples           : 1600
  burn-in           : 400
  proposal          : COVARIANCE-ADAPTED (correlated)
    scale           : 2.38 / sqrt(2)
    update interval : 1000
    min samples     : 500
  initial perturb   : No
```

then a progress line per iteration (§7). At the end it computes the posterior summaries, runs the
realizations, and makes a final forward run at the posterior median.

## 6. Run with a codegen kernel

The full workflow is in [OHQ-Common §5](../OHQ-Common/README.md#5-run-with-a-codegen-kernel):

```bash
OHQ=/path/to/OpenHydroQual
$OHQ/codegen/build/ohq_generate model.ohq $OHQ/resources gen_model MyModel Storage --project shared
cmake -S gen_model -B gen_model/build -DCMAKE_BUILD_TYPE=Release
cmake --build gen_model/build -j4

# the kernel must reproduce the interpreter: same n per observation, CalcMisfit within ~1%
$OHQ/terminal/OHQ-LM/build/OHQ-LM model.ohq check --verify-residuals
$OHQ/terminal/OHQ-LM/build/OHQ-LM model.ohq check --verify-residuals --kernel $PWD/gen_model/build/libMyModel.so

OHQ-MCMC model.ohq mcmc_dir --kernel $PWD/gen_model/build/libMyModel.so
```

The start-up output confirms the kernel (`Kernel verified : ... names match the model`). If the
parity check disagrees, sample with the interpreter: a posterior computed on a kernel that does not
reproduce the interpreter describes a different model.

**Regenerate the kernel whenever the model or a template it uses changes**
([OHQ-Common §6](../OHQ-Common/README.md#6-keep-the-kernel-in-sync-with-the-model)).

## 7. Monitor the run

Each iteration prints a progress line:

```
[   800/1600]  50.3%  logp best -134.28  now -136.50  accept 0.324  pert 0.0055  stuck 6/8  00:00:39 elapsed, ~00:00:38 left
```

| field | meaning |
|---|---|
| `logp best`, `now` | best and current log posterior (higher is better) |
| `accept` | acceptance rate so far. **Near 0: the step is too large (§4). Near 1: too small.** |
| `pert` | current step-size coefficient (of the first parameter) |
| `stuck` | chains whose last proposal was rejected |

Check `accept` in the first minutes. If it stays at 0.000, stop the run and reduce
`initual_purturbation_factor`; waiting rarely helps (§4).

When the covariance proposal is refreshed, a line `[covariance proposal refreshed at sample N from M
states]` appears. If it never appears, `covariance_update_interval` or `covariance_min_samples` is
too large for the run.

`MCMC_details.txt` records the start, run time, status and objective value of every model run.

## 8. Results

| file | content |
|---|---|
| `mcmc.txt` | the chains: sample number, parameter values, `logp` (log posterior), and diagnostic columns (see [Known issues](#14-known-issues)) |
| `MCMC_details.txt` | one entry per model run |
| `posterior_percentiles.txt` | 2.5%, 50% and 97.5% percentiles and the mean of each parameter, from the post-burn-in samples of all chains |
| `Posterior_distributions.txt` | the marginal posterior density of each parameter |
| `Realizations_<observation>.txt` | the modeled series of each observation for every realization |
| `Predicted_95p_Bracket<observation>.txt` | mean, 2.5%, 50% and 97.5% of the realizations over time: the prediction interval |
| `output.txt`, `observedoutput.txt`, `fit_measures.txt`, `mapped_modeled_results.txt`, `objective_function_values.txt`, `errors.txt`, `state.json` | the final forward run at the posterior median (see [OHQ-Common §7](../OHQ-Common/README.md#7-common-output-files)) |

`posterior_percentiles.txt` is tab-separated:

```
RowLabel	p_k_f	p_KD
0.025	5.171805611	510.858124
0.5	5.219448553	523.3475148
0.975	5.237724651	537.2469233
mean	5.2101267	523.6264627
```

In `mcmc.txt`, sample `n` belongs to chain `n mod number_of_chains`.

The prediction interval comes from `number_of_post_estimate_realizations` runs at random posterior
samples. Without `add_noise_to_realizations` it reflects parameter uncertainty only; with it, it
also includes the observation error.

## 9. Has it converged?

A posterior is only meaningful once the chains have forgotten where they started and agree with
each other. Check:

* **The acceptance rate** settled between about 0.15 and 0.4.
* **The chains agree.** The Gelman-Rubin statistic R-hat compares the spread within each chain with
  the spread between chains; values below about 1.1 for every parameter indicate convergence.
  Start the chains apart (`initial_purturbation = Yes`) for this test to be meaningful.

R-hat from the chain file, with only the Python standard library:

```python
import csv, statistics, sys

# usage: python3 rhat.py mcmc_dir/mcmc.txt <number_of_chains> <number_of_burnout_samples>
path, nchains, burnin = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
rows = [r for r in csv.reader(open(path), skipinitialspace=True) if r and r[0].strip()]
header, rows = [h.strip() for h in rows[0]], rows[1:]
params = header[1:header.index("logp")]
for k, name in enumerate(params, start=1):
    chains = [[] for _ in range(nchains)]
    for r in rows:
        if int(r[0]) >= burnin:
            chains[int(r[0]) % nchains].append(float(r[k]))
    n = min(len(c) for c in chains)
    chains = [c[:n] for c in chains]
    means = [statistics.fmean(c) for c in chains]
    W = statistics.fmean(statistics.variance(c) for c in chains)
    B = n * statistics.variance(means)
    rhat = (((n - 1) / n * W + B / n) / W) ** 0.5 if W > 0 else float("inf")
    print(f"{name:20s} R-hat = {rhat:.3f}   ({n} samples per chain)")
```

`R-hat = inf` means at least one chain never moved (step too large, §4). Values well above 1.1 mean
more samples are needed: run longer, or continue the run (§10). In the step-size example of §4,
1600 samples (200 per chain) gave R-hat 1.33 and 1.14, so that run was a good start but still too
short for final results.

## 10. Continue a run

```bash
OHQ-MCMC model.ohq mcmc_dir --continue            # add --kernel ... if the first run used one
```

`--continue` reads the chain file in `mcmc_dir` (`samples_filename`, default `mcmc.txt`), restarts
each chain from its last recorded sample, and appends to the file from the next sample number, so no
sample number appears twice. Raise `number_of_samples` in the model to extend a finished run. The
parameter names in the file must match the model's; otherwise the program stops. It also stops when
a chain has no rows in the file, which happens with `record_interval` larger than 1. If no chain
file is found, it starts a fresh run and says so.

## 11. Run time

The run makes about `number_of_samples + number_of_post_estimate_realizations` model runs (fewer
for `uniform` priors, whose out-of-range proposals are rejected without a run). They run
`number_of_threads` at a time, so

```
run time ≈ (number_of_samples / number_of_threads) × (time of one model run)
```

Example: 16000 samples, 8 threads and a 2-second model run take about an hour with the
interpreter. The same run with a kernel 20 times faster takes about 3 minutes. Time one forward run
with `gen_model/build/MyModel_example`, or from `MCMC_details.txt`.

Memory: each thread holds one copy of the model while it runs.

## 12. Troubleshooting

Problems common to all three programs are listed in
[OHQ-Common §8](../OHQ-Common/README.md#8-common-problems).

| symptom | cause and fix |
|---|---|
| `accept 0.000` for a long time, posterior percentiles equal to the starting values | the step is too large: reduce `initual_purturbation_factor` (§4) |
| acceptance close to 1, chains drift slowly | the step is too small: increase the factor |
| only one CPU busy | `number_of_threads` is 1 (the default) |
| no `covariance proposal refreshed` line | `covariance_update_interval` (default 1000) is too large for `number_of_samples`; lower it |
| R-hat well above 1.1 | not converged: more samples (§9, §10) |
| the posterior sits against a bound | with a `uniform` prior, the data want values outside the range: widen it. With `normal` or `log-normal`, the prior is pulling the estimate; check that its range is intended. |
| `logp best` jumps to a much better value late in the run | the chains started away from the optimum: start from the LM estimates and discard more burn-in |
| very slow progress | a slow model run: use a kernel (§6), more threads, and set `maximum_time_allowed` |

## 13. A complete script

```bash
#!/usr/bin/env bash
set -euo pipefail
OHQ=/path/to/OpenHydroQual
MODEL=model_lm.ohq         # the model, with value= set from LM and the MCMC settings of §3-4
WORK=calib/mcmc_run1
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
    "$OHQ/terminal/OHQ-LM/build/OHQ-LM" "$MODEL" "$WORK/check" --verify-residuals "${ARGS[@]}" | grep "n = \|CalcMisfit"
fi

"$OHQ/terminal/OHQ-MCMC/build/OHQ-MCMC" "$MODEL" "$WORK" "${ARGS[@]}"
```

To go from scratch to a posterior: run the GA script in [OHQ-GA §12](../OHQ-GA/README.md#12-a-complete-script),
then the LM script in [OHQ-LM §11](../OHQ-LM/README.md#11-a-complete-script), set the MCMC step
size from `LM_covariance.txt` (§4), and run this one.

## 14. Known issues

These are current limitations of the sampler, not of the command-line program:

* `logp_1`, `stuck_counter` and the `purt_coeff_*` columns of `mcmc.txt` are internal diagnostics.
