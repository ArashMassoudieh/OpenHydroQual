# Case 1: batch sorption and desorption

From `Batch_sorption_desorption.ohq`. A single groundwater cell holds copper in
two phases, aqueous (`Cu_aq`) and sorbed (`Cu_s`), exchanging by first-order
kinetics:

    sorption     rate = k_f * Cu_aq
    desorption   rate = (k_f / KD) * Cu_s

Starting from 1 g/m3 dissolved and nothing sorbed, the aqueous concentration
decays with a timescale of about 0.2 and reaches equilibrium by t = 8, where
the ratio Cu_s / Cu_aq equals KD exactly.

## The estimation problem

| Parameter | Truth | Start | Range | Searched in |
|---|---:|---:|---|---|
| `k_f` | 5.0 | 1.5 | 0.1 to 50 | log10 |
| `KD` | 500 | 120 | 10 to 5000 | log10 |

The two are informed by different parts of the curve, which is what makes the
case well conditioned: **k_f** by the early decay rate, **KD** by the height of
the equilibrium plateau. Their fitted correlation is only -0.08.

Observations are `Cu_aq:concentration` and `Cu_s:concentration`, 60 points each,
log-spaced from t = 0.02 to 20, with 5% multiplicative noise and a log-normal
error structure. Both `error_standard_deviation` values are fixed at the true
noise level of 0.05, so the confidence intervals are directly checkable.

## Result

    k_f = 5.012   (truth 5.0)    95% CI [4.996, 5.029]
    KD  = 511.2   (truth 500)    95% CI [498.5, 524.2]

Both intervals cover the truth. 5 iterations, 29 model solves, -logL = -299.5
against -286.5 at the true values.

## A note on the solver setting

This case overrides the source model's `nr_tolerance`, raising it from 1e-3 to
1e-10, and `minimum_timestep` from 1e-6 to 1e-9.

`Cu_aq` falls through six decades to a plateau at 2.3e-7. At the shipped
tolerance everything below roughly 1e-3 was numerical noise that changed
between runs, and since KD is identified by exactly that plateau, it could not
be recovered: LM returned k_f = 3.92 against a truth of 5.0, with confidence
intervals that excluded the true value. The fit looked converged and the
diagnostics looked healthy.

Any quantity whose informative range extends far below the solver tolerance has
this problem, and it is invisible unless you check the likelihood at known
parameter values. That is what `at_truth.ohq` is for.
