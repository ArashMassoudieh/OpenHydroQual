#!/usr/bin/env bash
# Batch test of the immobile / sorbed constituent templates in
# resources/mass_transfer.json: mass conservation, the volume each
# concentration is expressed per, the exchange kinetics against closed-form
# solutions, and linear-sorption equilibrium (C -> C0/R, S -> K_d*C).
# See build_models.py and check_results.py for the cases and the formulas.
#
# Usage:
#   OHQ_CONSOLE=/path/to/OHQTerminal ./run_test.sh
#
# OHQ_CONSOLE is required: use a console built from the current checkout.
# (Consoles built before commit 5a00373 evaluate "(0-1)" as 0; the models
# here use plain numbers, so they do not depend on that fix.)
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../.." && pwd)"
RESOURCES="${OHQ_RESOURCES:-$REPO_ROOT/resources/}"
PY="${PYTHON:-python3}"
if [ -z "${OHQ_CONSOLE:-}" ]; then
  echo "OHQ_CONSOLE is not set (path to an OpenHydroQual console built from this checkout)." >&2
  exit 2
fi
"$PY" "$HERE/build_models.py" "$RESOURCES"
cd "$HERE/generated"
for m in immobile_chem sorbed_chem sorbed_desorp porosity_stoich; do
  rm -f "${m}_output.txt"
  "$OHQ_CONSOLE" "$m.ohq" -q > "${m}_console.log" 2>&1 || { echo "run failed: $m (see generated/${m}_console.log)" >&2; exit 1; }
done
cd "$HERE"
"$PY" check_results.py
