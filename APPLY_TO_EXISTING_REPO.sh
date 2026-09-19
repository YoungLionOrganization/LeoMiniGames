#!/usr/bin/env bash
set -euo pipefail
SRC="$(cd "$(dirname "$0")" && pwd)"
REPO="${1:-.}"
REPO="$(cd "$REPO" && pwd)"
[[ -d "$REPO/.git" ]] || { echo "Target is not a Git repository: $REPO" >&2; exit 2; }
echo "Applying LeoMiniGames source/licensing/workflow repair to $REPO"
( cd "$SRC" && tar --exclude=.git --exclude=build --exclude=dist --exclude=.qt --exclude=.idea --exclude=.vs --exclude=APPLY_TO_EXISTING_REPO.ps1 --exclude=APPLY_TO_EXISTING_REPO.sh -cf - . ) | ( cd "$REPO" && tar -xf - )
cd "$REPO"
python3 tools/validate_distribution.py
python3 tools/validate_v070.py
echo "Repair applied. Review 'git status', commit, then push."
