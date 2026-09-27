# Levenberg-Marquardt calibration test cases

Three synthetic parameter-estimation cases for the LM calibration described in
`docs/lm_calibration.md`. Each is built from a model in the Aquifolium example
collection, with observed data manufactured by running the model forward at
known parameter values and adding noise. Because the truth is known, each case
answers a question a real calibration never can: did the method recover the
right answer, and is its stated uncertainty honest?

All three are cheap. The whole suite runs in about three minutes on four
threads, using the interpreter (no codegen kernel).

| | Case | Source model | Unknowns | Physics |
|---|---|---|---|---|
| 1 | `01_Batch_Sorption` | `Batch_sorption_desorption.ohq` | 2 | Linear sorption kinetics in a batch cell |
| 2 | `02_Monod_Reactor` | `Reaction_Example.ohq` | 3 + 2 profiled | Monod and second-order decay, time-varying forcing |
| 3 | `03_Pond_Weir` | `Pond_example.ohq` | 3 (and a 4-parameter variant) | Weir hydraulics plus first-order decay |

## Results

Each case folder carries a `fit.png` showing the observed data against the
model at the starting guess and at the calibrated parameters.

Every run starts from parameter values deliberately set well away from the
truth. "CI" is the 95% interval LM reports from `(J'J)^-1`.

| Case | Parameter | Truth | Start | Estimate | CI covers truth |
|---|---|---:|---:|---:|:--:|
| 1 | `p_k_f` | 5.0 | 1.5 | 5.012 | yes |
| 1 | `p_KD` | 500 | 120 | 511.2 | yes |
| 2 | `p_k_A` | 1.0 | 0.35 | 0.9803 | yes |
| 2 | `p_K_s` | 0.6 | 2.5 | 0.5775 | yes |
| 2 | `p_k_B` | 0.5 | 1.6 | 0.5097 | yes |
| 2 | `sigma_A` | 0.0393 | 0.05 | 0.0403 | profiled |
| 2 | `sigma_B` | 0.0375 | 0.05 | 0.0355 | profiled |
| 3 | `weir_alpha` | 794880 | 400000 | 722818 | yes |
| 3 | `crest_elev` | 1.5 | 1.35 | 1.4989 | yes |
| 3 | `p_k_d` | 1.2 | 0.45 | 1.2048 | yes |

Cost, and the likelihood reached against the likelihood at the true values:

| Case | Residuals | Iterations | Model solves | -logL at truth | -logL at estimate |
|---|---:|---:|---:|---:|---:|
| 1 | 120 | 5 | 29 | -286.5 | **-299.5** |
| 2 | 160 | 12 | 68 | -441.9 | **-443.9** |
| 3 | 360 | 6 | 35 | -1168.6 | **-1169.3** |

In every case LM reaches a likelihood slightly better than the one at the true
parameters, which is what should happen: it is fitting one particular noise
realisation, and the best fit to noisy data is never exactly the truth.

## Running them

```
cd Examples/LM_Calibration/01_Batch_Sorption
OHQ-LM calibration.ohq . --threads 4
```

Before trusting any calibration, check that the likelihood decomposes:

```
OHQ-LM calibration.ohq . --verify-residuals
```

Each folder also contains `at_truth.ohq`, the same case with every parameter
set to its true value. Scoring it is the reference point:

```
OHQ-LM at_truth.ohq . --verify-residuals
```

## A naming rule

A calibration `Parameter` must never share a name with the reaction parameter,
block, link or observation it drives. The estimated parameters here are prefixed
`p_`, so `p_k_f` drives the reaction parameter `k_f`:

```
create parameter;type=Parameter,name=p_k_f,low=0.1,high=50,value=1.5,prior_distribution=log-normal
setasparameter; object=k_f, quantity=base_value, parametername=p_k_f
```

Reusing the name makes `object=` and `parametername=` refer to two different
objects that happen to be spelled the same, and which one a lookup resolves to
is not something a model should depend on.

## Files in each case

| File | What it is |
|---|---|
| `truth.ohq` | The model at the true parameter values, with observations but no observed data. Running it forward regenerates the noise-free series the data came from. |
| `calibration.ohq` | The case LM is meant to solve: parameters declared with ranges and deliberately wrong starting values, observations pointing at the data files. |
| `at_truth.ohq` | `calibration.ohq` with the parameters set to the truth. The reference likelihood. |
| `obs_*.csv` | Synthetic observed data, two columns `t,value`. |
| other `.txt`/`.csv` | Forcing data the source model needs, copied from the example collection. |

## How the data were made

For each case the model was run forward at the true parameter values, the
observation series sampled at chosen times, and noise added: additive Gaussian
where the quantity varies over a narrow range, multiplicative (log-normal)
where it spans decades. Each observation's `error_standard_deviation` is then
set to the noise level actually used, except in case 2, where it is calibrated
and LM profiles it out analytically.

Sampling times were chosen to match each problem's dynamics: log-spaced for
case 1, where everything happens in the first two time units of twenty, and
uniform for the other two.

## Two things these cases exposed

**Solver tolerance can destroy identifiability.** Case 1's source model ships
with `nr_tolerance = 1e-3`, which appears to be absolute. Cu_aq decays through
six decades, so everything below about 1e-3 was solver noise that differed from
run to run. Since `KD` is identified almost entirely by the equilibrium plateau
at 2.3e-7, the parameter was unrecoverable and LM confidently returned a wrong
answer with tight confidence intervals. The case sets `nr_tolerance = 1e-10`.
If a calibration returns implausibly narrow intervals around a wrong answer,
suspect the solver before the optimiser.

**Correlated parameters are not the same as wrong ones.**
`03_Pond_Weir/calibration_4param.ohq` adds the pond's stage-storage coefficient
to the three parameters of the main case. It converges to a better likelihood
than the truth, yet lands 28% off on that coefficient and 22% off on the decay
rate. The covariance matrix says why: the pair is correlated at -0.98, because
pond volume sets residence time, which trades directly against a decay rate.
Nothing is wrong with the optimiser; the data cannot separate them. Fixing the
stage-storage coefficient at its known value, which is what the main
`calibration.ohq` does, recovers all three remaining parameters with intervals
that cover the truth. This is the single most useful habit these cases teach:
read the correlation matrix before believing a standard error.
