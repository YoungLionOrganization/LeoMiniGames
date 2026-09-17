#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_android.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.0}"
ABI="${LMG_ANDROID_ABI:-arm64-v8a}"
BUILD_APK="${LMG_BUILD_APK:-1}"
BUILD_AAB="${LMG_BUILD_AAB:-1}"
DIST="$ROOT/dist/android"
mkdir -p "$DIST"

LEGAL_ZIP="$DIST/LeoMiniGames-v${VERSION}-Android-Legal.zip"
python3 - "$ROOT" "$LEGAL_ZIP" <<'PY'
from pathlib import Path
import sys, zipfile
root = Path(sys.argv[1])
out = Path(sys.argv[2])
files = [
    "LICENSE",
    "LICENSE_APPLICATION.md",
    "NOTICE",
    "COPYRIGHT",
    "LICENSING.md",
    "licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt",
    "licenses/YOUNGLION_MOD_LICENSE_1.0.txt",
]
with zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for rel in files:
        p = root / rel
        if not p.is_file():
            raise SystemExit(f"missing legal file: {rel}")
        z.write(p, rel)
PY

if [[ "$BUILD_APK" == "1" ]]; then
  cmake --build "$BUILD_DIR" --target apk --parallel
  APK="$(find "$BUILD_DIR" -type f -name '*.apk' ! -name '*-unsigned.apk' | head -n1)"
  [[ -n "$APK" ]] || APK="$(find "$BUILD_DIR" -type f -name '*.apk' | head -n1)"
  [[ -n "$APK" ]] || { echo 'APK target completed but no APK was found' >&2; exit 2; }
  cp "$APK" "$DIST/LeoMiniGames-v${VERSION}-Android-${ABI}.apk"
fi

if [[ "$BUILD_AAB" == "1" ]]; then
  cmake --build "$BUILD_DIR" --target aab --parallel
  AAB="$(find "$BUILD_DIR" -type f -name '*.aab' | head -n1)"
  [[ -n "$AAB" ]] || { echo 'AAB target completed but no AAB was found' >&2; exit 3; }
  cp "$AAB" "$DIST/LeoMiniGames-v${VERSION}-Android-${ABI}.aab"
fi
