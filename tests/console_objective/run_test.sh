#!/usr/bin/env bash
# Regression test for objective-function reporting by OpenHydroQual-Console.
# Usage: OHQ_CONSOLE=/path/to/current/OpenHydroQual-Console ./run_test.sh
# Optionally set OHQ_KERNEL to a generated library for objective.ohq to test
# the generated-kernel path as well.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../.." && pwd)"
RESOURCES="${OHQ_RESOURCES:-$REPO_ROOT/resources}"

if [ -z "${OHQ_CONSOLE:-}" ] || [ ! -x "$OHQ_CONSOLE" ]; then
  echo "OHQ_CONSOLE must name an executable built from the current checkout." >&2
  exit 2
fi

if [ -n "${OHQ_KERNEL:-}" ] && [ ! -f "$OHQ_KERNEL" ]; then
  echo "OHQ_KERNEL does not name a generated kernel library." >&2
  exit 2
fi

workdir="$(mktemp -d /tmp/ohq-console-objective.XXXXXX)"
trap 'rm -rf "$workdir"' EXIT

check_objective() {
  local label="$1"
  shift
  local output line value
  output="$($OHQ_CONSOLE "$HERE/objective.ohq" -q -r "$RESOURCES" -w "$workdir" "$@")"
  line="$(printf '%s\n' "$output" | grep -F "Objective function 'Integrated Baseflow':")"
  value="${line##*: }"

  python3 - "$label" "$value" <<'PY'
import math
import sys

label = sys.argv[1]
actual = float(sys.argv[2])
expected = 1000.0 * (1.0 - math.exp(-1.0))
# The adaptive solver and end-of-step rate sampling differ slightly from the
# continuous closed form; keep the tolerance tight enough to reject a missing
# or stale objective while allowing that expected discretization error.
if not math.isclose(actual, expected, rel_tol=2e-3, abs_tol=0.05):
    raise SystemExit(f"objective mismatch: got {actual}, expected {expected}")
print(f"{label} objective reporting passed: {actual:.6f}")
PY
}

check_objective interpreter
if [ -n "${OHQ_KERNEL:-}" ]; then
  check_objective kernel --kernel "$OHQ_KERNEL"
fi
