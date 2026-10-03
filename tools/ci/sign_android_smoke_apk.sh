#!/usr/bin/env bash
set -euo pipefail
INPUT="${1:?usage: sign_android_smoke_apk.sh INPUT.apk OUTPUT.apk}"
OUTPUT="${2:?Specify an output path outside the release package directory}"
TOOLS="${ANDROID_SDK_ROOT:?}/build-tools/${ANDROID_BUILD_TOOLS:-36.0.0}"
[[ -f "$INPUT" && "$INPUT" != "$OUTPUT" ]] || exit 2
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
# CI launch tests use an ephemeral identity; release signing remains separate.
keytool -genkeypair -keystore "$WORK/smoke.jks" -storepass android -keypass android \
  -alias androiddebugkey -dname 'CN=Android CI Smoke' -keyalg RSA -keysize 2048 -validity 2 >/dev/null 2>&1
"$TOOLS/zipalign" -f -P 16 4 "$INPUT" "$WORK/aligned.apk"
"$TOOLS/apksigner" sign --ks "$WORK/smoke.jks" --ks-key-alias androiddebugkey \
  --ks-pass pass:android --key-pass pass:android --out "$OUTPUT" "$WORK/aligned.apk"
"$TOOLS/apksigner" verify "$OUTPUT"
"$TOOLS/zipalign" -c -P 16 4 "$OUTPUT"
