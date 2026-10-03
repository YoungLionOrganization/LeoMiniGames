#!/usr/bin/env bash
set -euo pipefail
APK="${1:?usage: android_smoke_test.sh APK [output-directory]}"
OUT="${2:-android-smoke}"
APP_ID=xyz.younglion.leominigames
mkdir -p "$OUT"
adb wait-for-device
adb shell getprop ro.product.cpu.abilist > "$OUT/device-abis.txt"
adb shell getprop ro.build.version.sdk > "$OUT/device-api.txt"
adb install -r "$APK"
adb logcat -c
adb shell am force-stop "$APP_ID"
ACTIVITY="$(adb shell cmd package resolve-activity --brief "$APP_ID" | tr -d '\r' | tail -n 1)"
[[ "$ACTIVITY" == "$APP_ID/"* ]] || { echo 'Launch activity could not be resolved.' >&2; exit 1; }
adb shell am start -W -n "$ACTIVITY" > "$OUT/launch.txt"
# This delay is a bounded application launch observation window.
sleep 10
adb logcat -d > "$OUT/logcat.txt"
adb shell pidof "$APP_ID" > "$OUT/pid.txt" || true
python3 "$(dirname "$0")/validate_android_logcat.py" "$OUT/logcat.txt" "$OUT/pid.txt"

# Use the actual installed Qt/OpenSSL backend, not host curl or an ELF-only test.
adb shell am force-stop "$APP_ID"
adb logcat -c
adb shell am start -W -n "$ACTIVITY" --ez xyz.younglion.leominigames.tlsSmokeTest true > "$OUT/tls-launch.txt"
for attempt in $(seq 1 40); do
  adb logcat -d > "$OUT/tls-logcat.txt"
  if grep -Fq 'PASS: HTTPS catalog probe; certificate validated;' "$OUT/tls-logcat.txt"; then
    echo 'PASS: installed APK completed a certificate-validated HTTPS catalog request'
    exit 0
  fi
  if grep -Eq 'FAIL: TLS backend unavailable|FAIL: HTTPS catalog probe' "$OUT/tls-logcat.txt"; then
    cat "$OUT/tls-logcat.txt"
    exit 1
  fi
  sleep 1
done
echo 'Android TLS probe did not report success; inspect tls-logcat.txt' >&2
exit 1
