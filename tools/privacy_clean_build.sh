#!/usr/bin/env bash
# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${PRIVACY_BUILD_DIR:-$ROOT/build-privacy}"
: "${ANDROID_ABI:=arm64-v8a}"

command -v cmake >/dev/null || { echo 'ERROR: cmake is required.' >&2; exit 2; }
command -v ninja >/dev/null || { echo 'ERROR: ninja is required.' >&2; exit 2; }
if command -v qt-cmake >/dev/null; then QT_CMAKE=qt-cmake
elif [[ -n "${QT_HOST_PATH:-}" && -x "${QT_HOST_PATH}/bin/qt-cmake" ]]; then QT_CMAKE="${QT_HOST_PATH}/bin/qt-cmake"
else echo 'ERROR: a local Qt 6 Android qt-cmake is required; this privacy build helper does not download toolchains.' >&2; exit 3
fi
[[ -n "${ANDROID_SDK_ROOT:-}" ]] || { echo 'ERROR: ANDROID_SDK_ROOT is required.' >&2; exit 4; }
[[ -n "${ANDROID_NDK_ROOT:-${ANDROID_NDK:-}}" ]] || { echo 'ERROR: ANDROID_NDK_ROOT/ANDROID_NDK is required.' >&2; exit 4; }

python3 "$ROOT/tools/audit_prebuilt_binaries.py"
python3 "$ROOT/tools/validate_v070.py"
rm -rf "$BUILD"
"$QT_CMAKE" -S "$ROOT" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLEOMINIGAMES_PRIVACY_BUILD=ON \
  -DBUILD_TESTING=OFF \
  -DANDROID_ABI="$ANDROID_ABI"
cmake --build "$BUILD" --parallel
cmake --build "$BUILD" --target apk --parallel
