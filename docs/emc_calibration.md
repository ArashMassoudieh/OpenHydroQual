# Calibrating against Event Mean Concentrations (EMC) in OpenHydroQual

Stormwater and BMP monitoring programs often report water quality as **event
mean concentrations** (EMCs): one flow-weighted composite concentration per
storm, rather than a concentration time series. An `Observation` with
`comparison_method = EMC` computes the modeled EMC for each monitored event and
compares it with the measured one. The likelihood it returns is used by
**Optimize** (GA) and **Inverse Run** (MCMC), and by the headless `OHQ-GA`
and `OHQ-MCMC` runners, the same way as any other observation. Nothing else
in the model or in the estimation settings has to change.

---

## 1. What is computed

For every event *k* with window `[t_start,k , t_end,k]`, the modeled EMC is the
flow-weighted mean of the observation's expression:

```
            ∫ C(t) · Q(t) dt
EMC_k  =  ---------------------      (integrals over [t_start,k , t_end,k])
               ∫ Q(t) dt
```

* **C(t)** is the observation's `expression` evaluated on its `object`, for
  example `Cu:concentration` on the outlet block.
* **Q(t)** is the weighting: `emc_weighting_expression`, for example `flow`,
  evaluated on `emc_weighting_object`, for example the underdrain link.
* Both are recorded at every accepted solver step, and the integrals are
  trapezoidal over those steps, clipped exactly at the window edges. With
  `Q = flow`, the numerator is the event's mass load, the denominator is its
  runoff volume, and the ratio is a true flow-weighted EMC.

The **observed EMC** for event *k* is the value in `observed_data` whose
timestamp falls inside the event window. If more than one value falls inside,
their mean is used.

The **likelihood** is the same Gaussian form used by `Least Squared`,
applied to the *N* events that have both a modeled and an observed EMC:

```
-log L = N · [ MSE / (2 σ²) + log σ ]      (constant terms dropped)
```

* `error_structure = normal`: the residuals are `EMC_obs − EMC_mod`, and `σ`
  (`error_standard_deviation`) is in concentration units.
* `error_structure = log-normal`: the residuals are
  `log(EMC_obs) − log(EMC_mod)`, and `σ` is the standard deviation of the log
  ratio. For example, σ = 0.3 is roughly a ±35% scatter. Use this for
  concentrations, because EMCs typically span orders of magnitude.

The fit measures reported per observation (MSE, R², NSE) are computed on the
EMCs, in log space when the error structure is log-normal.

### Rules the calculation follows

| Situation | What happens |
|---|---|
| An event has no observed value inside its window | The event is ignored. It adds nothing to N or to the likelihood. |
| An event window lies completely outside the simulated period | The event is ignored. |
| An event window partly overlaps the simulation | The integrals cover only the overlapping part. |
| The modeled weighting (flow) integrates to zero over an event | The EMC is undefined. The time-averaged value of the expression is used instead, and a warning is set on the observation. |
| No events file, or no weighting values were recorded | The observation contributes zero to the likelihood and sets an error message. **Check this before calibrating** (Section 5). |

---

## 2. Input files

### Events file (`emc_events`)

Plain text with two columns per line, `t_start, t_end`, in **model time**:
the same time base as `simulation_start_time` / `simulation_end_time` and as
the observed data. Commas, tabs or spaces all work as separators. Lines that do
not start with a number, such as comments or a header, are skipped.

```
// t_start, t_end   (days)
45728.10, 45728.72
45741.30, 45742.05
45760.00, 45760.40
```

* Rows must be sorted by `t_start`; the file is rejected otherwise.
* `t_end` must be greater than `t_start`.
* Use the **sampling window** of the composite (first to last aliquot) when you
  know it. That is the period the measured EMC actually represents. Otherwise,
  use the start and end of runoff at the monitoring point.
* Windows should not overlap. If they do, an observed value inside both
  windows is used for both events.

### Observed data (`observed_data`)

The usual two-column time series. Each row is one measured EMC, time-stamped
at **any time inside its event window**, such as the event midpoint or the
start of sampling:

```
45728.40, 18.5
45741.60, 11.2
```

The third event in the events file above has no observed value, so it is
simply skipped. That lets you keep one events file for several analytes that
were not all measured in every storm.

---

## 3. Observation properties

| Property | Used by | Meaning |
|---|---|---|
| `object` | all methods | Block or link on which `expression` is evaluated. For an effluent EMC, the block drained by the outlet, or the outlet link itself. |
| `expression` | all methods | The concentration being averaged, e.g. `Cu:concentration`. |
| `observed_data` | all methods | Measured EMCs, one per event, time-stamped inside the event window. |
| `comparison_method` | all methods | Set to **`EMC`**. |
| `emc_weighting_object` | EMC | Block or link on which the weighting expression is evaluated, typically the outlet or underdrain link. **Leave empty** to use `object`. |
| `emc_weighting_expression` | EMC | The weight. Default `flow`. Use `_pos(flow)` if the link can reverse, so that backflow does not subtract from the event volume. |
| `emc_events` | EMC | The events file (Section 2). |
| `error_structure` | all methods | `normal` or `log-normal`. |
| `error_standard_deviation` | all methods | σ of the residual. It can be assigned to a parameter and estimated along with the model parameters. |

The kernel-weighting and autocorrelation properties of the other comparison
methods are ignored when `comparison_method = EMC`.

---

## 4. Using EMC in the GUI

1. **Build the model** so that the monitored outlet exists as a block and/or
   link, and the forcing (inflow hydrograph, influent concentration) covers
   every monitored event. Continuous simulation across the whole monitoring
   period is best, because antecedent moisture and accumulated sorbed mass
   carry over between storms.
2. **Add an Observation** (Observations toolbar) and set, in the property panel:
   * **Object**: the effluent block or outlet link.
   * **Expression**: the concentration, e.g. `Cu:concentration`.
   * **Observed data time series**: browse to the observed-EMC file.
   * **Comparison Method**: `EMC`.
   * **EMC weighting object**: the outlet/underdrain link (or leave empty).
   * **EMC weighting expression**: `flow`.
   * **EMC events file**: browse to the events file.
   * **Error probability distribution**: `log-normal` for concentrations.
   * **Error standard deviation**: an initial guess, e.g. `0.3` for
     log-normal. To estimate it, create a parameter and assign it to this
     property.
3. **Run the model** once. The observation's plot shows the *continuous*
   modeled concentration, not the EMCs; the EMCs are scored after the run.
   Check that the modeled flow on the weighting object is non-zero during
   every event.
4. **Calibrate** with *Optimize* (GA) or *Inverse Run* (MCMC) exactly as for
   any other observation. When they finish, the working folder contains:
   * `fit_measures.txt`: MSE, R² and NSE on the EMCs for each observation.
   * `mapped_modeled_results.txt`: for EMC observations, the **modeled EMC
     per event**, time-stamped like the observed data. Compare it with
     `observed_data` directly.
   * The usual GA/MCMC outputs (`GA_output.txt`, the MCMC chain, posterior
     percentiles and realizations).

Several EMC observations, e.g. dissolved Cu, total Zn and TSS at the same
outlet, can share one events file. Their negative log-likelihoods add up.

---

## 5. Using EMC from the console

### Script syntax (`.ohq`)

```
create observation;type=Observation,name=Eff_Cu_EMC,object=Outlet_cell,expression=Cu:concentration,observed_data=eff_Cu_emc.txt,comparison_method=EMC,emc_weighting_object=Underdrain,emc_weighting_expression=flow,emc_events=events.txt,error_structure=log-normal,error_standard_deviation=0.3
```

Relative file names are resolved against the model's folder.

### Forward run and checking the EMCs: `TOpenHydroQual`

```bash
OpenHydroQual-Console model.ohq -q -r /path/to/OpenHydroQual/resources
```

After the solve, the console scores every observation that has observed data
and prints one line per observation:

```
Observation 'Eff_Cu_EMC' (EMC): MSE=0.2192 R2=1 NSE=-12.88 -logL=17.31
  2 event(s) written to '<workdir>/Eff_Cu_EMC_EMC.txt'
```

For each EMC observation it also writes `<name>_EMC.txt`, with the observed
and modeled EMC for every event that was scored:

```
t, observed_EMC, modeled_EMC
1.5, 9, 6.22253
5, 7, 4.03986
```

Always do this forward run before calibrating. It confirms that:

* the number of scored events is what you expect. Too few usually means
  timestamps outside their windows, or windows outside the simulation;
* no `warning:` line appears (zero flow during an event, missing events
  file, unknown weighting object);
* the modeled EMCs are in a plausible range.

### Calibration: `OHQ-GA` and `OHQ-MCMC`

```bash
OHQ-GA   model.ohq [working_folder]
OHQ-MCMC model.ohq [working_folder]
```

The model's `parameter` entries define what is estimated, and its
observations, including EMC ones, define the likelihood. Both runners write
`fit_measures.txt` and `mapped_modeled_results.txt` (modeled EMCs per event) as
well as their usual outputs. See `terminal/OHQ-Common/README.md` for all the
options.

`--kernel` (codegen): a generated kernel records only each observation's
expression, not the weighting series. If any observation uses EMC, the runners
fall back to the interpreter for the forward solve automatically, so results
stay correct but you do not get the kernel speed-up.

---

## 6. Worked example

A tank receives two storms whose inflow carries a tracer with a first-flush
pollutograph. A time-varying outlet link supplies the weighting flow. One
parameter, a constant background inflow concentration `p_Cbase`, is
calibrated against two observed EMCs. A third event in the events file has no
observation and is correctly ignored.

```
create block;type=Reactor,Storage=10,...,name=Tank,time_variable_inflow=Qin.txt,Tracer:time_variable_inflow_concentration=Cin.txt,...
create block;type=Reactor,Storage=1000000,...,name=Sink
create link;from=Tank,to=Sink,type=User_flow,flow=Qin.txt,name=Outlet
create parameter;type=Parameter,high=20,low=0.1,name=p_Cbase,prior_distribution=log-normal,value=1
setasparameter; object= Tank, parametername= p_Cbase, quantity= Tracer:constant_inflow_concentration
create observation;type=Observation,comparison_method=EMC,error_standard_deviation=0.1,error_structure=log-normal,expression=Tracer:concentration,name=Effluent_EMC,object=Tank,observed_data=emc_obs.txt,emc_weighting_object=Outlet,emc_weighting_expression=flow,emc_events=events.txt
```

`events.txt`:
```
// t_start, t_end
1.0, 2.5
4.0, 6.0
6.5, 6.8
```

`emc_obs.txt`:
```
1.5, 9.0
5.0, 7.0
```

Results:

* The modeled EMCs agree to within about 1% with an independent calculation
  of ∫CQ dt / ∫Q dt from the model's output file.
* **GA** converged to `p_Cbase = 3.97` (NSE on log-EMCs 0.97).
* **MCMC** gave a posterior median of 3.72, with a 95% interval of
  [3.14, 5.40].

---

## 7. Practical notes

* **Which object is the effluent?** For a BMP whose only outlet is an
  underdrain, weight by the underdrain link's flow and take the
  concentration in the block that link drains. If there is an overflow or
  bypass that is *included* in the monitored effluent, the EMC must be
  computed on the combined outflow. In that case, route both paths into a
  single outlet link or node, and observe that.
* **Influent EMCs** can be used the same way, e.g. to check a load model:
  weight by the inflow and observe the inflow concentration.
* **Event definition matters.** A window that is too long dilutes the modeled
  EMC with baseflow; one that is too short drops the tail of the hydrograph.
  Match the window to how the composite was actually sampled.
* **Few events, many parameters.** Each event gives one data point. Do not
  calibrate more parameters than the EMCs can constrain. Hold hydraulic
  parameters from a flow calibration, or share reaction parameters across
  sites.
* **σ and N.** The weight of an EMC observation in a joint likelihood grows
  with the number of events N. When it is combined with a densely sampled
  time-series observation, estimate each observation's σ rather than fixing
  them, so that neither data set dominates by sample count alone.
