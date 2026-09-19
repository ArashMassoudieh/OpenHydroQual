#!/usr/bin/env bash
# Runs every culvert example and reports its exit code.
# Usage: ./run_all.sh [path-to-OpenHydroQual-Console]
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CONSOLE="${1:-$ROOT/terminal/TOpenHydroQual/OpenHydroQual-Console}"
export OHQ_RESOURCES="$ROOT/resources"
cd "$(dirname "$0")"
[ -x "$CONSOLE" ] || { echo "Console binary not found: $CONSOLE"; exit 1; }
pass=0; fail=0
for f in T*.ohq; do
    printf '%-34s ' "$f"
    if out=$("$CONSOLE" "$PWD/$f" -q 2>&1); then
        echo "ok"; pass=$((pass+1))
    else
        echo "FAILED (exit $?)"; echo "$out" | sed 's/^/    /' | tail -5; fail=$((fail+1))
    fi
done
echo "---- $pass passed, $fail failed ----"
[ "$fail" -eq 0 ] && echo "Run 'python3 verify.py' for the regime and volume-balance report."
exit $((fail>0))
