#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_linux_native.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
SUFFIX="${LMG_PLATFORM_SUFFIX:-$(uname -m)}"
DIST="$ROOT/dist/linux-native"
STAGE="$DIST/LeoMiniGames-$SUFFIX"
rm -rf "$DIST"
mkdir -p "$STAGE/licenses"

EXE="$(find "$BUILD_DIR" -type f -name LeoMiniGames -perm -111 | head -n1)"
[[ -n "$EXE" ]] || { echo "LeoMiniGames executable not found" >&2; exit 2; }

cp "$EXE" "$STAGE/LeoMiniGames"
cp "$ROOT/LICENSE" "$ROOT/LICENSE_APPLICATION.md" "$ROOT/NOTICE" \
   "$ROOT/COPYRIGHT" "$ROOT/LICENSING.md" "$STAGE/"
cp "$ROOT/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt" \
   "$ROOT/licenses/YOUNGLION_MOD_LICENSE_1.0.txt" "$STAGE/licenses/"

cat > "$STAGE/RUNTIME.txt" <<EOF
LeoMiniGames $VERSION — native $SUFFIX build

This archive is intentionally linked against the target distribution's native
Qt/runtime packages. It is a distro-validation/native artifact, not the
self-contained portable build. For a bundled cross-distro artifact use the
Ubuntu-produced AppImage.
EOF

tar -C "$DIST" -czf "$DIST/LeoMiniGames-v${VERSION}-${SUFFIX}-native.tar.gz" \
    "$(basename "$STAGE")"
