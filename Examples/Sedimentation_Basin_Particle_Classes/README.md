# Sedimentation basin — four particle classes, no underflow

A quiescent sedimentation basin discretized into four layers, fed by a constant
influent and drained by an equal effluent. **Nothing is withdrawn from the
bottom** (`underflow = 0`), so the only downward transport is gravitational
settling and solids accumulate on the floor.

Open `Sedimentation_Basin_Particle_Classes.ohq`.

## Layout

```
Influent ──Fixed flow, 2000 m³/d──▶ Sed_Basin ──Effluent flow, 2000 m³/d──▶ Effluent
                                    (Layer1)                    (Layer1)
```

`Sed_Basin` is a `Clarifier (4 layers)` composite. Influent and effluent both
attach to `Layer1`; no `Sludge flow` link is attached, and `underflow = 0`
zeroes the advective flow on all three internal interfaces.

## Design point

| Quantity | Value |
| --- | --- |
| Inflow = outflow, `Q` | 2000 m³/day |
| Plan area, `A` | 400 m² |
| Overflow rate, `Q/A` | **5 m/day** |
| Layer depth / volume | 1 m / 400 m³ each (4 m, 1600 m³ total) |
| Layer bottom elevations | 0, −1, −2, −3 m |
| Influent concentration | 50 g/m³ per class (200 g/m³ total TSS) |
| Underflow | **0** |
| Simulated period | day 36526 → 36556 (30 days) |

## Particle classes

Four `Particle` constituents chosen to straddle the 5 m/day overflow rate, so
removal spans nearly the whole range:

| Class | Settling velocity | Steady-state effluent | Removal |
| --- | --- | --- | --- |
| `P1` | 0.5 m/day | 45.45 g/m³ | 9.1 % |
| `P2` | 2 m/day | 35.71 g/m³ | 28.6 % |
| `P3` | 10 m/day | 16.67 g/m³ | 66.7 % |
| `P4` | 50 m/day | 4.55 g/m³ | 90.9 % |

## What to expect

Layer 1 is the only layer with advective throughflow, so at steady state it
satisfies

```
Q·C_in = Q·C₁ + vs·A·C₁     ⟹     C₁ = C_in / (1 + vs·A/Q)
```

Layers 2 and 3 pass the settling flux straight through, so `C₂ = C₃ = C₁` once
steady. Layer 4 has no outlet and accumulates indefinitely — its concentration
climbs roughly linearly and represents the sludge blanket.

The simulated results reproduce the analytical values above to five significant
figures, and the mass balance closes (for `P3`: 100 000 g/day in = 33 333 g/day
out + 66 667 g/day settled).

**Reading the output:** effluent quality is `Sed_Basin__Layer1_<P>:concentration`.
The `Effluent_<P>:concentration` column is the concentration inside the
100 000 m³ receiving reservoir, which is still filling and is *not* the effluent
quality.

## Varying the example

- Give the basin a sludge draw: attach a `Sludge flow` link from `Sed_Basin` and
  set `underflow` to that rate. Layer 4 then reaches a steady blanket instead of
  accumulating.
- Change `surface_area` to move the overflow rate `Q/A`; removal of every class
  shifts with it.
- Add more classes, or replace the four with a measured settling-velocity
  distribution.
