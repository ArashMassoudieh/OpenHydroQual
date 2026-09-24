# OpenHydroQual overview presentation

A general-audience deck on OpenHydroQual: what it is, the breadth of systems it
models, the command-line and embedded uses, inverse modelling, the model
compiler, and OpenHydroTwin.

Built PDF: `openhydroqual_overview.pdf`.

## Build

```
latexmk -pdf openhydroqual_overview.tex
```

`pdflatex openhydroqual_overview.tex` twice also works. Needs TeX Live with
`beamertheme-metropolis`, `pgfplots` and `tikz`.

To regenerate the drawn figures as well:

```
./make_figures.sh
```

## Layout

| file | what it is |
|---|---|
| `openhydroqual_overview.tex` | the slides |
| `preamble.tex` | theme, colours, listing styles, the figure-slot macros |
| `fig_*.tex` | standalone TikZ/pgfplots figures, one per file |
| `make_*.py` | generators that build figures or slide fragments from data |
| `make_figures.sh` | rebuilds every generated figure and installs it |
| `figures/` | every image the deck includes |

## Figure slots

Each image is a `\figslot{ID}{filename}{width}{description}`. If
`figures/<filename>` exists it is included; if not, a labelled placeholder is
drawn in its place so a missing figure is obvious instead of silent. To fill
one, drop the file into `figures/` under the name printed on the placeholder
and rebuild. No source edits.

## Generated figures

Four generators build content from source data rather than from a static image:

* `make_icon_grid.py` reads `resources/default_templates.list`, the same list
  the GUI's Add-plugin menu is built from, and writes `icongrid.tex` plus the
  template icons. Re-run it after adding a template.
* `make_paramdrift.py`, `make_cumvolume.py`, `make_eventzoom.py` read run
  outputs from the **OpenHydroTwin** repository, expected as a sibling of this
  one at `../../../OpenHydroTwin`. That repository is separate and may not be
  present. When it is missing, `make_figures.sh` says so, skips those three and
  keeps the committed PDFs, so the deck still builds.

Each generator records its exact input files and any caveats in its header
comment.

## Figure provenance

Drawn for this deck (TikZ/pgfplots, sources in this folder): the block/link
concept, the framework diagram, the toolchain, the codegen speed-up, the
inverse-modelling workflow, the recency kernel, the roadmap, and the three
OpenHydroTwin result figures listed above.

Reused from the OpenHydroQual repository: the logo, the wizard component
diagrams (`resources/Wizard_Scripts/Diagrams/`), the template icons
(`resources/Icons/`) and the sedimentation-basin tutorial screenshots
(`docs/figures/`).

Screenshots of the application, the Model Wizard, GreenInfraIQ and the
OpenHydroTwin viewer were captured for this deck.

Some result figures come from work reported in:

* Massoudieh (2023), *An extensible, plugin-based tool for modeling flow and
  reactive transport in water systems*, Environmental Modelling and Software
  164, 105707.
* Shakouri, Kiriazes and Massoudieh (2026), *OpenHydroTwin: a generic digital
  twin framework for stormwater infrastructure*.
* A copper column-sorption study (calibration and MCMC figures on the inverse
  modelling slides).

Check the publication status of those before distributing the deck outside
your own organisation.
