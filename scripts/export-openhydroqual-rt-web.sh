#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") [options] <destination-dir>

Create/update a standalone repository from templates/openhydroqual-rt-web.

Options:
  --init-git                Initialize a git repo and create an initial commit.
  --branch <name>           Branch name to initialize/use with --init-git (default: main).
  --remote <url>            Configure origin remote URL (requires --init-git).
  --push                    Push to origin/<branch> after commit (requires --remote and --init-git).
  -h, --help                Show this help.

Examples:
  $(basename "$0") ../openhydroqual-rt-web
  $(basename "$0") --init-git --remote https://github.com/acme/openhydroqual-rt-web.git ../openhydroqual-rt-web
  $(basename "$0") --init-git --remote https://github.com/acme/openhydroqual-rt-web.git --push ../openhydroqual-rt-web
USAGE
}

INIT_GIT=false
PUSH=false
BRANCH="main"
REMOTE_URL=""
DEST_DIR=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      usage
      exit 0
      ;;
    --init-git)
      INIT_GIT=true
      shift
      ;;
    --push)
      PUSH=true
      shift
      ;;
    --branch)
      [[ $# -ge 2 ]] || { echo "Missing value for --branch" >&2; exit 1; }
      BRANCH="$2"
      shift 2
      ;;
    --remote)
      [[ $# -ge 2 ]] || { echo "Missing value for --remote" >&2; exit 1; }
      REMOTE_URL="$2"
      shift 2
      ;;
    --*)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
    *)
      if [[ -n "$DEST_DIR" ]]; then
        echo "Unexpected extra argument: $1" >&2
        usage
        exit 1
      fi
      DEST_DIR="$1"
      shift
      ;;
  esac
done

if [[ -z "$DEST_DIR" ]]; then
  usage
  exit 1
fi

if [[ "$PUSH" == true && "$INIT_GIT" != true ]]; then
  echo "--push requires --init-git" >&2
  exit 1
fi

if [[ -n "$REMOTE_URL" && "$INIT_GIT" != true ]]; then
  echo "--remote requires --init-git" >&2
  exit 1
fi

if [[ "$PUSH" == true && -z "$REMOTE_URL" ]]; then
  echo "--push requires --remote <url>" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SOURCE_DIR="$REPO_ROOT/templates/openhydroqual-rt-web"

if [[ ! -d "$SOURCE_DIR" ]]; then
  echo "Template directory not found: $SOURCE_DIR" >&2
  exit 1
fi

mkdir -p "$DEST_DIR"

rsync -a --delete \
  --exclude '.git/' \
  --exclude '.venv/' \
  --exclude '__pycache__/' \
  --exclude '.pytest_cache/' \
  "$SOURCE_DIR/" "$DEST_DIR/"

echo "Synced template to: $DEST_DIR"

if [[ "$INIT_GIT" == true ]]; then
  pushd "$DEST_DIR" >/dev/null

  if [[ -d .git ]]; then
    echo "Existing git repository detected; reusing it."
  else
    git init -b "$BRANCH"
  fi

  current_branch="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
  if [[ "$current_branch" != "$BRANCH" ]]; then
    git checkout -B "$BRANCH"
  fi

  git add .
  if git diff --cached --quiet; then
    echo "No changes to commit."
  else
    git commit -m "Initial import of openhydroqual-rt-web scaffold"
  fi

  if [[ -n "$REMOTE_URL" ]]; then
    if git remote get-url origin >/dev/null 2>&1; then
      git remote set-url origin "$REMOTE_URL"
    else
      git remote add origin "$REMOTE_URL"
    fi
    echo "Configured origin: $REMOTE_URL"
  fi

  if [[ "$PUSH" == true ]]; then
    git push -u origin "$BRANCH"
  fi

  popd >/dev/null
else
  echo
  echo "Next steps (if creating a brand-new standalone repo):"
  echo "  cd $DEST_DIR"
  echo "  git init -b $BRANCH"
  echo "  git add ."
  echo "  git commit -m 'Initial import of openhydroqual-rt-web scaffold'"
fi
