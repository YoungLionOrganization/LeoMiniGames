#!/usr/bin/env bash
set -euo pipefail
APK="${1:?usage: run_android_emulator_smoke.sh APK [ABI] [output-directory]}"
ABI="${2:-x86_64}"
OUT="${3:-android-smoke}"
[[ -f "$APK" ]] || { echo "APK not found: $APK" >&2; exit 1; }
case "$ABI" in
  x86) IMAGE='system-images;android-28;google_apis;x86'; DEVICE_ABI=x86;;
  x86_64|universal) IMAGE='system-images;android-35;google_apis;x86_64'; DEVICE_ABI=x86_64;;
  *) echo 'ARM launch evidence requires a matching device.' >&2; exit 1;;
esac
mkdir -p "$OUT"
sdkmanager "emulator" "$IMAGE"
# Pin a known command-line tools release: newer preview tools have failed while
# reading the optional devices.xml in otherwise valid Google system images.
sdkmanager 'cmdline-tools;16.0'
AVDMANAGER="${LMG_AVDMANAGER:-${ANDROID_SDK_ROOT:?}/cmdline-tools/16.0/bin/avdmanager}"
export ANDROID_AVD_HOME="$(cd "$OUT" && pwd)/avd"
mkdir -p "$ANDROID_AVD_HOME"
if ! "$AVDMANAGER" create avd --force -n lmg-smoke -k "$IMAGE" --abi "$DEVICE_ABI" --device pixel_2 \
  > "$OUT/avd-create.log" 2>&1 <<< no; then
  cat "$OUT/avd-create.log" >&2
  exit 1
fi
[[ -s "$ANDROID_AVD_HOME/lmg-smoke.ini" && -s "$ANDROID_AVD_HOME/lmg-smoke.avd/config.ini" ]] || {
  echo 'AVD creation did not produce a usable configuration.' >&2
  cat "$OUT/avd-create.log" >&2
  exit 1
}
if [[ -e /dev/kvm ]]; then sudo chmod 666 /dev/kvm; fi
# install-qt-action exports Android Qt libraries/plugins. The host emulator ships
# its own Qt: inheriting those paths can abort it before adb sees a device.
env -u LD_LIBRARY_PATH -u QT_PLUGIN_PATH -u QT_QPA_PLATFORM_PLUGIN_PATH \
  -u QML2_IMPORT_PATH -u QML_IMPORT_PATH -u QT_QPA_PLATFORM -u QT_QPA_PLATFORMTHEME \
  "${ANDROID_SDK_ROOT:?}/emulator/emulator" -avd lmg-smoke -port 5554 \
  -no-window -no-audio -no-boot-anim -no-snapshot -gpu swiftshader \
  > "$OUT/emulator.log" 2>&1 &
EMULATOR_PID=$!
export ANDROID_SERIAL=emulator-5554
cleanup() {
  adb -s "$ANDROID_SERIAL" emu kill >/dev/null 2>&1 || true
  kill "$EMULATOR_PID" >/dev/null 2>&1 || true
  wait "$EMULATOR_PID" 2>/dev/null || true
}
trap cleanup EXIT
DEADLINE=$((SECONDS + 600))
while [[ "$(adb -s "$ANDROID_SERIAL" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" != "1" ]]; do
  if ! kill -0 "$EMULATOR_PID" 2>/dev/null; then
    echo 'Android emulator exited before boot.' >&2
    cat "$OUT/emulator.log" >&2
    exit 1
  fi
  if (( SECONDS >= DEADLINE )); then
    echo 'Android emulator boot timed out.' >&2
    cat "$OUT/emulator.log" >&2
    adb -s "$ANDROID_SERIAL" logcat -d > "$OUT/boot-logcat.txt" 2>&1 || true
    exit 1
  fi
  sleep 2
 done
bash "$(dirname "$0")/android_smoke_test.sh" "$APK" "$OUT"
