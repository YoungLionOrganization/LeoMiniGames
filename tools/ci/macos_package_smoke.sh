#!/usr/bin/env bash
set -euo pipefail
SUFFIX="${1:?usage: macos_package_smoke.sh arm64|x86_64|universal [output-directory]}"
case "$SUFFIX" in arm64|x86_64|universal) ;; *) echo 'Invalid macOS package architecture.' >&2; exit 2;; esac
OUT="${2:-macos-package-smoke}"
[[ "${GITHUB_ACTIONS:-}" == true && "$(uname -s)" == Darwin ]] || {
  echo 'Run packaged macOS checks only on a disposable GitHub macOS runner.' >&2; exit 2;
}
VERSION="${LMG_VERSION:-0.7.3}"
DIST="$PWD/dist/macos"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
WORK="$(mktemp -d)"
MOUNT="$WORK/mount"
MOUNTED=0
cleanup() {
  if ((MOUNTED)); then hdiutil detach "$MOUNT" -force >/dev/null 2>&1 || true; fi
  rm -rf "$WORK"
}
trap cleanup EXIT
launch() {
  local app="$1" name="$2"
  [[ -f "$app/Contents/Resources/Documentation/INSTALLATION.md" ]] || {
    echo 'Packaged installation guide missing.' >&2; return 1;
  }
  env -u DYLD_LIBRARY_PATH -u DYLD_FRAMEWORK_PATH -u LD_LIBRARY_PATH \
    -u QT_PLUGIN_PATH -u QT_QPA_PLATFORM_PLUGIN_PATH -u QML2_IMPORT_PATH -u QML_IMPORT_PATH \
    QT_QPA_PLATFORM=cocoa QT_QUICK_BACKEND=software QT_FORCE_STDERR_LOGGING=1 \
    python3 - "$app/Contents/MacOS/LeoMiniGames" "$OUT/$name.log" <<'PY'
import subprocess, sys
from pathlib import Path
try:
    run = subprocess.run([sys.argv[1], '--smoke-test'], stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, timeout=45, text=True)
except subprocess.TimeoutExpired as error:
    output = error.stdout or b''
    Path(sys.argv[2]).write_bytes(output if isinstance(output, bytes) else output.encode())
    raise SystemExit('Packaged app timed out')
Path(sys.argv[2]).write_text(run.stdout, encoding='utf-8')
if run.returncode or 'PASS: six builtin QML sessions opened and closed' not in run.stdout:
    print(run.stdout)
    raise SystemExit('Packaged app did not complete builtin session checks')
PY
}

ZIP="$DIST/LeoMiniGames-v${VERSION}-macOS-${SUFFIX}.zip"
DMG="$DIST/LeoMiniGames-v${VERSION}-macOS-${SUFFIX}.dmg"
mkdir -p "$WORK/zip" "$MOUNT"
ditto -x -k "$ZIP" "$WORK/zip"
launch "$WORK/zip/LeoMiniGames.app" zip-launch
hdiutil attach -readonly -nobrowse -mountpoint "$MOUNT" "$DMG" > "$OUT/mount.log" 2>&1
MOUNTED=1
launch "$MOUNT/LeoMiniGames.app" dmg-launch
[[ -L "$MOUNT/Applications" ]] || { echo 'DMG Applications shortcut missing.' >&2; exit 1; }

# Assess and retain the real result. Unsigned distribution may be rejected;
# this test records that fact and never disables Gatekeeper or removes quarantine.
if spctl --assess --type execute --verbose=4 "$MOUNT/LeoMiniGames.app" > "$OUT/gatekeeper.log" 2>&1; then
  GATEKEEPER_STATUS=0
else
  GATEKEEPER_STATUS=$?
fi
[[ "$GATEKEEPER_STATUS" == 0 || "$GATEKEEPER_STATUS" == 3 ]] || {
  cat "$OUT/gatekeeper.log" >&2; exit 1;
}
python3 - "$SUFFIX" "$OUT" "$ZIP" "$DMG" "$GATEKEEPER_STATUS" <<'PY'
import datetime, hashlib, json, os, platform, sys
from pathlib import Path
def digest(path):
    checksum = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            checksum.update(block)
    return checksum.hexdigest()
artifacts = {Path(name).name: digest(Path(name)) for name in sys.argv[3:5]}
report = {
    'source_sha': os.environ['GITHUB_SHA'], 'version': os.environ.get('LMG_VERSION', '0.7.3'),
    'target': 'macOS-' + sys.argv[1], 'environment': 'GitHub macOS ' + platform.mac_ver()[0] + ' / ' + platform.machine(),
    'evidence': 'https://github.com/' + os.environ['GITHUB_REPOSITORY'] + '/actions/runs/' + os.environ['GITHUB_RUN_ID'],
    'tested_at': datetime.datetime.now(datetime.timezone.utc).isoformat(),
    'tester': 'GitHub Actions macos_package_smoke.sh',
    'zip_launch': True, 'dmg_launch': True, 'gatekeeper_assessed': True,
    'gatekeeper_exit_code': int(sys.argv[5]), 'tested_artifacts': artifacts,
}
Path(sys.argv[2], 'package-checks.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
PY
echo "PASS: deployed ZIP and mounted DMG launch; Gatekeeper assessment status=$GATEKEEPER_STATUS"
