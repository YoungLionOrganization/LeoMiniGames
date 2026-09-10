#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_android.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
DIST="$ROOT/dist/android"
mkdir -p "$DIST"

# The workflow may already have built these targets. Rebuilding is safe and makes
# the script usable on its own as well.
cmake --build "$BUILD_DIR" --target apk --parallel
cmake --build "$BUILD_DIR" --target aab --parallel

APK="$(find "$BUILD_DIR" -type f -name '*.apk' ! -name '*-unsigned.apk' | head -n1)"
if [ -z "$APK" ]; then APK="$(find "$BUILD_DIR" -type f -name '*.apk' | head -n1)"; fi
AAB="$(find "$BUILD_DIR" -type f -name '*.aab' | head -n1)"
[ -n "$APK" ] || { echo 'APK target completed but no APK was found' >&2; exit 2; }
[ -n "$AAB" ] || { echo 'AAB target completed but no AAB was found' >&2; exit 3; }
cp "$APK" "$DIST/LeoMiniGames-v${VERSION}-Android-arm64-v8a.apk"
cp "$AAB" "$DIST/LeoMiniGames-v${VERSION}-Android-arm64-v8a.aab"
