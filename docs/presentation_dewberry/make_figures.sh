#!/bin/sh
# Rebuild the hand-drawn TikZ figures and drop them into figures/.
set -e
cd "$(dirname "$0")"
python3 make_icon_grid.py
python3 make_paramdrift.py
python3 make_cumvolume.py
python3 make_eventzoom.py

for f in fig_concept fig_framework fig_toolchain fig_codegen fig_inverse fig_roadmap fig_paramdrift fig_cumvolume fig_eventzoom; do
    pdflatex -interaction=nonstopmode -halt-on-error "$f.tex" > "$f.buildlog" 2>&1
    echo "built $f.pdf"
done
cp fig_concept.pdf   figures/N1_block_link_concept.pdf
cp fig_toolchain.pdf figures/N3_toolchain.pdf
cp fig_codegen.pdf   figures/N4_codegen_speedup.pdf
cp fig_inverse.pdf   figures/N5_inverse_workflow.pdf
cp fig_paramdrift.pdf figures/N6_drift_detection.pdf
cp fig_cumvolume.pdf  figures/N9_cumulative_volume.pdf
cp fig_eventzoom.pdf  figures/N10_event_zoom.pdf
cp fig_roadmap.pdf   figures/N7_roadmap.pdf
cp fig_framework.pdf figures/N8_framework.pdf
echo "installed into figures/"
