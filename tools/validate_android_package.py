#!/usr/bin/env python3
"""Check actual ELF headers and TLS dependencies, not merely filenames."""
import argparse
from pathlib import Path
import struct
import zipfile

MACHINES = {'armeabi-v7a': 40, 'arm64-v8a': 183, 'x86': 3, 'x86_64': 62}

def validate_elf(data, abi, name):
    if len(data) < 64 or data[:4] != b'\x7fELF' or data[5] != 1:
        raise ValueError(f'{name}: invalid ELF')
    bits = data[4]
    if bits not in (1, 2) or (bits == 2) != (abi in ('arm64-v8a', 'x86_64')):
        raise ValueError(f'{name}: wrong ELF class for {abi}')
    if struct.unpack_from('<H', data, 18)[0] != MACHINES[abi]:
        raise ValueError(f'{name}: wrong ELF machine for {abi}')
    phoff = struct.unpack_from('<Q' if bits == 2 else '<I', data, 32 if bits == 2 else 28)[0]
    entsize, count = struct.unpack_from('<HH', data, 54 if bits == 2 else 42)
    required = 56 if bits == 2 else 32
    if entsize < required or not count or phoff + entsize * count > len(data):
        raise ValueError(f'{name}: invalid program headers')
    loads = 0
    for i in range(count):
        entry = phoff + i * entsize
        if struct.unpack_from('<I', data, entry)[0] != 1: continue
        loads += 1
        if bits == 2:
            offset, address = struct.unpack_from('<QQ', data, entry + 8)
            alignment = struct.unpack_from('<Q', data, entry + 48)[0]
            if alignment < 16384 or offset % 16384 != address % 16384:
                raise ValueError(f'{name}: incompatible with Android 16 KiB pages')
    if not loads: raise ValueError(f'{name}: no loadable segments')

def check_groups(groups, expected, require_plugin):
    if set(groups) != set(expected):
        raise ValueError(f'ABI mismatch: expected {sorted(expected)}, found {sorted(groups)}')
    for abi, libs in groups.items():
        for required in ('libcrypto_3.so', 'libssl_3.so'):
            if required not in libs: raise ValueError(f'{abi}: missing {required}')
        if require_plugin and not any('qopensslbackend' in x for x in libs):
            raise ValueError(f'{abi}: missing Qt OpenSSL TLS plugin')
        for name, data in libs.items(): validate_elf(data, abi, name)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('package', nargs='?', type=Path)
    ap.add_argument('--libs', type=Path)
    ap.add_argument('--abis', help='comma-separated expected ABIs')
    args = ap.parse_args()
    groups = {}
    if args.libs:
        for abi in MACHINES:
            folder = args.libs / abi
            if folder.is_dir(): groups[abi] = {p.name:p.read_bytes() for p in folder.glob('*.so')}
    elif args.package:
        with zipfile.ZipFile(args.package) as z:
            if len(z.namelist()) != len(set(z.namelist())): raise ValueError('duplicate APK/AAB entries')
            for name in z.namelist():
                parts = name.split('/')
                if name.endswith('.so') and len(parts) >= 3 and parts[-3] == 'lib' and parts[-2] in MACHINES:
                    groups.setdefault(parts[-2], {})[parts[-1]] = z.read(name)
    else: ap.error('provide an APK/AAB or --libs directory')
    if not groups: raise ValueError('No Android native libraries found')
    expected = args.abis.split(',') if args.abis else groups.keys()
    check_groups(groups, expected, not args.libs)
    print('PASS: Android TLS libraries, ABI and 16 KiB ELF alignment:', ', '.join(sorted(groups)))

if __name__ == '__main__': main()
