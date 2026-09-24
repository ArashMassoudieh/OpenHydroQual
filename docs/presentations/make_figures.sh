#!/bin/sh
# Rebuild the generated figures and install them into figures/.
#
# The three OpenHydroTwin generators read run outputs from a sibling
# repository that is not part of this one. If it is absent they are skipped
# and the committed PDFs in figures/ are kept, so the deck still builds.
set -e
cd "$(dirname "$0")"

TWIN_DATA="../../../OpenHydroTwin/deployments"

python3 make_icon_grid.py

FIGS="fig_concept fig_framework fig_toolchain fig_codegen fig_inverse fig_kernel fig_roadmap"

if [ -d "$TWIN_DATA" ]; then
    python3 make_paramdrift.py
    python3 make_cumvolume.py
    python3 make_eventzoom.py
    FIGS="$FIGS fig_paramdrift fig_cumvolume fig_eventzoom"
else
    echo "note: $TWIN_DATA not found; keeping the committed OpenHydroTwin figures"
fi

for f in $FIGS; do
    pdflatex -interaction=nonstopmode -halt-on-error "$f.tex" > "$f.buildlog" 2>&1
    echo "built $f.pdf"
done

install_fig() { [ -f "$1" ] && cp "$1" "$2"; }

install_fig fig_concept.pdf    figures/N1_block_link_concept.pdf
install_fig fig_toolchain.pdf  figures/N3_toolchain.pdf
install_fig fig_codegen.pdf    figures/N4_codegen_speedup.pdf
install_fig fig_inverse.pdf    figures/N5_inverse_workflow.pdf
install_fig fig_paramdrift.pdf figures/N6_drift_detection.pdf
install_fig fig_roadmap.pdf    figures/N7_roadmap.pdf
install_fig fig_framework.pdf  figures/N8_framework.pdf
install_fig fig_cumvolume.pdf  figures/N9_cumulative_volume.pdf
install_fig fig_eventzoom.pdf  figures/N10_event_zoom.pdf
install_fig fig_kernel.pdf     figures/N11_kernel.pdf
echo "installed into figures/"
