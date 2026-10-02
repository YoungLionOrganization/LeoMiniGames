#!/usr/bin/env sh
set -eu
SDK_PROJECT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
exec python3 "$SDK_PROJECT_DIR/../../tools/sdk/build_package.py" "$SDK_PROJECT_DIR" --rcc "${1:-rcc}"
