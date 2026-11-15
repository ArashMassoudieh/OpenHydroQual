#!/usr/bin/env bash
# Test of the Heat (temperature) constituent and the Surface heat exchange
# source in resources/mass_transfer.json: mixing of two inflows, a heater
# (dT = H/(rho*c_p*Q)), surface exchange with the air, and time-series inputs,
# each against its closed-form solution. See build_models.py and
# check_results.py for the cases and the formulas.
#
# Usage:
#   OHQ_CONSOLE=/path/to/OHQTerminal ./run_test.sh
#
# Consoles built before commit 42c0824 evaluate a source's coefficient
# without the source context in some calls and log "property '...' does not
# exist in 'Tank'" (exit code 4) for the surface/timeseries cases, although
# the results are right. Set ALLOW_SOURCE_CONTEXT_ERRORS=1 to accept exit
# code 4 from such a console.
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
for m in mixing heater surface timeseries; do
  rm -f "${m}_output.txt"
  rc=0
  "$OHQ_CONSOLE" "$m.ohq" -q -r "$RESOURCES" > "${m}_console.log" 2>&1 || rc=$?
  if [ "$rc" -ne 0 ] && ! { [ "$rc" -eq 4 ] && [ "${ALLOW_SOURCE_CONTEXT_ERRORS:-0}" = 1 ] && [ -f "${m}_output.txt" ]; }; then
    echo "run failed: $m (exit $rc, see generated/${m}_console.log)" >&2
    exit 1
  fi
done
cd "$HERE"
"$PY" check_results.py
