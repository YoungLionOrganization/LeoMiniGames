#!/usr/bin/env bash
# Build the pinned, supported LTS source; never use the obsolete 3.1 prebuilt binaries.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:?usage: build_android_openssl.sh <output-root> [ABI ...]}"
shift
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
NDK="${ANDROID_NDK_ROOT:-${ANDROID_NDK_HOME:-}}"
[[ -d "$NDK/toolchains/llvm/prebuilt" ]] || { echo 'Set ANDROID_NDK_ROOT to NDK r27c or newer.' >&2; exit 2; }
case "$(uname -s)" in Linux) HOST=linux-x86_64;; Darwin) HOST=darwin-x86_64;; *) echo 'Use Linux, macOS or WSL to build Android OpenSSL.' >&2; exit 2;; esac
export ANDROID_NDK_ROOT="$NDK"
export PATH="$NDK/toolchains/llvm/prebuilt/$HOST/bin:$PATH"
for tool in curl perl make patchelf; do command -v "$tool" >/dev/null || { echo "Missing tool: $tool" >&2; exit 2; }; done
VERSION=3.5.8
SHA256=a8f84a39918ec6415ce765d9b429d313ba97b8143169c172e734b9514464f5b2
ARCHIVE="$OUT/openssl-$VERSION.tar.gz"
if [[ ! -f "$ARCHIVE" ]]; then
  curl --fail --location --proto '=https' --proto-redir '=https' --retry 3 --connect-timeout 20 --max-time 300 \
    "https://github.com/openssl/openssl/releases/download/openssl-$VERSION/openssl-$VERSION.tar.gz" -o "$ARCHIVE.part"
  mv "$ARCHIVE.part" "$ARCHIVE"
fi
python3 - "$ARCHIVE" "$SHA256" <<'PY'
import hashlib,sys
from pathlib import Path
if hashlib.sha256(Path(sys.argv[1]).read_bytes()).hexdigest()!=sys.argv[2]:
    raise SystemExit('OpenSSL source checksum mismatch; remove the archive and retry.')
PY
ABIS=("$@")
if [[ ${#ABIS[@]} -eq 0 ]]; then ABIS=(arm64-v8a armeabi-v7a x86_64 x86); fi
for abi in "${ABIS[@]}"; do
  case "$abi" in arm64-v8a) target=android-arm64;; armeabi-v7a) target=android-arm;; x86_64) target=android-x86_64;; x86) target=android-x86;; *) echo "Unsupported ABI: $abi" >&2; exit 2;; esac
  WORK="$OUT/build-$abi"
  mkdir -p "$WORK" "$OUT/$abi"
  tar -xzf "$ARCHIVE" --strip-components=1 -C "$WORK"
  (
    cd "$WORK"
    perl Configure "$target" shared no-tests no-apps -D__ANDROID_API__=28 \
      -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384
    make -j"${LMG_BUILD_JOBS:-4}" build_libs
    cp -L libcrypto.so "$OUT/$abi/libcrypto_3.so"
    cp -L libssl.so "$OUT/$abi/libssl_3.so"
    # patchelf can rewrite PT_LOAD segments using its default 4 KiB page size.
    # Preserve Android's required 16 KiB alignment on every rewrite.
    patchelf --page-size 16384 --set-soname libcrypto_3.so "$OUT/$abi/libcrypto_3.so"
    patchelf --page-size 16384 --set-soname libssl_3.so "$OUT/$abi/libssl_3.so"
    patchelf --page-size 16384 --replace-needed libcrypto.so.3 libcrypto_3.so "$OUT/$abi/libssl_3.so"
    cp LICENSE.txt "$OUT/LICENSE-OpenSSL.txt"
  )
done
python3 "$ROOT/tools/validate_android_package.py" --libs "$OUT"
echo "Android TLS libraries ready: $OUT"
