Plant transpiration from a rooted soil profile
==============================================

Two models, same soil profile, differing only in where the evaporative demand
comes from.

  rooted_soil_profile.ohq          ET0 supplied as a time series (synthetic)
  rooted_soil_profile_penman.ohq   ET0 computed from measured 2010 weather

Both use a 1 ha field: a catchment surface receiving rainfall over three soil
layers (0.3 / 0.4 / 0.8 m) draining to a water table at 7.5 m.

Root distribution
-----------------
A source instance carries ONE root fraction, so a root distribution is built by
creating one source per layer, all sharing the same weather and Kcb:

    Crop_upper   root_fraction = 0.5   -> Root_zone_upper
    Crop_middle  root_fraction = 0.3   -> Root_zone_middle
    Crop_lower   root_fraction = 0.2   -> Root_zone_lower

The fractions should sum to 1.

1. rooted_soil_profile.ohq  (Transpiration_Time_Series)
-------------------------------------------------------
120 days, synthetic data: 567 mm of seasonal ET0 against 174 mm of rain in
seven events, so stress develops strongly. Ends at theta = 0.171 / 0.191 /
0.220 from a uniform 0.300, with stress coefficients 0.48 / 0.62 / 0.83 - the
layer carrying half the roots dries most and is most stressed.

2. rooted_soil_profile_penman.ohq  (Transpiration_Penman)
----------------------------------------------------------
1 June to 29 September 2010, driven by measured weather:

    solar_radiation_2010.csv   0 - 1013 W/m2, mean 304 in this window
    temperature_2010.csv       mean 26.1 C
    humidity_2010.csv          relative humidity as a fraction, 0.12 - 1.00
    wind_speed_2010.csv        mean 8.8 in the file's own units
    rainfall_2010.txt          270 mm over the window, depth in m per interval

Ends at theta = 0.193 / 0.201 / 0.231; the profile loses about 127 mm of stored
water while receiving 270 mm of rain.

Because the demand is now driven by real radiation, transpiration has a diurnal
cycle. Instantaneous rates are about 13.9 mm/day at midday and essentially zero
at night, which integrates to roughly 4.4 mm/day. Sampling the model at
midnight will correctly show almost no transpiration - look at the daily
pattern in the plot, not at a single instant.

Scale factors
-------------
solar_scale_fact = 0.5 converts incoming shortwave to the net radiation
available for evaporation. wind_scale_fact = 0.28 converts the wind column to
m/s on the assumption that it is recorded in km/h.

Both are judgement calls about this particular dataset and are worth checking
against your own metadata. Note that Examples/Bioretention uses 0.1 for both
with a similar 2010 dataset; that gives only about 1 mm/day of radiation-driven
demand, which is low for a 26 C summer, so the two examples do not agree. The
values here were chosen to give a physically plausible seasonal ET.

Note on evaporation
-------------------
These sources are TRANSPIRATION ONLY. Bare-soil evaporation is not included.
To add it, attach an Evapotranspiration_*(Soil) source to the top layer as well
- but do not attach a single-Kc evapotranspiration source to a layer that
already has a transpiration source, or the loss is counted twice.
