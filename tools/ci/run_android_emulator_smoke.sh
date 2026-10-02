#!/usr/bin/env bash
set -euo pipefail
APK="${1:?usage: run_android_emulator_smoke.sh APK [ABI] [output-directory]}"
ABI="${2:-x86_64}"
OUT="${3:-android-smoke}"
case "$ABI" in
  x86) IMAGE='system-images;android-28;google_apis;x86';;
  x86_64|universal) IMAGE='system-images;android-35;google_apis;x86_64';;
  *) echo 'ARM launch evidence requires a matching device.' >&2; exit 1;;
esac
mkdir -p "$OUT"
sdkmanager "emulator" "$IMAGE"
avdmanager create avd --force -n lmg-smoke -k "$IMAGE" <<< no
if [[ -e /dev/kvm ]]; then sudo chmod 666 /dev/kvm; fi
"${ANDROID_SDK_ROOT:?}/emulator/emulator" -avd lmg-smoke -no-window -no-audio -no-boot-anim -gpu swiftshader_indirect > "$OUT/emulator.log" 2>&1 &
trap 'adb emu kill >/dev/null 2>&1 || true' EXIT
timeout 300 bash -c 'until [[ "$(adb shell getprop sys.boot_completed 2>/dev/null | tr -d "\r")" == "1" ]]; do sleep 2; done'
bash "$(dirname "$0")/android_smoke_test.sh" "$APK" "$OUT"
