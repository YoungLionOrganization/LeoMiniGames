#!/usr/bin/env bash
set -euo pipefail
[ "$#" -eq 2 ] || { echo 'usage: compare_reproducible.sh <artifact-a> <artifact-b>' >&2; exit 2; }
A="$1"; B="$2"; sha256sum "$A" "$B"
if cmp -s "$A" "$B"; then echo 'PASS: artifacts are byte-identical'; exit 0; fi
if command -v diffoscope >/dev/null 2>&1; then diffoscope "$A" "$B" || true; fi
echo 'FAIL: artifacts differ' >&2; exit 1
