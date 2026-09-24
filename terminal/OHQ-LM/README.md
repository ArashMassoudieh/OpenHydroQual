# OHQ-LM

Headless Levenberg-Marquardt calibration. Build and usage instructions shared
with the other headless runners, including the codegen `--kernel` workflow, are
in one place:

    ../OHQ-Common/README.md

Quick start:

    mkdir -p build && cd build
    qmake6 ../OHQ-LM.pro
    make -j$(nproc)

## What it does

Minimises the same negative log-likelihood `OHQ-GA` minimises, but by
Gauss-Newton steps on the residual vector instead of by selection over a
population. Roughly one model run per calibrated parameter per iteration,
against the GA's population times generations.

It also reports what the GA cannot: at the solution `(J'J)^-1` is the
Gauss-Newton approximation of the likelihood's inverse Hessian, so the run ends
with a standard error and 95% interval for every parameter and the correlation
matrix between them, written to `LM_covariance.txt`.

## It is local

LM starts from each parameter's **current value** in the model file. That is the
intended division of labour:

    OHQ-GA model.ohq out/      # finds the basin, writes its estimates back
    OHQ-LM model.ohq out/      # converges inside it, quantifies the uncertainty

From a cold start on a multimodal problem it will find whichever optimum is
nearest, which is why the GA remains the right first pass.

## Check the likelihood decomposes before trusting a calibration

    OHQ-LM model.ohq out/ --verify-residuals

One forward solve. Confirms that `0.5*||r||^2 + sum_k N_k*log(sigma_k)` equals
`System::CalcMisfit()` on this model, which is the identity the whole method
rests on, and reports the residual count and effective sample size per
observation.

`Least Squared`, `Weighted Least Squared` and `EMC` all decompose. `Similarity`
does not: it is an autocorrelation and Kolmogorov-Smirnov distance with no
Gauss-Newton form, and LM refuses to run on a model that uses it.

## Calibrated error standard deviations

When a parameter drives an observation's `error_standard_deviation`, LM sets it
to its analytic maximiser at every iteration and drops it from the search
instead of differentiating through it. This is exact, costs nothing, and is what the
GA converges to as well. `--no-profile-sigma` turns it off, which exists to make
the difference measurable, not because it is a good idea.

## Options

    --verify-residuals   check the decomposition and exit
    --iterations <n>     maximum iterations (default 50)
    --lambda0 <x>        initial Marquardt damping (default 1e-2)
    --fd-step <x>        finite-difference step, as a fraction of each
                         parameter's range (default 1e-3)
    --central            central differences: twice the runs, less bias
    --no-profile-sigma   keep a calibrated sigma in the LM vector
    --threads <n>        parallel model solves per Jacobian (default 8)
    --kernel <lib.so>    forward model from a codegen-generated library

A flag that is not given leaves the model's own `LM` settings object alone;
a flag that is given overrides it.

## A note on the finite-difference step

The forward model is solved with an adaptive time step, so the objective is only
piecewise smooth. A perturbation smaller than the solver's own error measures
numerical noise, not a gradient. The default step is 1e-3 of each
parameter's range, which clears that on typical settings. If a calibration
stalls at the first iteration with no step accepted, try a larger `--fd-step`
or a tighter solver tolerance before concluding the model is at an optimum.
