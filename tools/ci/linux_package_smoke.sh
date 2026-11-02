#!/usr/bin/env bash
set -euo pipefail
PACKAGE="$(realpath "${1:?usage: linux_package_smoke.sh PACKAGE deb|rpm|arch}")"
FORMAT="${2:?}"
OUT="${3:-linux-package-smoke}"
mkdir -p "$OUT"
OUT="$(realpath "$OUT")"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
trap 'status=$?; echo "Package smoke failed at line $LINENO (status $status)" >&2; for log in "$OUT"/*.log; do [[ ! -f "$log" ]] || cat "$log" >&2; done; exit "$status"' ERR
export XDG_DATA_HOME="$WORK/data" XDG_CONFIG_HOME="$WORK/config" XDG_CACHE_HOME="$WORK/cache"
mkdir -p "$XDG_DATA_HOME/YoungLion/LeoMiniGames"
echo 'user save survives servicing' > "$XDG_DATA_HOME/YoungLion/LeoMiniGames/sentinel"
for pass in install reinstall; do
  # Reinstallation must repair the payload, rather than only report installed.
  if [[ "$pass" == reinstall ]]; then rm -- /usr/bin/LeoMiniGames; fi
  case "$FORMAT" in
    deb) DEBIAN_FRONTEND=noninteractive apt-get install -y --reinstall "$PACKAGE";;
    rpm) dnf install -y "$PACKAGE"; [[ "$pass" != reinstall ]] || dnf reinstall -y "$PACKAGE";;
    arch) pacman -U --noconfirm "$PACKAGE";;
    *) exit 2;;
  esac
  [[ -f /usr/share/applications/xyz.younglion.leominigames.desktop ]]
  [[ -f /usr/share/licenses/leominigames/LICENSE ]]
  if [[ "${LMG_REQUIRE_RUNTIME_ONLY:-0}" == 1 ]]; then
    # Arch bundles CMake metadata with its runtime packages. Check build tools
    # instead of rejecting that legitimate distro layout as a development kit.
    if command -v cmake >/dev/null || command -v c++ >/dev/null || command -v g++ >/dev/null; then
      echo 'Clean-runtime test unexpectedly contains a build toolchain.' >&2
      exit 1
    fi
  fi
  QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=software \
    xvfb-run -a timeout 40 LeoMiniGames --smoke-test > "$OUT/$pass.log" 2>&1
  grep -F 'PASS: six builtin QML sessions opened and closed' "$OUT/$pass.log"
  grep -F 'PASS: PNG JPEG WebP SVG codecs and Controls Shapes Dialogs imports' "$OUT/$pass.log"
  if [[ "${LMG_TEST_AUDIO:-0}" == 1 ]]; then
    bash "$(dirname "$0")/linux_audio_smoke.sh" "$OUT/$pass-audio" LeoMiniGames
  fi
done
case "$FORMAT" in
  deb) apt-get remove -y leominigames;;
  rpm) dnf remove -y leominigames;;
  arch) pacman -R --noconfirm leominigames;;
esac
[[ ! -e /usr/bin/LeoMiniGames && ! -e /usr/share/applications/xyz.younglion.leominigames.desktop ]]
grep -Fx 'user save survives servicing' "$XDG_DATA_HOME/YoungLion/LeoMiniGames/sentinel"
sha256sum "$PACKAGE" > "$OUT/package.sha256"
echo 'PASS: package install, reinstall, six builtin GUI sessions and uninstall; user data preserved'
