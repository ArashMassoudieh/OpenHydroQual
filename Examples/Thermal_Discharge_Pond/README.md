# Thermal discharge to a flow-through pond

This example demonstrates the **Heat (temperature)** constituent and the
**Surface heat exchange** source in `resources/mass_transfer.json`.

A completely mixed pond (200,000 m³, 100,000 m² surface) receives
50,000 m³/day of river water at 15 °C and exchanges heat with 20 °C air
(K = 30 W/m²/°C). From day 10 a power plant adds 20 MW of waste heat through
the pond's time-variable heat input (`plant_heat.csv`, kW).

The Heat constituent stores the heat content ρ·c_p·V·T (MJ) of each block and
reports the temperature T (°C), with ρ·c_p = 4.186 MJ/m³/°C. The pond's energy
balance is

    ρ·c_p·V·dT/dt = ρ·c_p·Q·(T_in − T) + 86.4·H + 0.0864·K·A·(T_air − T)

with H in kW and K in W/m²/°C. For constant inputs T relaxes exponentially
toward

    T_eq = (a·T_in + 86.4·H + b·T_air) / (a + b),   τ = ρ·c_p·V / (a + b)

where a = ρ·c_p·Q = 209,300 and b = 0.0864·K·A = 259,200 MJ/day/°C:
17.77 °C before the plant starts, 21.45 °C after, τ = 1.79 days. The plant
alone would raise a stream by H/(ρ·c_p·Q) = 8.26 °C; surface exchange cuts the
rise to 3.69 °C. `exact_temperature.csv` tabulates the exact solution; the
model agrees within 0.004 °C.

Run from this directory:

```sh
CONSOLE=/path/to/built/OpenHydroQual-Console
"$CONSOLE" thermal_discharge_pond.ohq --resources ../../resources --quiet
```

Compare `Pond_Temperature:concentration` in `output.txt` with
`exact_temperature.csv`. The heat budget terms are `Pond_Temperature:inflow_loading`,
`Pond_Temperature:heat_input`, `Pond_Temperature:external_source` (surface
exchange, positive when the pond gains heat) and
`Outflow_Temperature:masstransfer` (all MJ/day).

The same example, with plots, is in the examples catalog:
https://examples.openhydroqual.com/examples/thermal-discharge-pond/
