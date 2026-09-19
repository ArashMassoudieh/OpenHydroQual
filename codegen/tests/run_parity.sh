#!/usr/bin/env bash
# OpenHydroQual codegen - numerical parity check
# Generates a standalone model from a .ohq, compiles it into the parity harness,
# runs interpreter vs generated, and diffs per-block final state.
#
# Usage: tests/run_parity.sh <model.ohq> <ClassName> [stateVar]
# Run from the codegen/ directory. Requires libOHQLib.so and Qt6.
set -euo pipefail

MODEL="${1:?model.ohq}"
CLASS="${2:?ClassName}"
STATEVAR="${3:-Storage}"

CG="$(cd "$(dirname "$0")/.." && pwd)"     # codegen/
ROOT="$(cd "$CG/.." && pwd)"               # repo root
RES="$ROOT/resources"
LIB="$ROOT/OHQLib/build"
QT="${QT_DIR:-/home/arash/Qt/6.8.2/gcc_64}"
OUT="$(mktemp -d)"

echo "generating $CLASS from $MODEL ..."
"$CG/build/ohq_generate" "$MODEL" "$RES" "$OUT" "$CLASS" "$STATEVAR"

echo "compiling parity harness ..."
g++ -std=c++17 -O2 -fPIC \
  -I"$QT/include" -I"$QT/include/QtCore" \
  -I"$ROOT/aquifolium/include" -I"$ROOT/aquifolium/src" -I"$ROOT/jsoncpp/include" \
  -I"$ROOT" -I"$ROOT/aquifolium/include/GA" -I"$ROOT/aquifolium/include/MCMC" \
  -I"$CG/runtime" -I"$OUT" \
  -DQ_JSON_SUPPORT -DGSL -DTerminal_version -Dubuntu_version -DARMA_USE_LAPACK -DARMA_USE_BLAS \
  -DGEN_HEADER="\"$CLASS.h\"" -DGEN_CLASS="$CLASS" \
  "$CG/tests/parity_harness.cpp" \
  -L"$LIB" -lOHQLib -Wl,-rpath,"$LIB" -L"$QT/lib" -lQt6Core -Wl,-rpath,"$QT/lib" \
  -o "$OUT/parity"

echo "running parity ..."
"$OUT/parity" "$MODEL" "$RES"
