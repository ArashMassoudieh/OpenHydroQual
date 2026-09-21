## OpenHydroQual 2.0.7 (unreleased)

### Fixes

#### Constituent transport across links without `length` / `area`

The dispersive mass transfer that every Constituent adds to each link is

```
(diffusion_coefficient*area + dispersivity*flow)/length * (concentration.s - concentration.e)
```

Many link templates did not define `length` (and often `area`). A missing property is read as 0, with a single
`property 'length' does not exist in '<link>'` entry in the error log, so the term divided by zero. When such a link
carried flow in a model with constituents, the transport solve diverged. Depending on the model, the run either
stalled with *"The Jacobian Matrix is not full-ranked"* at a time step of about 1e-9, or kept running with spurious
mass transfer that instantly equalised the concentrations on both sides of the link. A common trigger was a pond
overflow weir (`wier` in the Stormwater control measures library) that failed as soon as the pond reached the crest.

Every link template now defines both properties. Where a template lacked them, they were added as hidden values:

- `length` = 1 (m), used only by the dispersion term
- `area` = 0, so there is no molecular diffusion across the structure

Advective transport (`flow * concentration`) is unchanged, and dispersion across these links is now finite and small.
For the two `Settling element interface` links, where a diffusion distance is physically meaningful, `length` is
shown in the property window so it can be set.

Affected templates:

| Library | Links |
|---|---|
| main_components | `Catchment_link`, `Reservoir_link_rule`, `User_flow` |
| Stormwater_control_measures | `wier`, `drain_pipe` |
| Pond_Plugin | `drain_pipe`, `orifice`, `darcy_connector`, `aggregate2aggregate_H_Link` (area only) |
| Pond_Plugin_Revised | `wier`, `drain_pipe` |
| Reservoir_operation | `Spillway` |
| Sewer_system | `soil2sewer`, `Catchment_link_MRM`, `Curb_cut`, `Sewer_pipe` and `sewer2pond` (area only) |
| Well | `Surface water to well` (area only) |
| mass_transfer | `Settling element interface`, `Settling element interface (time variable)` (length only) |
| open_channel | `Leaky Dam`, `Leaky Dam with pipes` |
| physicsbaseddatadrivenhydrology | `storage_unit_link` |
| pipe_pump_tank | `pump`, `pump_with_timeseries_control`, `Flow_to_user`; `pipe_1w`, `pipe_2w`, `valve` (area only) |
| plants | `Soil_to_plant_link` (area only) |
| precipitation_forcast | all five `Flow_control_device` variants |
| rainfall_runoff, rainfall_runoff_depthonly | `distributed_catchment_link`, `distributed_catchment_to_fixed_head_link`, `distributed_catchment_to_stream_link`, `Sheetflow_link` |
| unconfined_groundwater | `aquifer2well_link`, `aquifer2well_link_pump`; `UC_groundwater_link`, `unconfined_groundwater_to_fixedhead` (area only) |
| unsaturated_soil, _EC_logistic, _revised_model, _old, _bkup | `fixed_hydraulic_gradient_link` (length only) |
| urban_water_management, urban_water_management_old | all transfer and user-flow links |

Existing models need no changes. They pick up the new defaults when loaded.
