# Snow accumulation and temperature-index melt

`resources/snowmelt.json` adds an explicit snow water-equivalent store to
OpenHydroQual. It contains the following component types:

- `Liquid_Precipitation` and `Snowfall` partition a common precipitation
  series with a linear temperature transition. Their fractions are
  complementary, so precipitation volume is conserved.
- `Air_Temperature` provides temperature forcing to a snowpack.
- `Snowpack` stores solid precipitation as water-equivalent volume and reports
  snow water equivalent as `Storage/area`.
- `Snowmelt_link` releases `area * degree_day_factor * max(T-T_melt, 0)` into a
  receiving catchment. The link is storage-limited, so it cannot melt more
  water than the snowpack holds. The pervious and impervious link variants use
  the same equation and expose the two surfaces of a mixed HRU.

The default phase transition is all snow at or below -1 °C, all rain at or
above 1 °C, and linear between them. The default melt threshold is 0 °C and
the degree-day factor is 0.003 m/day/°C. These are initial estimates and should
be calibrated or replaced with locally supported values.

This is a temperature-index model. It does not represent canopy interception,
snow redistribution, cold content, refreezing, liquid-water retention in the
pack, sublimation, or a surface-energy balance. Use an energy-balance model
when radiation, humidity, wind, and snow thermal state are material to the
study.
