#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VERSION="${LMG_VERSION:-0.7.3}"
ARCH="${LMG_FLATPAK_ARCH:-$(flatpak --default-arch)}"
[[ "$ARCH" == x86_64 || "$ARCH" == aarch64 ]] || { echo "Unsupported Flatpak architecture: $ARCH" >&2; exit 2; }
DIST="$ROOT/dist/flatpak"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$DIST" "$WORK/source"
# Snapshot tracked and untracked source while excluding build/dist/git caches.
tar -C "$ROOT" --exclude='./.git' --exclude='./dist' --exclude='./build*' \
  --exclude='./.flatpak-builder' --exclude='__pycache__' -cf - . | tar -C "$WORK/source" -xf -
flatpak-builder --force-clean --user --arch="$ARCH" --install-deps-from=flathub \
  --repo="$WORK/repo" "$WORK/build" "$WORK/source/packaging/flatpak/xyz.younglion.leominigames.json"
flatpak build-bundle --arch="$ARCH" --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo \
  "$WORK/repo" "$DIST/LeoMiniGames-v${VERSION}-Linux-${ARCH}.flatpak" xyz.younglion.leominigames
echo "Flatpak: $DIST/LeoMiniGames-v${VERSION}-Linux-${ARCH}.flatpak"
