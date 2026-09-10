#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_ios.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
DIST="$ROOT/dist/ios"
rm -rf "$DIST"; mkdir -p "$DIST"
APP="$(find "$BUILD_DIR" -type d -name 'LeoMiniGames.app' | head -n1)"
[ -n "$APP" ] || { echo 'Unsigned/test LeoMiniGames.app not found' >&2; exit 2; }
ditto -c -k --sequesterRsrc --keepParent "$APP" "$DIST/LeoMiniGames-v${VERSION}-iOS-unsigned-simulator.zip"
echo 'No IPA/App Store artifact is claimed without Apple signing credentials.'
