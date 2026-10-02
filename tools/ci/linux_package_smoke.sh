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
  case "$FORMAT" in
    deb) DEBIAN_FRONTEND=noninteractive apt-get install -y --reinstall "$PACKAGE";;
    rpm) dnf install -y "$PACKAGE"; [[ "$pass" != reinstall ]] || dnf reinstall -y "$PACKAGE";;
    arch) pacman -U --noconfirm "$PACKAGE";;
    *) exit 2;;
  esac
  [[ -f /usr/share/applications/xyz.younglion.leominigames.desktop ]]
  [[ -f /usr/share/licenses/leominigames/LICENSE ]]
  QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=software \
    xvfb-run -a timeout 40 LeoMiniGames --smoke-test > "$OUT/$pass.log" 2>&1
  grep -F 'PASS: six builtin QML sessions opened and closed' "$OUT/$pass.log"
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
