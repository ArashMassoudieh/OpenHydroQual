## OpenHydroQual 2.1.1

Download the Linux package below, or visit [www.OpenHydroQual.com](https://www.openhydroqual.com).

This release adds a model compiler that turns a model into standalone C++, a Levenberg-Marquardt calibrator, a covariance-adapted MCMC sampler, and new hydrology components (snow, culverts, dual drainage, urban and riparian HRUs). It also fixes several bugs that gave wrong answers without reporting an error. Please read **Changes that affect existing models** before upgrading.

### Changes that affect existing models

- **Channel `area` is now the water-surface area.** In the `open_channel`, `river_processes` and `compound_channel` templates the flow cross-section is now called `xsectionarea`, and `area` is the plan (water-surface) area, as in every other block. Precipitation, reaeration and other area-based sources now act on the correct area. Models whose expressions use a channel's `area` to mean the cross-section must be changed to use `xsectionarea`.
- **Evapotranspiration time-series sources.** The three `Evapotranspiration_Time_Series` sources added water where they should have removed it, and applied the series twice. They now remove water, and the series property is called `ET_timeseries`, so the source's `rate` is the actual ET rate in m/day. Models that set `timeseries=` on these sources load with error 1012 and need the file selected again.
- **Uncertain flow control device renamed.** `Flow_control_device (function, uncertain)` is now `Flow_control_device (function uncertain)`. The comma split the name when a model was loaded from a script, so the link never loaded and any parameters bound to it did nothing. Rename the type in existing `.ohq` files.
- **Uniform priors in MCMC now have bounds.** The uniform prior used to be flat and unbounded, so MCMC chains could wander far outside a parameter's range. Values outside the range are now rejected, which can change the results of MCMC runs that use uniform priors.
- **Time-step clamping.** By default the time step is again limited only by precipitation series. Models driven by fast non-rain forcing, such as imposed gate openings, should set the solver setting `timestep_clamp_series` to `All time series`.

### Model compiler (codegen)

A loaded model can now be compiled into a standalone C++ library. Every equation becomes plain C++, with no expression interpreter at runtime and no dependency on Qt or Armadillo, so the model runs much faster and can be embedded in other programs. Use **Export to C++** in the interface, or the `ohq_generate` command-line tool. The interface can also build the generated project and check its results against the interpreter.

- A fixed C ABI (`ohq_kernel_*`, version 3) for loading a compiled model at runtime: parameters, observations, objective functions, state vectors, forcing replacement, hot restart and constituent masses
- Optional extra outputs selected with `ohq_generate --outputs`
- Transport steps that fail to converge are retried with smaller sub-steps, where previously the whole run was aborted
- Windows DLL kernels are supported
- Results of the compiled and interpreted models agree closely; this was checked on the example models

Documentation: `codegen/README.md`

### Calibration and uncertainty

- **Levenberg-Marquardt** calibration, available from the toolbar and the `lmoptimize` script command, plus a headless runner (`OHQ-LM`). Documentation: `docs/lm_calibration.tex`
- **Covariance-adapted MCMC proposals.** The sampler can propose correlated steps from the covariance of the chain so far, so it can move along ridges where parameters trade off against each other. This is now the default
- **AR1 observation error model.** Observations can carry autocorrelated (continuous-time AR1) errors, with an estimable correlation time. Irregular spacing and gaps are handled exactly. The default is no autocorrelation, which leaves existing models unchanged
- **EMC calibration.** Models can be calibrated against event mean concentrations. Documentation: `docs/emc_calibration.tex`
- New objective-function method **tail_expectation**, the upper-tail partial expectation, which responds to the size of the largest values and not only to how long they last
- **Continue a GA run** from a previous GA output file. GA and MCMC now check that the file's parameter names match the model, and stop if they do not
- Headless GA and MCMC runners (`OHQ-GA`, `OHQ-MCMC`) are now in the repository and can drive compiled models
- The GA keeps only the model copies it is solving in memory, which cuts memory use by up to 5x on large models, with identical results
- Misfit, R² and NSE calculations no longer scale quadratically with series length (about 20x faster on long series)
- Failed simulations are handled during optimization, and the Levenberg-Marquardt progress chart now scales correctly

### Hydrology and hydraulics

- **Snow**: temperature-index snow accumulation and melt, with rain/snow partitioning and elevation offsets (`snowmelt.json`). Documentation: `docs/snowmelt.md`
- **Culverts**: inlet and outlet control based on HDS-5, roadway overtopping, multi-barrel box culverts, reverse flow and constituent transport (`culvert.json`). Documentation: `docs/culvert_model.tex`
- **Dual drainage**: storm-drain inlets between surface channels and sewers, pipe outfalls, and catch basins with a rim and surface spreading. Sewer segments gain a diameter multiplier and a surcharge coefficient
- **Urban HRU** rebuilt on the unconfined groundwater cell, with a deeper-soil ET source, a surface K_sat multiplier, a recharge elevation offset and optional groundwater ET
- **Riparian Urban HRU**: an Urban HRU whose groundwater is split into a regional cell and a near-stream cell, for watersheds whose baseflow recedes over days to weeks
- **Linear reservoir baseflow** connector, available as a port on the HRU
- Channel-to-soil and channel-to-aggregate links that use the channel's top width, and an inflow multiplier on trapezoidal channel segments
- Reaeration sources (O'Connor-Dobbins, Churchill, Owens-Gibbs and Banks-Herrera wind) with temperature correction
- Particle trapping, bed wash-off and interface exchange between a channel and its bed
- Distributed rainfall-runoff in depth-only form, unsaturated soil with a logistic EC model, trench outfall links, and new forecast valve variants (z-threshold and graded opening)
- Compound channel, dam spillway and floodplain bypass templates

Documentation for the HRUs and dual drainage: `docs/hydrologic_response_units.tex`

### Solver

- **Oscillation control**: optionally detects oscillating outputs and either rewinds to a restore point or continues with a smaller time step
- Per-block Newton convergence test (`nr_block_tolerance`), so a small block can no longer be accepted far out of balance next to very large ones
- Jacobian refresh when the time step changes by more than `jacobian_dt_refresh_factor`
- `make_timeseries_uniform` setting, to turn off resampling of input series that can use a lot of memory when `dt0` is small
- Smoother formulations that help Newton converge: sewer pipe flow direction, forecast valve flow near zero head, and soil infiltration from an empty catchment
- A switch for whether results are recorded, and a settings manual (`docs/ohq_settings_manual.tex`)

### Other fixes

- Constituent transport across links that had no `length` or `area` divided by zero, so the run stalled or produced spurious mass transfer (for example at a pond overflow weir). Every link template now defines both properties
- Constants written as a group, such as `(3-1)/2`, were evaluated as their first number only. They are now evaluated correctly
- A time-series default value was loaded as a file name and crashed model loading
- Time series with a single point are handled safely
- Compiled models now match parameters and observations by name, so systems rebuilt in a different order receive the correct values
- macOS build and notarization fixes

### Examples

New example models in `Examples/`: `Snowmelt`, `Culvert` (eleven test cases), `Dual_Drainage`, `Urban_HRU`, `Linear_Baseflow` and `LM_Calibration`.
