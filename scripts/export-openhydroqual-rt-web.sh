#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") <destination-dir>

Create/update a standalone repository working tree from
templates/openhydroqual-rt-web in this monorepo.

Examples:
  $(basename "$0") ../openhydroqual-rt-web
  $(basename "$0") /mnt/3rd900/Projects/openhydroqual-rt-web
USAGE
}

if [[ ${1:-} == "-h" || ${1:-} == "--help" || $# -ne 1 ]]; then
  usage
  exit $([[ $# -eq 1 ]] && echo 0 || echo 1)
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SOURCE_DIR="$REPO_ROOT/templates/openhydroqual-rt-web"
DEST_DIR="$1"

if [[ ! -d "$SOURCE_DIR" ]]; then
  echo "Template directory not found: $SOURCE_DIR" >&2
  exit 1
fi

mkdir -p "$DEST_DIR"

# Sync template content, excluding VCS and local env/build artifacts.
rsync -a --delete \
  --exclude '.git/' \
  --exclude '.venv/' \
  --exclude '__pycache__/' \
  --exclude '.pytest_cache/' \
  "$SOURCE_DIR/" "$DEST_DIR/"

echo "Synced template to: $DEST_DIR"

echo
echo "Next steps (if creating a brand-new standalone repo):"
echo "  cd $DEST_DIR"
echo "  git init -b main"
echo "  git add ."
echo "  git commit -m 'Initial import from OpenHydroQual template'"
