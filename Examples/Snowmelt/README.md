# Snow accumulation and melt example

This four-day example demonstrates the temperature-index snow components in
`resources/snowmelt.json`:

1. Ten millimetres of precipitation falls below freezing and accumulates as
   snow water equivalent.
2. Air temperature rises to 5 °C and the stored snow melts into the catchment.
3. A second ten-millimetre event falls above freezing and reaches the
   catchment directly as liquid rain.

From this directory, run:

```sh
../../terminal/TOpenHydroQual/build_lm/OpenHydroQual-Console snowmelt.ohq \
  --resources ../../resources --quiet
```

The generated `snowmelt_observed.txt` reports snow water equivalent, melt
flow, and catchment storage. The snowpack must rise during the cold event and
return to zero after thaw. This is a compact behavior example; the transition
temperatures and degree-day factor should be calibrated for a real basin.
