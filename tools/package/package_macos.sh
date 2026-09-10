#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; BUILD_DIR="${1:?usage: package_macos.sh <build-dir>}"; DIST="$ROOT/dist/macos"
rm -rf "$DIST"; mkdir -p "$DIST"
APP="$(find "$BUILD_DIR" -maxdepth 4 -type d -name 'LeoMiniGames.app' | head -n1)"
[ -n "$APP" ] || { echo 'LeoMiniGames.app not found' >&2; exit 2; }
cp -R "$APP" "$DIST/LeoMiniGames.app"
MACDEPLOYQT="${MACDEPLOYQT:-$(command -v macdeployqt || true)}"; [ -n "$MACDEPLOYQT" ] || { echo 'macdeployqt not found' >&2; exit 3; }
"$MACDEPLOYQT" "$DIST/LeoMiniGames.app" -qmldir="$ROOT/qml"
ditto -c -k --sequesterRsrc --keepParent "$DIST/LeoMiniGames.app" "$DIST/LeoMiniGames-v0.7.0-macOS.zip"
hdiutil create -volname LeoMiniGames -srcfolder "$DIST/LeoMiniGames.app" -ov -format UDZO "$DIST/LeoMiniGames-v0.7.0-macOS.dmg"
echo 'Artifacts are unsigned unless signing/notarization is performed separately with user-provided credentials.'
