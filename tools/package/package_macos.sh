#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_macos.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
SUFFIX="${LMG_PLATFORM_SUFFIX:-$(uname -m)}"
DIST="$ROOT/dist/macos"
rm -rf "$DIST"; mkdir -p "$DIST"
APP="$(find "$BUILD_DIR" -maxdepth 5 -type d -name 'LeoMiniGames.app' | head -n1)"
[ -n "$APP" ] || { echo 'LeoMiniGames.app not found' >&2; exit 2; }
cp -R "$APP" "$DIST/LeoMiniGames.app"
MACDEPLOYQT="${MACDEPLOYQT:-$(command -v macdeployqt || true)}"
[ -n "$MACDEPLOYQT" ] || { echo 'macdeployqt not found' >&2; exit 3; }
"$MACDEPLOYQT" "$DIST/LeoMiniGames.app" -qmldir="$ROOT/qml"
LEGAL="$DIST/LeoMiniGames.app/Contents/Resources/Legal"
mkdir -p "$LEGAL/licenses"
cp "$ROOT/LICENSE" "$ROOT/LICENSE_APPLICATION.md" "$ROOT/NOTICE" \
   "$ROOT/COPYRIGHT" "$ROOT/LICENSING.md" "$LEGAL/"
cp "$ROOT/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt" \
   "$ROOT/licenses/YOUNGLION_MOD_LICENSE_1.0.txt" "$LEGAL/licenses/"
ditto -c -k --sequesterRsrc --keepParent "$DIST/LeoMiniGames.app" "$DIST/LeoMiniGames-v${VERSION}-macOS-${SUFFIX}.zip"
hdiutil create -volname LeoMiniGames -srcfolder "$DIST/LeoMiniGames.app" -ov -format UDZO "$DIST/LeoMiniGames-v${VERSION}-macOS-${SUFFIX}.dmg"
echo 'Artifacts are unsigned unless signing/notarization is performed separately with user-provided credentials.'
