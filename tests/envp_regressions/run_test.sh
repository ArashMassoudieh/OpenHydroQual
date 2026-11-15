#!/usr/bin/env bash
# Template regressions found while building the CEE 466/566 course models:
# Activated_Sludge_Reactor without inflow/bottom_elevation, fixed_head without
# inflow, and Sorbed_Constituent in a Reactor (no volume). See build_models.py.
#
# Usage:
#   OHQ_CONSOLE=/path/to/OHQTerminal ./run_test.sh
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../.." && pwd)"
RESOURCES="${OHQ_RESOURCES:-$REPO_ROOT/resources/}"
PY="${PYTHON:-python3}"
if [ -z "${OHQ_CONSOLE:-}" ]; then
  echo "OHQ_CONSOLE is not set (path to an OpenHydroQual console)." >&2
  exit 2
fi
"$PY" "$HERE/build_models.py" "$RESOURCES"
cd "$HERE/generated"
for m in asr_particle fixed_head sorbed_reactor; do
  rm -f "${m}_output.txt"
  # not quiet: missing-property warnings are only printed without -q
  "$OHQ_CONSOLE" "$m.ohq" -r "$RESOURCES" > "${m}_console.log" 2>&1 || echo "$m: console exit $?" >&2
done
cd "$HERE"
"$PY" check_results.py
