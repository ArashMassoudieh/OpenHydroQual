# OpenHydroQual presentation for Dewberry

## Build

```
latexmk -pdf main.tex
```

(`pdflatex main.tex` twice also works. Toolchain: TeX Live with
`beamertheme-metropolis`, `pgfplots`, `tikz`, all already installed here.)

## How the figure slots work

Every image on a slide is a `\figslot{ID}{filename}{width}{description}`.

* If `figures/<filename>` **exists**, it is included.
* If it **does not**, a dashed placeholder box is drawn on the slide showing
  the ID, the expected filename and a description of what to capture, and the
  item is added to the **Figure checklist** slide at the end of the deck.

So to fill a slot: **drop the file into `figures/` under the exact name shown
on the placeholder and rebuild.** No source edits.

`.png` and `.pdf` both work. For a vector figure, convert first:

```
inkscape --export-type=pdf --export-filename=figures/N1_block_link_concept.pdf drawing.svg
```

## Already supplied (copied from the OpenHydroQual / OpenHydroTwin repos)

| File | Source |
|---|---|
| `F1_logo.png` | `OpenHydroQual/OHQ.png` |
| `F2_bioretention_schema.png` | `OpenHydroTwin/Figures/` |
| `F3_wetland_schematic.png` | `OpenHydroTwin/Figures/` (not yet placed on a slide) |
| `F4_twin_architecture.pdf` | `OpenHydroTwin/Figures/fig1_architecture.svg` |
| `F5_assimilation_cycle.pdf` | `OpenHydroTwin/Figures/` |
| `F6_truth_vs_assim.png` | `OpenHydroTwin/deployments/paper_truth_vs_assim_drift.png` |
| `F7_parameter_history.png` | `.../paper_parameter_history_drift.png` |
| `F8_physical_parameters.png` | `.../paper_physical_parameters_drift.png` |
| `F9_event_zoom.png` | `.../paper_event_zoom_drift.png` |
| `F10_fitness_history.png` | `.../paper_fitness_history_drift.png` |
| `F11_*.pdf` (7) | `resources/Wizard_Scripts/Diagrams/*.svg` |
| `F12*.png` (4) | `docs/figures/` (sedimentation-basin tutorial) |

**Check before presenting:** the `F6` to `F10` panels were taken from the
`_drift` deployment. Confirm that is the run you want to show.

## Hand-drawn figures

`fig_concept.tex` (N1, the block/link/source picture) and `fig_framework.tex`
(N8, the framework diagram, after Fig. 1 of the 2023 EMS paper) are TikZ
sources in this folder. Rebuild and install both with:

```
./make_figures.sh
```

## The template montage

`make_icon_grid.py` reads `resources/default_templates.list`, the same list the
GUI's Add-plugin menu is built from, copies each template icon into
`figures/icons/` and writes `icongrid.tex`. Re-run it after adding a template:

```
python3 make_icon_grid.py
```

## Still outstanding

See the **Figure checklist** slide at the end of the built PDF. It is generated
from the source, so it is always current.

Groups:
* `S*`: GUI screenshots you need to capture
* `T*`: terminal captures (optional; mock-ups are already typeset on the slides)
* `N*`: new graphics to draw (TikZ/pgfplots)
* `R1`: validation against a reference model (HEC-HMS / SWMM)
