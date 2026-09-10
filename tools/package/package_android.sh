#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; BUILD_DIR="${1:?usage: package_android.sh <build-dir>}"; DIST="$ROOT/dist/android"
mkdir -p "$DIST"
cmake --build "$BUILD_DIR" --target apk --parallel
APK="$(find "$BUILD_DIR" -type f -name '*.apk' | head -n1)"; [ -n "$APK" ] || { echo 'APK target completed but no APK was found' >&2; exit 2; }
cp "$APK" "$DIST/LeoMiniGames-v0.7.0-Android.apk"
if cmake --build "$BUILD_DIR" --target aab --parallel; then
  AAB="$(find "$BUILD_DIR" -type f -name '*.aab' | head -n1)"; [ -n "$AAB" ] && cp "$AAB" "$DIST/LeoMiniGames-v0.7.0-Android.aab" || { echo 'AAB target returned success but no AAB was found' >&2; exit 3; }
else
  echo 'AAB target is not available in this Qt/Android configuration.' >&2; exit 4
fi
