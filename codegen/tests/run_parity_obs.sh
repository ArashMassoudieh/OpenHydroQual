#!/usr/bin/env bash
# OpenHydroQual codegen - parameter (G1) + observation (G2) parity check.
# Generates the model, compiles tests/parity_obs.cpp against it, runs the
# interpreter (perturbed parameters, MCMC-style) and the generated model, and
# diffs every observation series + final storages.
#
# Usage: tests/run_parity_obs.sh <model.ohq> <ClassName> [perturbation]
# NOTE: generation and the run are done FROM THE MODEL'S FOLDER -- the
# interpreter resolves `addtemplate` file names relative to the cwd first
# (issues.md ISSUE 6).
set -euo pipefail

MODEL="$(realpath "${1:?model.ohq}")"
CLASS="${2:?ClassName}"
PERT="${3:-0.15}"

CG="$(cd "$(dirname "$0")/.." && pwd)"     # codegen/
ROOT="$(cd "$CG/.." && pwd)"               # repo root
RES="$ROOT/resources"
LIB="$ROOT/OHQLib/build"
OUT="$(mktemp -d)"
QT="${QT_DIR:-/home/arash/Qt/6.8.2/gcc_64}"; [ -d "$QT" ] || QT=/usr
QTLIBS="$(pkg-config --libs Qt6Core 2>/dev/null || echo "-L$QT/lib -lQt6Core")"

cd "$(dirname "$MODEL")"
echo "generating $CLASS from $MODEL ..."
"$CG/build/ohq_generate" "$MODEL" "$RES" "$OUT" "$CLASS"

echo "compiling parity_obs ..."
g++ -std=c++17 -O2 -fPIC \
  -I"$QT/include/x86_64-linux-gnu/qt6" -I"$QT/include/x86_64-linux-gnu/qt6/QtCore" -I"$QT/include" -I"$QT/include/QtCore" \
  -I"$ROOT/aquifolium/include" -I"$ROOT/aquifolium/src" -I"$ROOT/jsoncpp/include" \
  -I"$ROOT" -I"$ROOT/aquifolium/include/GA" -I"$ROOT/aquifolium/include/MCMC" \
  -I"$CG/runtime" -I"$OUT" \
  -DQ_JSON_SUPPORT -DGSL -DTerminal_version -Dubuntu_version -DARMA_USE_LAPACK -DARMA_USE_BLAS \
  -DGEN_HEADER="\"$CLASS.h\"" -DGEN_CLASS="$CLASS" \
  "$CG/tests/parity_obs.cpp" \
  -L"$LIB" -lOHQLib -Wl,-rpath,"$LIB" $QTLIBS \
  -o "$OUT/parity_obs"

echo "running parity_obs ..."
"$OUT/parity_obs" "$MODEL" "$RES" "$PERT"
