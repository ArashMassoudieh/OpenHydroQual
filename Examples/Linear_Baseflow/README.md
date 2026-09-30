# Linear threshold baseflow

This example checks the `Linear_baseflow` connector in `groundwater.json`.
It is an alternative to the head-gradient-driven `groundwater_to_stream`
connector. Do not place both connectors in parallel between the same aquifer
and stream unless two physical discharge pathways are intentionally being
represented.

The connector implements

`Q = k max(S - S_min, 0)`,

where `S` is groundwater storage, `S_min = min_moisture_content * volume`, and
`k = recession_rate`. With no recharge, active storage follows
`S_active(t) = S_active(0) exp(-k t)`.

In this example, groundwater volume is 10,000 m3, initial active storage is
1,000 m3, and `k` is 0.01/day. Baseflow therefore begins at 10 m3/day. At day
100 the analytical active storage is 367.879 m3 and groundwater storage is
3,367.879 m3. Small differences reflect the numerical integration and output
time selected by the adaptive solver.
