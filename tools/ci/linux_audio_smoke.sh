#!/usr/bin/env bash
# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
set -euo pipefail
OUT="${1:?usage: linux_audio_smoke.sh OUT COMMAND [ARGS...]}"
shift
mkdir -p "$OUT"
OUT="$(realpath "$OUT")"
flatpak_mode=false
if [[ "${1:-}" == --flatpak ]]; then flatpak_mode=true; shift; fi
[[ $# -gt 0 ]] || { echo 'Missing application command or Flatpak ID.' >&2; exit 2; }
WORK="$(mktemp -d)"
if [[ "$(id -u)" == 0 ]]; then
  # GitHub containers can map HOME to a directory owned by the runner UID.
  # libpulse rejects that mismatched home even with an explicit test socket.
  export HOME="$WORK/home"
  mkdir -m 700 "$HOME"
fi
server= recorder=
cleanup() {
  [[ -z "$recorder" ]] || { kill "$recorder" 2>/dev/null || true; wait "$recorder" 2>/dev/null || true; }
  [[ -z "$server" ]] || { kill "$server" 2>/dev/null || true; wait "$server" 2>/dev/null || true; }
  rm -rf "$WORK"
}
trap cleanup EXIT
export XDG_RUNTIME_DIR="$WORK/runtime"
mkdir -m 700 "$XDG_RUNTIME_DIR"
export PULSE_SERVER="unix:$WORK/pulse.sock"
# This private server cannot change the runner's or a user's real audio setup.
pulseaudio --daemonize=no --use-pid-file=no --exit-idle-time=-1 -n \
  --load="module-native-protocol-unix socket=$WORK/pulse.sock auth-anonymous=1" \
  --load="module-null-sink sink_name=lmg rate=48000 channels=2" > "$OUT/server.log" 2>&1 &
server=$!
ready=false
for attempt in $(seq 1 60); do
  if pactl info > "$OUT/device.log" 2>&1; then ready=true; break; fi
  kill -0 "$server" 2>/dev/null || break
  sleep 0.1
done
if [[ "$ready" != true ]]; then cat "$OUT/server.log" >&2; exit 1; fi
pactl set-default-sink lmg
pactl set-sink-volume lmg 100%
pactl set-sink-mute lmg 0
parec --device=lmg.monitor --format=s16le --rate=48000 --channels=2 > "$OUT/output.pcm" 2> "$OUT/record.log" &
recorder=$!
sleep 0.2
if [[ "$flatpak_mode" == true ]]; then
  # Only the private test socket is exposed; production permissions are unchanged.
  command=(flatpak run --filesystem="$WORK:ro" --env="PULSE_SERVER=$PULSE_SERVER"
    --env=QT_QPA_PLATFORM=xcb --env=QT_QUICK_BACKEND=software "$@")
else
  command=("$@")
fi
if ! QT_QPA_PLATFORM=xcb QT_QUICK_BACKEND=software xvfb-run -a timeout 15 "${command[@]}" --audio-smoke-test > "$OUT/playback.log" 2>&1; then
  cat "$OUT/playback.log" >&2
  exit 1
fi
kill "$recorder"
wait "$recorder" || true
recorder=
python3 "$(dirname "$0")/validate_audio_capture.py" "$OUT/output.pcm"
