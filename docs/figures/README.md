# Screenshots for `sedimentation_basin_tutorial.tex`

Drop PNGs here with these exact names and they are picked up automatically on
the next `pdflatex` run. Until a file exists, the tutorial renders a labelled
placeholder box in its place, so the document always compiles.

| File | Status | What to capture |
| --- | --- | --- |
| `01-add-plugin.png` | done | The Select Plugin dialog (File ▸ Preferences ▸ Add plugin) with **Wastewater processes** selected |
| `02-particle-properties.png` | done | Object Browser ▸ Constituents showing P1–P4, with P1 selected and Settling velocity = 0.5 m/day |
| `03-basin-properties.png` | done | The `Clarifier (4 layers)` property panel showing underflow = 0, floor elevation, depth and total volume |
| `04-diagram.png` | done | The finished canvas: Influent → Sed_Basin → Effluent |

Screenshots are cropped to the region of interest and given a 1 px grey border.
Wide canvas shots read better cropped to the nodes themselves rather than left
as the full application window.

Rebuild with:

    pdflatex sedimentation_basin_tutorial.tex   # twice, for the table of contents

`03-basin-properties.superseded` is the pre-change shot, kept only for
reference. It predates deriving the layer elevations from the floor elevation
and depth, so it shows properties (`Elevation of the interface between layers
1 and 2`, `Initial volume of water in layer 1 (top)`, …) that the composite no
longer has. It is not used by the document and can be deleted.
