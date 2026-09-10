#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_linux.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
DIST="$ROOT/dist/linux"
APPDIR="$DIST/LeoMiniGames.AppDir"
rm -rf "$DIST"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/512x512/apps"

EXE="$(find "$BUILD_DIR" -type f -name LeoMiniGames -perm -111 | head -n1)"
[ -n "$EXE" ] || { echo 'LeoMiniGames executable not found' >&2; exit 2; }
LINUXDEPLOY="${LINUXDEPLOY:-$(command -v linuxdeploy || true)}"
APPIMAGETOOL="${APPIMAGETOOL:-$(command -v appimagetool || true)}"
[ -n "$LINUXDEPLOY" ] || { echo 'linuxdeploy is required to create a genuinely portable Linux artifact.' >&2; exit 3; }
[ -n "$APPIMAGETOOL" ] || { echo 'appimagetool is required for the required AppImage artifact.' >&2; exit 4; }

cp "$EXE" "$APPDIR/usr/bin/LeoMiniGames"
cp -a "$ROOT/LICENSE" "$ROOT/README.md" "$APPDIR/"
cp "$ROOT/resources/branding/leominigames_icon_512.png" "$APPDIR/usr/share/icons/hicolor/512x512/apps/leominigames.png"
cat > "$APPDIR/usr/share/applications/leominigames.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=LeoMiniGames
Comment=Modular Qt mini-game platform
Exec=LeoMiniGames
Icon=leominigames
Categories=Game;
Terminal=false
EOF
"$LINUXDEPLOY" --appdir "$APPDIR" \
  --executable "$APPDIR/usr/bin/LeoMiniGames" \
  --desktop-file "$APPDIR/usr/share/applications/leominigames.desktop" \
  --icon-file "$APPDIR/usr/share/icons/hicolor/512x512/apps/leominigames.png"
[ -x "$APPDIR/AppRun" ] || { echo 'linuxdeploy did not create a runnable AppDir/AppRun.' >&2; exit 5; }

tar -C "$DIST" -czf "$DIST/LeoMiniGames-v${VERSION}-Linux-x86_64.tar.gz" "$(basename "$APPDIR")"
"$APPIMAGETOOL" "$APPDIR" "$DIST/LeoMiniGames-v${VERSION}-Linux-x86_64.AppImage"

echo "Portable: $DIST/LeoMiniGames-v${VERSION}-Linux-x86_64.tar.gz"
echo "AppImage: $DIST/LeoMiniGames-v${VERSION}-Linux-x86_64.AppImage"
