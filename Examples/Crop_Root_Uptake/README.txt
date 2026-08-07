================================================================================
 Crop block with multi-layer root water uptake
 OpenHydroQual example -- plants.json (Crop, Crop_Penman, Root_uptake_link)
================================================================================

These two models demonstrate the demand-driven plant water uptake components.
A Crop block computes a potential transpiration from the canopy, and one
Root_uptake_link per soil layer distributes that demand over the profile
according to an exponential root density, reducing it layer by layer with the
Feddes water stress function.

    Tp  = ET0 * Kcb * (1 - exp(-k*LAI)) * area
    q_i = Tp * beta_i * alpha_i * compensation_factor

You never enter root fractions. Each link derives its own beta by integrating
the exponential root density between its layer's top and bottom, which it reads
from the layer's bottom_elevation and depth and the crop's surface_elevation
and rooting depth. Add or remove a layer, or let the roots grow deeper during a
season, and the fractions redistribute themselves and still sum to one.

--------------------------------------------------------------------------------
 1. crop_root_uptake.ohq -- root water compensation, side by side
--------------------------------------------------------------------------------

Two identical 1.5 m soil profiles under the same weather, each with its own
corn crop. The only difference is the crop's "compensation" setting:

    Corn_compensated     compensation = 1   (wet layers cover for dry ones)
    Corn_uncompensated   compensation = 0   (stress in a layer is simply lost)

Layout of each profile (surface at elevation 10 m):

    Field_A  (Catchment, receives Rain)
      |  Infiltration_A
    Soil_A1   0.0 - 0.3 m      -- Uptake_A1 --.
      |  Percolation_A1                        \
    Soil_A2   0.3 - 0.7 m      -- Uptake_A2 ----> Corn_compensated
      |  Percolation_A2                        /
    Soil_A3   0.7 - 1.5 m      -- Uptake_A3 --'
      |  Drainage_A
    Water_table_A  (fixed head at 8.2 m)

Rooting depth is 1.0 m, so Uptake_A3 covers only the top 0.3 m of the third
layer. The link clips itself at the root tip -- check its z_bottom output.

Forcing (150 days, starting 1 Jan 2000 = day 36526):
    rainfall.csv    100 mm in five storms over the first 15 days, then nothing
    et0.csv         2.5 mm/day rising to 6.5 mm/day and back

What to plot:
    Corn_*/soil_stress              root-weighted water availability, sum(beta*alpha)
    Corn_*/Tp                       potential transpiration
    Uptake_*/alpha                  per-layer Feddes stress
    Uptake_*/flow                   per-layer uptake
    Soil_*/theta                    the profile drying from the top down

Verified results at the end of the 135-day drought:

                              compensated    uncompensated
    soil_stress                   0.084          0.587
    sum(uptake) / Tp              21.9 / 21.9    12.9 / 21.9
    theta  layer 1                0.100          0.107
           layer 2                0.100          0.141
           layer 3                0.119          0.184
    profile storage (m3)          1655           2354

The compensated crop has mined the whole profile to wilting point and is still
meeting its full demand, drawing 98% of it from the deepest layer. The
uncompensated crop is stuck at 59% of potential even though its profile still
holds 700 m3 more water, because most of its root mass sits in layers that have
dried out and it cannot shift the demand elsewhere.

Note that compensation here is unlimited: as the profile dries, the
compensation factor grows without bound (about 12 at the end of this run) and
concentrates the entire demand on whichever layer is still wet. SWAP and APEX
behave the same way in principle, but some implementations cap the factor. If
you want a limit, reduce it by lowering h_wilting or by turning compensation
off.

--------------------------------------------------------------------------------
 2. crop_growing_season_penman.ohq -- a corn season with real weather
--------------------------------------------------------------------------------

One 1.5 m profile in four layers under a Crop_Penman block, which computes ET0
internally from measured solar radiation, air temperature, relative humidity and
wind speed instead of taking it from a time series.

    Corn_field (Catchment, receives Rain_2010)
      |
    Topsoil         0.0 - 0.2 m   -- Uptake_topsoil --------.
    Subsoil_upper   0.2 - 0.5 m   -- Uptake_subsoil_upper ---\
    Subsoil_lower   0.5 - 1.0 m   -- Uptake_subsoil_lower ----> Corn
    Deep_subsoil    1.0 - 1.5 m   -- Uptake_deep_subsoil ----'
      |
    Water_table  (fixed head at 7.0 m)

The crop calendar runs from 1 April to 1 October 2010 (days 40269 to 40452):

    lai_corn.csv          LAI 0 at planting, 5.0 at the start of July, 0.5 at harvest
    root_depth_corn.csv   added to root_depth_constant = 0.2 m, so roots reach
                          0.2 m at planting and 1.2 m by the start of July

Both are supplied as time series added to the constant part of the quantity, so
you can leave either one empty for a static canopy or a static root system.

Weather data are half-hourly observations for 2010, copied from the
Climate_Data collection:

    solar_radiation_2010.csv, temperature_2010.csv,
    wind_speed_2010.csv, humidity_2010.csv, rainfall_2010.txt

Verified results (values are instantaneous, sampled at 14:24 local):

    date        LAI    root_depth   ET0        Tp         sum(beta)   soil_stress
    17 May      1.74     0.55 m     13.3 mm/d   9.9 mm/d   1.000000     0.996
    16 Jul      4.97     1.20 m     14.3 mm/d  15.6 mm/d   1.000000     0.949
    14 Sep      2.04     1.20 m     12.9 mm/d  10.5 mm/d   1.000000     0.993

The root fractions sum to exactly one at every stage even while the rooting
front is advancing, and the share of each layer shifts downward as the roots
grow: Uptake_topsoil holds 0.600 of the root system in May and 0.328 in July.
Drainage to the water table is negative through the summer, which is capillary
rise supporting the crop from below.

IMPORTANT -- solar_scale_fact is 0.5 in this model. The Penman formulation in
OpenHydroQual drives the radiation term with incoming shortwave radiation
rather than net radiation, so it overestimates ET0 by roughly a factor of two
on clear days. The scale factor is the intended calibration knob. Set it from
your own site data before using this for anything quantitative.

--------------------------------------------------------------------------------
 Things worth knowing
--------------------------------------------------------------------------------

Only draw Root_uptake_links to a Crop block. The compensation factor is built
from beta.v and beta_alpha.v, which average over all of the block's links, so
attaching any other link type to a Crop corrupts the denominator.

Plot soil_stress, not relative_transpiration, as the crop stress index. With
compensation on, total uptake equals Tp by construction, so
relative_transpiration is 1 apart from the lag of the canopy reservoir
(K_plant), which makes it a noisy diagnostic. It is meaningful when
compensation is 0. soil_stress is the lag-free quantity.

These components model transpiration only. Kcb is the FAO-56 basal crop
coefficient and excludes soil evaporation. If you need evaporation from the
surface, add an Evapotranspiration_*(Soil) source to the top layer. Be aware
that the stock Evapotranspiration_Time_Series (Soil) source currently defines
its rate as corr_fact*timeseries while the source value is already multiplied
by the series, so the series is applied twice; neither example here uses it.

Both models are also documented, with the full set of equations, in
docs/plant_water_uptake.tex.
