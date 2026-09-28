#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:?usage: package_android.sh <build-dir>}"
VERSION="${LMG_VERSION:-0.7.2}"
ABI="${LMG_ANDROID_ABI:-arm64-v8a}"
BUILD_APK="${LMG_BUILD_APK:-1}"
BUILD_AAB="${LMG_BUILD_AAB:-1}"
DIST="$ROOT/dist/android"
ANDROID_BUILD_RETRY="$ROOT/tools/ci/android_build_with_retry.sh"
[[ -f "$ANDROID_BUILD_RETRY" ]] || { echo "Missing Android retry helper: $ANDROID_BUILD_RETRY" >&2; exit 65; }
mkdir -p "$DIST"
# Release artifacts must use the persistent signing identity. Debug CI is separate.
[[ "${LMG_ANDROID_SIGNING:-0}" == "1" ]] || { echo 'Release Android packaging requires the configured signing secrets.' >&2; exit 2; }
EXPECTED_ABIS="$ABI"
[[ "$ABI" != "universal" ]] || EXPECTED_ABIS="arm64-v8a,armeabi-v7a,x86_64,x86"


LEGAL_ZIP="$DIST/LeoMiniGames-v${VERSION}-Android-Legal.zip"
python3 - "$ROOT" "$LEGAL_ZIP" <<'PY'
from pathlib import Path
import sys, zipfile
root = Path(sys.argv[1])
out = Path(sys.argv[2])
files = [
    "LICENSE",
    "NOTICE",
    "COPYRIGHT",
    "LICENSING.md",
    "LICENSE_HISTORY.md",
    "LICENSE_METADATA.json",
    "licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt",
    "licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt",
    "docs/THIRD_PARTY_NOTICES.md",
    "docs/QT_LGPL_COMPLIANCE.md",
]
with zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for rel in files:
        p = root / rel
        if not p.is_file():
            raise SystemExit(f"missing legal file: {rel}")
        z.write(p, rel)
PY

if [[ "$BUILD_APK" == "1" ]]; then
  bash "$ANDROID_BUILD_RETRY" "$BUILD_DIR" apk
  APK="$(find "$BUILD_DIR" -type f -name '*.apk' ! -name '*-unsigned.apk' | head -n1)"
  [[ -n "$APK" ]] || APK="$(find "$BUILD_DIR" -type f -name '*.apk' | head -n1)"
  [[ -n "$APK" ]] || { echo 'APK target completed but no APK was found' >&2; exit 2; }
  python3 "$ROOT/tools/validate_android_package.py" "$APK" --abis "$EXPECTED_ABIS"
  "${ANDROID_SDK_ROOT:?}/build-tools/${ANDROID_BUILD_TOOLS:-36.0.0}/apksigner" verify "$APK"
  "${ANDROID_SDK_ROOT}/build-tools/${ANDROID_BUILD_TOOLS:-36.0.0}/zipalign" -c -P 16 4 "$APK"
  cp "$APK" "$DIST/LeoMiniGames-v${VERSION}-Android-${ABI}.apk"
fi

if [[ "$BUILD_AAB" == "1" ]]; then
  bash "$ANDROID_BUILD_RETRY" "$BUILD_DIR" aab
  AAB="$(find "$BUILD_DIR" -type f -name '*.aab' | head -n1)"
  [[ -n "$AAB" ]] || { echo 'AAB target completed but no AAB was found' >&2; exit 3; }
  python3 "$ROOT/tools/validate_android_package.py" "$AAB" --abis "$EXPECTED_ABIS"
  cp "$AAB" "$DIST/LeoMiniGames-v${VERSION}-Android-${ABI}.aab"
fi
