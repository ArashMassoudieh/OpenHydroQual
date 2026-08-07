## OpenHydroQual 2.0.6

Download the Linux package below, or visit [www.OpenHydroQual.com](https://www.openhydroqual.com).

### Composite components

Several blocks and the links between them can now be grouped into a single component that behaves as one block in the interface. The solver still treats every member individually, so the numerics are unchanged, but the diagram, the property window and the results menus present the group as a unit. Composites can be copied, deleted and ungrouped as a whole, and models save either with composites intact or expanded into their members, so files remain readable by earlier versions.

Two composites ship with this release:

- **Clarifier** (wastewater library) — two settling compartments with an interface link, exposing separate effluent and sludge ports
- **Hydrologic Response Unit** — a catchment, three soil layers and a groundwater cell, with all layer geometry derived from the depth to groundwater

Documentation: https://openhydroqual.com/knowledge-base/theory-and-equations/composite-components/

### Plant water uptake

The plants library is complete and offers two approaches.

Transpiration **sources** attach to individual soil layers and scale a reference evapotranspiration by a crop coefficient, a stress function and a user-supplied root fraction.

A **Crop** block computes the plant's potential transpiration as `ET0 * Kcb * (1 - exp(-k*LAI)) * area`, and one **Root_uptake_link** per soil layer distributes that demand over the profile. Each link derives its own root fraction by integrating an exponential root density over its layer's geometry, so root fractions are never entered by hand and remain consistent when layers are added or when roots deepen through a season. Uptake is reduced layer by layer by the Feddes stress function, and an optional compensation mechanism lets layers that are still wet make up for layers that have dried out. Reference evapotranspiration is either supplied as a time series or computed internally by the Penman combination equation.

New types: `Crop`, `Crop_Penman`, `Root_uptake_link`, `Transpiration_Time_Series (Soil)`, `Transpiration_Penman (Soil)`

Documentation: https://openhydroqual.com/knowledge-base/theory-and-equations/plant-plugin/

### SCS Curve Number catchment

The Curve Number method has been recast as a system of differential equations so that it runs continuously rather than event by event, and the reformulation reproduces the classical algebraic method exactly. Runoff is routed to the outlet through a Nash cascade tuned to match the NRCS unit hydrograph peak, with the residence time derived from hydraulic length, slope and curve number. Between storms the abstraction store recovers through an exponential drying term. Catchments come in five-reservoir and three-reservoir variants and can be chained.

New types: `CN_Catchment`, `CN_Catchment_3`, `CN_retention`, `CN_reservoir`, `CN_partition_link`, `CN_cascade_link`, `CN_outlet`

Documentation: https://openhydroqual.com/knowledge-base/theory-and-equations/cn-curve-method/

### Calibration and uncertainty

- Least-squares calibration now uses a properly scaled Gaussian negative log-likelihood, in linear and log space, so the estimated error standard deviation is meaningful
- New **Weighted Least Squares** comparison method, weighting observations under a temporal kernel that favours recent data
- Coefficient of determination and Nash–Sutcliffe efficiency reported alongside the objective function
- Realization generation can apply Ornstein–Uhlenbeck correlated noise instead of independent noise
- Genetic algorithm settings file handling fixed; guards added to several MCMC edge cases
- Genetic algorithm runs and standalone runs now produce identical outputs

### Programmatic use

OpenHydroQual is available as a shared library (`OHQLib`) with both qmake and CMake builds, including a Windows DLL, and **Python bindings** (`openhydroqual_py`) for building, parameterising, solving and saving models from Python.

### Other additions and fixes

- Street gutter segments and curb cuts in the sewer library
- Green infrastructure upscaling block
- Uncertain flow control device for rainfall forecasting
- A source of non-determinism in the solver removed; repeated runs of the same model now give identical results
- `System::Solve` refactored; results loading improved
- macOS and Windows build fixes

### Examples

New example models ship in `Examples/`: `Crop_Root_Uptake`, `CN_Catchment`, `Hydrologic_Response_Unit`, `Plant_Transpiration` and `ASM_Clarifier_Composite`.
