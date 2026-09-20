#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_macos.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.1}"
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
cp "$ROOT/LICENSE" "$ROOT/NOTICE" \
   "$ROOT/COPYRIGHT" "$ROOT/LICENSING.md" "$ROOT/LICENSE_HISTORY.md" "$ROOT/LICENSE_METADATA.json" "$LEGAL/"
cp "$ROOT/licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt" \
   "$ROOT/licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt" "$LEGAL/licenses/"
cp "$ROOT/docs/THIRD_PARTY_NOTICES.md" "$ROOT/docs/QT_LGPL_COMPLIANCE.md" "$LEGAL/"
ditto -c -k --sequesterRsrc --keepParent "$DIST/LeoMiniGames.app" "$DIST/LeoMiniGames-v${VERSION}-macOS-${SUFFIX}.zip"
DMG="$DIST/LeoMiniGames-v${VERSION}-macOS-${SUFFIX}.dmg"
TMP_DMG_DIR="$(mktemp -d)"
TMP_DMG="$TMP_DMG_DIR/LeoMiniGames.dmg"
VOLUME_NAME="LeoMiniGames-${SUFFIX}"
cleanup_dmg() { rm -rf "$TMP_DMG_DIR"; }
trap cleanup_dmg EXIT
rm -f "$DMG"
for attempt in 1 2 3; do
  hdiutil detach "/Volumes/$VOLUME_NAME" -force >/dev/null 2>&1 || true
  rm -f "$TMP_DMG"
  sync
  if hdiutil create -volname "$VOLUME_NAME" -srcfolder "$DIST/LeoMiniGames.app" -format UDZO "$TMP_DMG"; then
    mv "$TMP_DMG" "$DMG"
    break
  fi
  if [[ "$attempt" -eq 3 ]]; then
    echo "ERROR: hdiutil failed after $attempt attempts." >&2
    exit 4
  fi
  sleep $((attempt * 2))
done
[[ -s "$DMG" ]] || { echo 'ERROR: DMG was not created.' >&2; exit 4; }
echo 'Artifacts are unsigned unless signing/notarization is performed separately with user-provided credentials.'
