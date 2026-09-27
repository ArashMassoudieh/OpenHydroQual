# Case 3: pond hydraulics and decay

From `Pond_example.ohq`. A pond receives inflow from `Inflow_pond.csv` over a
year and discharges two ways: a fixed 3000 m3/day withdrawal and a weir with
rating curve `Q = alpha * (depth - crest_elevation)^beta`. A PCB decays in the
pond at rate `k_d * PCB`, with an Arrhenius factor driven by
`Temperature_pond_example.csv`.

This is the only case of the three with flow, and the only one mixing
hydraulic and water-quality parameters.

**One change from the source model.** The shipped example has
`PCB:constant_inflow_concentration = 0`, so the PCB concentration is
identically zero everywhere and the decay rate has nothing to act on. The
inflow concentration is set to 5 g/m3 here so that `k_d` is identifiable.

## The estimation problem

| Parameter | Truth | Start | Range | Bound to |
|---|---:|---:|---|---|
| `weir_alpha` | 794880 | 400000 | 5e4 to 5e6 | weir link `alpha` |
| `crest_elev` | 1.5 | 1.35 | 1.0 to 2.0 | weir link `crest_elevation` |
| `k_d` | 1.2 | 0.45 | 0.05 to 20 | reaction parameter `base_value` |

Three observations, 120 points each, uniform from day 2 to day 364:

| Observation | Range | Noise | Error structure |
|---|---|---|---|
| `pond_depth` | 0.75 to 1.53 m | 0.005 m absolute | normal |
| `weir_outflow` | 19 to 4248 m3/day | 5% multiplicative | log-normal |
| `PCB_conc` | 0.012 to 2.64 g/m3 | 5% multiplicative | log-normal |

The stage record uses an absolute 5 mm error, which is what a pressure
transducer gives; the two quantities spanning decades use relative error.

## Result

    weir_alpha = 722818   (truth 794880)   95% CI [587109, 889895]
    crest_elev = 1.4989   (truth 1.5)      95% CI [1.4958, 1.5020]
    k_d        = 1.2048   (truth 1.2)      95% CI [1.1895, 1.2204]

All three intervals cover the truth. 6 iterations, 35 model solves,
-logL = -1169.3 against -1168.6 at the true values.

`weir_alpha` and `crest_elev` are correlated at 0.96, which is intrinsic to a
weir rating curve: a higher crest and a larger coefficient produce nearly the
same discharge over the observed stage range. The interval on `weir_alpha` is
correspondingly wide, from 587k to 890k, and honestly so. `k_d` is essentially
uncorrelated with either, because it is constrained by the PCB series rather
than the hydraulics.

## The four-parameter variant

`calibration_4param.ohq` adds the pond's stage-storage coefficient
(`pond_alpha`, truth 4140, start 7000) to the same three. It is included
because it fails in an instructive way.

    weir_alpha = 724819   (truth 794880)   CI [588559, 892625]   covers truth
    crest_elev = 1.4990   (truth 1.5)      CI [1.4958, 1.5021]   covers truth
    pond_alpha = 5314     (truth 4140)     CI [4977, 5674]       MISSES
    k_d        = 0.9385   (truth 1.2)      CI [0.8779, 1.0032]   MISSES

It converges, and to a better likelihood than the truth (-1169.7 against
-1168.6), so nothing is wrong with the optimiser. The correlation matrix shows
what happened:

    pond_alpha  <->  k_d     -0.9813

The stage-storage coefficient sets the pond volume, the volume sets the
residence time, and residence time trades directly against a decay rate: a
larger pond with slower decay produces almost the same outflow concentration as
a smaller pond with faster decay. The two cannot be separated by these data, so
the search slid along that ridge to a point that fits marginally better and is
displaced in both.

The individual standard errors give no warning. Each looks tight, and
`pond_alpha`'s interval of [4977, 5674] is 14% wide around a value that is 28%
wrong. Only the correlation tells you not to believe them. Fixing the
stage-storage coefficient at its known value, which the main `calibration.ohq`
does, is what makes the remaining three recoverable.
