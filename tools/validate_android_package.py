#!/usr/bin/env python3
"""Check actual ELF headers and TLS dependencies, not merely filenames."""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

MACHINES = {'armeabi-v7a': 40, 'arm64-v8a': 183, 'x86': 3, 'x86_64': 62}
# Public NDK libraries available at the project's minSdk 28. Android's private
# libcrypto/libssl and libc++_shared are deliberately absent from this list.
SYSTEM_LIBRARIES = frozenset({
    'libc.so', 'libm.so', 'libdl.so', 'liblog.so', 'libandroid.so',
    'libjnigraphics.so', 'libz.so', 'libEGL.so', 'libGLESv1_CM.so',
    'libGLESv2.so', 'libGLESv3.so', 'libOpenSLES.so', 'libvulkan.so',
    'libaaudio.so', 'libmediandk.so', 'libcamera2ndk.so',
    'libnativewindow.so', 'libneuralnetworks.so', 'libstdc++.so',
})

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
        elif name in ('libcrypto_3.so', 'libssl_3.so'):
            offset, address = struct.unpack_from('<II', data, entry + 4)
            alignment = struct.unpack_from('<I', data, entry + 28)[0]
            if alignment < 16384 or offset % 16384 != address % 16384:
                raise ValueError(f'{name}: OpenSSL is not 16 KiB aligned')
    if not loads: raise ValueError(f'{name}: no loadable segments')

def dynamic_dependencies(data, name):
    """Read PT_DYNAMIC without relying on section headers or host readelf."""
    bits = data[4]
    phoff = struct.unpack_from('<Q' if bits == 2 else '<I', data, 32 if bits == 2 else 28)[0]
    entsize, count = struct.unpack_from('<HH', data, 54 if bits == 2 else 42)
    loads, dynamic = [], None
    for i in range(count):
        pos = phoff + i * entsize
        kind = struct.unpack_from('<I', data, pos)[0]
        if bits == 2:
            offset, address, _, size = struct.unpack_from('<QQQQ', data, pos + 8)
        else:
            offset, address, _, size = struct.unpack_from('<IIII', data, pos + 4)
        if offset + size > len(data):
            raise ValueError(f'{name}: segment extends beyond ELF file')
        if kind == 1: loads.append((address, offset, size))
        if kind == 2: dynamic = (offset, size)
    if dynamic is None: raise ValueError(f'{name}: missing dynamic table')
    step, fmt = (16, '<qQ') if bits == 2 else (8, '<iI')
    tags = {}
    terminated = False
    for pos in range(dynamic[0], dynamic[0] + dynamic[1] - step + 1, step):
        tag, value = struct.unpack_from(fmt, data, pos)
        if tag == 0:
            terminated = True
            break
        tags.setdefault(tag, []).append(value)
    if not terminated or 5 not in tags or 10 not in tags:
        raise ValueError(f'{name}: invalid dynamic string table')
    address, length = tags[5][0], tags[10][0]
    table = None
    for base, offset, size in loads:
        if base <= address and address + length <= base + size:
            start = offset + address - base
            table = data[start:start + length]
            break
    if table is None: raise ValueError(f'{name}: unmapped dynamic string table')
    def string(index):
        if index >= len(table): raise ValueError(f'{name}: invalid dynamic string offset')
        end = table.find(b'\0', index)
        if end < 0: raise ValueError(f'{name}: unterminated dynamic string')
        return table[index:end].decode('utf-8', errors='strict')
    return [string(index) for index in tags.get(1, [])], [string(index) for index in tags.get(14, [])]

def check_groups(groups, expected, require_plugin):
    if set(groups) != set(expected):
        raise ValueError(f'ABI mismatch: expected {sorted(expected)}, found {sorted(groups)}')
    for abi, libs in groups.items():
        for required in ('libcrypto_3.so', 'libssl_3.so'):
            if required not in libs: raise ValueError(f'{abi}: missing {required}')
        if require_plugin and not any('qopensslbackend' in x for x in libs):
            raise ValueError(f'{abi}: missing Qt OpenSSL TLS plugin')
        if require_plugin and not any('libplugins_multimedia_' in x for x in libs):
            raise ValueError(f'{abi}: missing Qt Multimedia media backend plugin')
        for name, data in libs.items():
            validate_elf(data, abi, name)
            needed, sonames = dynamic_dependencies(data, name)
            if name in ('libcrypto_3.so', 'libssl_3.so') and sonames != [name]:
                raise ValueError(f'{abi}/{name}: unexpected SONAME {sonames}')
            if name == 'libssl_3.so' and 'libcrypto_3.so' not in needed:
                raise ValueError(f'{abi}/{name}: must depend on libcrypto_3.so, got {needed}')
            if require_plugin or name in ('libcrypto_3.so', 'libssl_3.so'):
                missing = set(needed) - set(libs) - SYSTEM_LIBRARIES
                if missing: raise ValueError(f'{abi}/{name}: unresolved DT_NEEDED {sorted(missing)}')

def manifest_extract_native_libs(data):
    """Read the real binary Android manifest emitted by aapt, not source XML."""
    if len(data) < 8 or struct.unpack_from('<H', data)[0] != 3:
        raise ValueError('invalid binary Android manifest')
    strings = []
    position = struct.unpack_from('<H', data, 2)[0]
    def length(offset, utf8):
        if utf8:
            value = data[offset]
            return (((value & 127) << 8) | data[offset+1], offset+2) if value & 128 else (value, offset+1)
        value = struct.unpack_from('<H', data, offset)[0]
        return (((value & 32767) << 16) | struct.unpack_from('<H', data, offset+2)[0], offset+4) if value & 32768 else (value, offset+2)
    while position + 8 <= len(data):
        kind, header, size = struct.unpack_from('<HHI', data, position)
        if size < header or size < 8 or position + size > len(data): raise ValueError('invalid manifest chunk')
        if kind == 1:
            count, _, flags, start = struct.unpack_from('<IIII', data, position+8)
            if header + count*4 > size: raise ValueError('invalid manifest string pool')
            for index in range(count):
                offset = struct.unpack_from('<I', data, position+header+4*index)[0]
                current = position+start+offset
                if flags & 256:
                    _, current = length(current, True)
                    size_string, current = length(current, True)
                    strings.append(data[current:current+size_string].decode('utf-8'))
                else:
                    size_string, current = length(current, False)
                    strings.append(data[current:current+size_string*2].decode('utf-16-le'))
        if kind == 0x102:
            name = struct.unpack_from('<I', data, position+20)[0]
            if name >= len(strings): raise ValueError('invalid manifest element name')
            if strings[name] == 'application':
                start, step, count = struct.unpack_from('<HHH', data, position+24)
                if step < 20 or 16+start+step*count > size: raise ValueError('invalid application attributes')
                for index in range(count):
                    attribute = position+16+start+step*index
                    name = struct.unpack_from('<I', data, attribute+4)[0]
                    if name >= len(strings): raise ValueError('invalid manifest attribute name')
                    if strings[name] == 'extractNativeLibs':
                        if data[attribute+15] != 18: raise ValueError('extractNativeLibs must be boolean')
                        return struct.unpack_from('<I', data, attribute+16)[0] != 0
        position += size
    return None

def require_extracted_native_libraries(data):
    if manifest_extract_native_libs(data) is not True:
        raise ValueError('APK must set extractNativeLibs=true so Qt can discover its TLS plugin')

def signing_certificate_sha256(path):
    """Read the signing certificate from an APK v2 block; apksigner verifies it."""
    with path.open('rb') as stream:
        stream.seek(0, 2); size=stream.tell(); tail_size=min(size, 65557)
        stream.seek(size-tail_size);tail=stream.read(tail_size)
        end=tail.rfind(b'PK\x05\x06')
        if end < 0 or end+22 > len(tail): raise ValueError('APK central directory missing')
        central=struct.unpack_from('<I',tail,end+16)[0]
        if central < 24: raise ValueError('APK signing block missing')
        stream.seek(central-24);footer=stream.read(24)
        if footer[8:] != b'APK Sig Block 42': raise ValueError('APK v2 signing block missing')
        block_size=struct.unpack_from('<Q',footer)[0]
        if block_size > 16 * 1024 * 1024 or block_size < 24 or block_size+8 > central: raise ValueError('invalid APK signing block size')
        stream.seek(central-block_size-8);block=stream.read(block_size+8)
        if struct.unpack_from('<Q',block)[0] != block_size: raise ValueError('APK signing block size mismatch')
    def length_prefixed(data, offset=0):
        if offset+4 > len(data): raise ValueError('truncated APK signer')
        length=struct.unpack_from('<I',data,offset)[0];start=offset+4
        if start+length > len(data): raise ValueError('invalid APK signer length')
        return data[start:start+length],start+length
    pos=8
    while pos < len(block)-24:
        length=struct.unpack_from('<Q',block,pos)[0];pos+=8
        if length < 4 or pos+length > len(block)-24: raise ValueError('invalid APK signing entry')
        identifier=struct.unpack_from('<I',block,pos)[0]
        if identifier==0x7109871a:
            signers,_=length_prefixed(block[pos+4:pos+length])
            signer,end_signer=length_prefixed(signers)
            if end_signer != len(signers): raise ValueError('multiple APK signers are unsupported for this release identity')
            signed_data,_=length_prefixed(signer)
            _,end_digests=length_prefixed(signed_data)
            certificates,_=length_prefixed(signed_data,end_digests)
            certificate,_=length_prefixed(certificates)
            return hashlib.sha256(certificate).hexdigest()
        pos+=length
    raise ValueError('APK v2 signer not found')

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('package', nargs='?', type=Path)
    ap.add_argument('--libs', type=Path)
    ap.add_argument('--certificate-sha256', help='expected production signer certificate digest')
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
            if args.package.suffix == '.apk': require_extracted_native_libraries(z.read('AndroidManifest.xml'))
            for name in z.namelist():
                parts = name.split('/')
                if name.endswith('.so') and len(parts) >= 3 and parts[-3] == 'lib' and parts[-2] in MACHINES:
                    groups.setdefault(parts[-2], {})[parts[-1]] = z.read(name)
    else: ap.error('provide an APK/AAB or --libs directory')
    if not groups: raise ValueError('No Android native libraries found')
    expected = args.abis.split(',') if args.abis else groups.keys()
    check_groups(groups, expected, not args.libs)
    if args.certificate_sha256:
        if not args.package or signing_certificate_sha256(args.package) != args.certificate_sha256.lower(): raise ValueError('APK signer differs from the production upgrade identity')
    print('PASS: Android TLS SONAME/DT_NEEDED closure, ABI and 16 KiB ELF alignment:', ', '.join(sorted(groups)))

if __name__ == '__main__': main()
