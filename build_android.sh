#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
ABI="${1:-arm64-v8a}"
CONFIG="${2:-Release}"
PACKAGE="${3:-apk}"
case "$ABI" in
  arm64-v8a) QT_ARCH=android_arm64_v8a ;;
  armeabi-v7a) QT_ARCH=android_armv7 ;;
  x86_64) QT_ARCH=android_x86_64 ;;
  x86) QT_ARCH=android_x86 ;;
  *) echo "Unsupported ABI: $ABI" >&2; exit 2 ;;
esac
QT_BASE="${QT_BASE:-$HOME/Qt}"
QT_VERSION="${QT_VERSION:-6.10.2}"
QT_KIT="${QT_ANDROID_ROOT:-$QT_BASE/$QT_VERSION/$QT_ARCH}"
QTCMAKE="$QT_KIT/bin/qt-cmake"
[[ -f "$QTCMAKE" ]] || { echo "Qt Android kit missing: $QTCMAKE" >&2; exit 3; }
ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$HOME/Android/Sdk}}"
ANDROID_NDK_ROOT="${ANDROID_NDK_ROOT:-$ANDROID_SDK_ROOT/ndk/27.2.12479018}"
[[ -f "$ANDROID_NDK_ROOT/source.properties" ]] || { echo "NDK r27c missing: $ANDROID_NDK_ROOT" >&2; exit 4; }
[[ -f "$ANDROID_SDK_ROOT/platforms/android-36/android.jar" ]] || { echo "Android platform 36 missing" >&2; exit 5; }
[[ -d "$ANDROID_SDK_ROOT/build-tools/36.0.0" ]] || { echo "Android build-tools 36.0.0 missing" >&2; exit 6; }
command -v java >/dev/null || { echo "JDK not found" >&2; exit 7; }
command -v cmake >/dev/null || { echo "cmake not found" >&2; exit 8; }
command -v ninja >/dev/null || { echo "ninja not found" >&2; exit 9; }
BUILD_DIR="$ROOT/build/android-$ABI-$CONFIG"
[[ "${LMG_KEEP_ANDROID_BUILD:-0}" == 1 ]] || rm -rf "$BUILD_DIR"
bash "$QTCMAKE" -S "$ROOT" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE="$CONFIG" -DBUILD_TESTING=OFF \
  -DANDROID_ABI="$ABI" -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
  -DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT"
case "$PACKAGE" in
  apk) cmake --build "$BUILD_DIR" --target apk --parallel ;;
  aab) cmake --build "$BUILD_DIR" --target aab --parallel ;;
  both) cmake --build "$BUILD_DIR" --target apk --parallel; cmake --build "$BUILD_DIR" --target aab --parallel ;;
  *) echo "Package must be apk, aab, or both" >&2; exit 10 ;;
esac
find "$BUILD_DIR" -type f \( -name '*.apk' -o -name '*.aab' \) -print
