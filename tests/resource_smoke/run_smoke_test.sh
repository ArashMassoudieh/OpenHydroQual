#!/usr/bin/env bash
# Minimal regression smoke test for the riparian_urban_hru.json /
# unconfined_groundwater.json additions (hillslope_width_scale/
# hillslope_length_scale, Riparian_Urban_HRU_OneWayRecharge,
# Riparian_Urban_HRU_SymmetricKRecharge). See build_smoke_models.py's
# docstring for why this lives here rather than in codegen/tests/ (that
# directory tests the codegen pipeline itself, not resource templates).
#
# Usage:
#   OHQ_CONSOLE=/path/to/OpenHydroQual-Console ./run_smoke_test.sh
#
# OHQ_CONSOLE is REQUIRED -- there is deliberately no default-path guess.
# An earlier version defaulted to terminal/TOpenHydroQual/build-fresh/,
# which turned out to be a stale build whose expression parser cannot
# handle scientific notation ("1e-12", used in several existing resource
# templates' own criteria, not anything added here) -- it failed with
# "property '1e' does not exist" and a nonzero exit code that the per-model
# loop caught correctly, but the misleading part was that stale output
# files from a *previous successful run* were still sitting in generated/
# and got read by check_smoke_invariants.py, which printed PASS messages
# against 2-day-old data. Build a console from the CURRENT checkout (see
# the OHQLib/codegen/console build commands in OHQ_Model/audit_2026-10-26/
# MANIFEST.md for the exact recipe) and pass it explicitly -- never assume
# a pre-existing binary on disk is still compatible or fresh.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../.." && pwd)"
RESOURCES="${OHQ_RESOURCES:-$REPO_ROOT/resources/}"

if [ -z "${OHQ_CONSOLE:-}" ]; then
  echo "OHQ_CONSOLE is not set." >&2
  echo "Build a console from the current checkout and pass it explicitly, e.g.:" >&2
  echo "  OHQ_CONSOLE=/path/to/fresh/OpenHydroQual-Console $0" >&2
  echo "Do not point this at an arbitrary pre-existing build -- an older build" >&2
  echo "may be incompatible (confirmed: one older build's expression parser" >&2
  echo "cannot handle scientific notation used by several existing templates)." >&2
  exit 2
fi
CONSOLE="$OHQ_CONSOLE"
if [ ! -x "$CONSOLE" ]; then
  echo "OpenHydroQual-Console not found/executable at $CONSOLE" >&2
  exit 2
fi

cd "$HERE/generated"
fail=0
for model in smoke_default_baseline smoke_default_width_scale_half smoke_default_length_scale_double smoke_oneway_recharge smoke_symmetric_k_recharge; do
  echo "=== $model ==="
  # Remove any output from a previous run FIRST, so a failed solve can
  # never leave stale data behind for check_smoke_invariants.py to read
  # and report a false PASS against.
  rm -f "${model}_OHQ_output.txt" "${model}_observed.txt"
  if ! "$CONSOLE" "$model.ohq" -q -r "$RESOURCES"; then
    echo "FAIL: $model did not solve cleanly" >&2
    fail=1
    continue
  fi
  if [ ! -s "${model}_OHQ_output.txt" ]; then
    echo "FAIL: $model produced no output" >&2
    fail=1
  fi
done

if [ "$fail" -ne 0 ]; then
  echo "One or more models failed to solve -- skipping invariant checks" \
       "(their output is missing or stale, not meaningful to check)." >&2
  echo "SMOKE TEST FAILED"
  exit 1
fi

echo "=== checking type-specific invariants ==="
if ! python3 "$HERE/check_smoke_invariants.py"; then
  fail=1
fi

if [ "$fail" -ne 0 ]; then
  echo "SMOKE TEST FAILED"
  exit 1
fi
echo "SMOKE TEST PASSED"
