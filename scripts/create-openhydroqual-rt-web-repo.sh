#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") <destination-dir> <remote-url> [branch]

GitKraken-terminal friendly wrapper to create a standalone
openhydroqual-rt-web repository in one command.

Arguments:
  destination-dir   Local folder to create/update.
  remote-url        Git remote URL for origin.
  branch            Branch name (default: main).

Examples:
  $(basename "$0") ../openhydroqual-rt-web https://github.com/acme/openhydroqual-rt-web.git
  $(basename "$0") ../openhydroqual-rt-web git@github.com:acme/openhydroqual-rt-web.git main
USAGE
}

if [[ ${1:-} == "-h" || ${1:-} == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -lt 2 || $# -gt 3 ]]; then
  usage
  exit 1
fi

DEST_DIR="$1"
REMOTE_URL="$2"
BRANCH="${3:-main}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"$SCRIPT_DIR/export-openhydroqual-rt-web.sh" \
  --init-git \
  --branch "$BRANCH" \
  --remote "$REMOTE_URL" \
  --push \
  "$DEST_DIR"

echo
echo "Done. Open the repo in GitKraken: $DEST_DIR"
