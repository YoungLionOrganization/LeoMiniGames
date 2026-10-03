#!/usr/bin/env bash
set -euo pipefail
PREFIX="${HOME:?}/.local/opt/LeoMiniGames"
INTEGRATE=1
ACTION=install
while (($#)); do
  case "$1" in
    --prefix) [[ $# -ge 2 ]] || exit 2; PREFIX="$2"; shift 2;;
    --no-integration) INTEGRATE=0; shift;;
    --uninstall) ACTION=uninstall; shift;;
    --help) echo 'Usage: bash LeoMiniGames-Setup.run [--prefix DIRECTORY] [--no-integration] [--uninstall]'; exit 0;;
    *) echo "Unknown option: $1" >&2; exit 2;;
  esac
done
[[ "$PREFIX" == /* && "$PREFIX" != *$'\n'* && "$PREFIX" != *$'\r'* ]] || { echo 'Prefix must be an absolute single-line path.' >&2; exit 2; }
[[ ! -L "$PREFIX" ]] || { echo 'A symlink cannot be used as the installation directory.' >&2; exit 2; }
PREFIX="$(realpath -m -- "$PREFIX")"
case "$PREFIX" in /|/usr|/usr/local|/opt|/home|"$HOME"|"$HOME/.local"|"$HOME/.local/opt") echo 'Choose a dedicated application directory.' >&2; exit 2;; esac
MARKER="$PREFIX/.leominigames-install"
[[ ! -e "$PREFIX" || -f "$MARKER" ]] || { echo 'Refusing to replace a directory that is not a LeoMiniGames installation.' >&2; exit 2; }
if [[ "$ACTION" == uninstall ]]; then
  [[ -f "$MARKER" ]] || { echo 'No managed installation at this prefix.' >&2; exit 2; }
  # Only remove integration files recorded by this installation and still owned by it.
  for file in "$PREFIX"/.integration-*; do
    [[ -f "$file" ]] || continue
    target="$(cat "$file")"
    if [[ ! -L "$target" && -f "$target" ]] && grep -Fxq "# LeoMiniGames prefix: $PREFIX" "$target"; then rm -- "$target"; fi
  done
  rm -rf -- "$PREFIX"
  echo 'LeoMiniGames uninstalled. Games, saves and settings in the user data directory are preserved.'
  exit 0
fi
# Check menu/launcher ownership before replacing the current payload.
BIN="${XDG_BIN_HOME:-$HOME/.local/bin}"
APPS="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
LAUNCHER="$BIN/leominigames"
DESKTOP="$APPS/xyz.younglion.leominigames.desktop"
if ((INTEGRATE)); then
  for file in "$LAUNCHER" "$DESKTOP"; do
    [[ ! -e "$file" && ! -L "$file" ]] || {
      [[ ! -L "$file" && -f "$file" ]] && grep -Fxq "# LeoMiniGames prefix: $PREFIX" "$file"
    } || { echo "Integration file already belongs to another installation: $file" >&2; exit 2; }
  done
  mkdir -p -- "$BIN" "$APPS"
  [[ -w "$BIN" && -w "$APPS" ]] || { echo 'Integration directories are not writable.' >&2; exit 2; }
fi
SELF="$(realpath -- "$0")"
PAYLOAD_LINE="$(awk '/^__LMG_PAYLOAD_BELOW__$/ { print NR + 1; exit }' "$SELF")"
[[ -n "$PAYLOAD_LINE" ]] || { echo 'This script must be run from a packaged Setup.run file.' >&2; exit 2; }
mkdir -p -- "$(dirname "$PREFIX")"
WORK="$(mktemp -d "$(dirname "$PREFIX")/.lmg-install.XXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
tail -n +"$PAYLOAD_LINE" "$SELF" | tar -xz -C "$WORK"
[[ -x "$WORK/LeoMiniGames.AppDir/AppRun" ]] || { echo 'Installer payload is incomplete.' >&2; exit 2; }
touch "$WORK/LeoMiniGames.AppDir/.leominigames-install"
# Keep an uninstall command without the embedded binary payload.
head -n "$((PAYLOAD_LINE - 2))" "$SELF" > "$WORK/LeoMiniGames.AppDir/uninstall.sh"
chmod +x "$WORK/LeoMiniGames.AppDir/uninstall.sh"
if [[ -e "$PREFIX" ]]; then
  cp -a "$PREFIX"/.integration-* "$WORK/LeoMiniGames.AppDir/" 2>/dev/null || true
  mv -- "$PREFIX" "$WORK/previous"
fi
if ! mv -- "$WORK/LeoMiniGames.AppDir" "$PREFIX"; then
  [[ ! -e "$WORK/previous" ]] || mv -- "$WORK/previous" "$PREFIX"
  exit 1
fi
if ((INTEGRATE)); then
  # Bash %q protects paths containing spaces, quotes, dollars and backticks.
  printf '#!/usr/bin/env bash\n# LeoMiniGames prefix: %s\nexec %q "$@"\n' "$PREFIX" "$PREFIX/AppRun" > "$WORK/launcher"
  install -m755 "$WORK/launcher" "$LAUNCHER"
  # Desktop Entry quoting has different rules from shell quoting.
  ESCAPED="${LAUNCHER//\\/\\\\}"
  ESCAPED="${ESCAPED//\"/\\\"}"
  ESCAPED="${ESCAPED//\$/\\\$}"
  ESCAPED="${ESCAPED//\`/\\\`}"
  ESCAPED="${ESCAPED//%/%%}"
  printf '[Desktop Entry]\n# LeoMiniGames prefix: %s\nType=Application\nName=LeoMiniGames\nExec="%s"\nIcon=%s\nCategories=Game;\nTerminal=false\n' \
    "$PREFIX" "$ESCAPED" "$PREFIX/usr/share/icons/hicolor/512x512/apps/leominigames.png" > "$DESKTOP"
  printf '%s\n' "$LAUNCHER" > "$PREFIX/.integration-launcher"
  printf '%s\n' "$DESKTOP" > "$PREFIX/.integration-desktop"
fi
echo "LeoMiniGames installed at $PREFIX"
echo "Launch: $PREFIX/AppRun"
echo "Uninstall: bash '$PREFIX/uninstall.sh' --prefix '$PREFIX' --uninstall"
exit 0
__LMG_PAYLOAD_BELOW__
