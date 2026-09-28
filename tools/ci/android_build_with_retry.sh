#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:?usage: android_build_with_retry.sh <build-dir> <target>}"
TARGET="${2:?usage: android_build_with_retry.sh <build-dir> <target>}"
MAX_ATTEMPTS="${LMG_ANDROID_BUILD_RETRIES:-3}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
[[ -d "$BUILD_DIR" ]] || { echo "Android build directory not found: $BUILD_DIR" >&2; exit 2; }
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
# Both CI and release packaging call this helper. Keep Qt's already aligned
# native libraries intact when Android Gradle Plugin assembles APKs and AABs.
# The isolated Gradle home keeps this override out of other projects.
export GRADLE_USER_HOME="$BUILD_DIR/.lmg-gradle-home"
mkdir -p "$GRADLE_USER_HOME/init.d"
cp "$ROOT/tools/ci/preserve_android_native_alignment.gradle" \
   "$GRADLE_USER_HOME/init.d/preserve_android_native_alignment.gradle"

if ! [[ "$MAX_ATTEMPTS" =~ ^[1-9][0-9]*$ ]]; then
  echo "Invalid LMG_ANDROID_BUILD_RETRIES: $MAX_ATTEMPTS" >&2
  exit 64
fi

is_transient_gradle_failure() {
  local log="$1"
  grep -Eqi \
    'gradle-distributions|services\.gradle\.org/distributions|github\.com/gradle/gradle-distributions|Server returned HTTP response code: 5[0-9]{2}|HTTP/[0-9.]+ 5[0-9]{2}|Read timed out|Connection reset|Connection refused|Temporary failure|Could not GET|Could not HEAD|Remote host terminated the handshake' \
    "$log"
}

cleanup_partial_gradle_downloads() {
  local gradle_home="${GRADLE_USER_HOME:-$HOME/.gradle}"
  local dists="$gradle_home/wrapper/dists"
  [[ -d "$dists" ]] || return 0
  find "$dists" -type f \( -name '*.part' -o -name '*.lck' \) -print -delete 2>/dev/null || true
}

attempt=1
while (( attempt <= MAX_ATTEMPTS )); do
  log="${RUNNER_TEMP:-${TMPDIR:-/tmp}}/lmg-android-${TARGET}-attempt-${attempt}.log"
  echo "[Android build] target=$TARGET attempt=$attempt/$MAX_ATTEMPTS"

  set +e
  cmake --build "$BUILD_DIR" --target "$TARGET" --parallel 2>&1 | tee "$log"
  rc=${PIPESTATUS[0]}
  set -e

  if (( rc == 0 )); then
    exit 0
  fi

  if (( attempt >= MAX_ATTEMPTS )) || ! is_transient_gradle_failure "$log"; then
    echo "Android build failed with a non-retryable error (or retry budget exhausted)." >&2
    exit "$rc"
  fi

  echo "Transient Gradle/network failure detected; cleaning partial wrapper downloads before retry." >&2
  cleanup_partial_gradle_downloads
  sleep $((attempt * 10))
  ((attempt += 1))
done

exit 1
