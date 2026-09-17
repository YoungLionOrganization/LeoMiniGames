#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_ios.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
SUFFIX="${LMG_PLATFORM_SUFFIX:-arm64}"
PLATFORM="${LMG_APPLE_PLATFORM:-iOS}"
SDK_KIND="${LMG_APPLE_SDK:-simulator}"
DIST="$ROOT/dist/ios"
STAGE="$DIST/stage-${PLATFORM}-${SDK_KIND}-${SUFFIX}"

rm -rf "$DIST"
mkdir -p "$STAGE/Legal/licenses"

APP="$(find "$BUILD_DIR" -type d -name 'LeoMiniGames.app' | head -n1)"
[[ -n "$APP" ]] || { echo 'Unsigned LeoMiniGames.app not found' >&2; exit 2; }

cp -R "$APP" "$STAGE/LeoMiniGames.app"
cp "$ROOT/LICENSE" "$ROOT/LICENSE_APPLICATION.md" "$ROOT/NOTICE" \
   "$ROOT/COPYRIGHT" "$ROOT/LICENSING.md" "$STAGE/Legal/"
cp "$ROOT/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt" \
   "$ROOT/licenses/YOUNGLION_MOD_LICENSE_1.0.txt" "$STAGE/Legal/licenses/"

OUT="$DIST/LeoMiniGames-v${VERSION}-${PLATFORM}-${SDK_KIND}-${SUFFIX}-unsigned.zip"
ditto -c -k --sequesterRsrc "$STAGE" "$OUT"
rm -rf "$STAGE"

echo "Unsigned Apple bundle: $OUT"
if [[ "$SDK_KIND" == "device" ]]; then
  echo "Device .app is intentionally unsigned. IPA/App Store distribution requires Apple signing/provisioning."
fi
