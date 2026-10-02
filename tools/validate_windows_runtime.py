#!/usr/bin/env python3
"""Check imported MSVC runtime DLLs in every packaged PE, without executing it."""
import argparse
from pathlib import Path
import re
import struct


def imports(data):
    if data[:2] != b'MZ':
        raise ValueError('missing DOS signature')
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    if data[pe:pe+4] != b'PE\0\0':
        raise ValueError('missing PE signature')
    machine, sections = struct.unpack_from('<HH', data, pe+4)
    optional_size = struct.unpack_from('<H', data, pe+20)[0]
    optional = pe+24
    magic = struct.unpack_from('<H', data, optional)[0]
    if magic not in (0x10b, 0x20b):
        raise ValueError('unsupported optional header')
    directory = optional+(96 if magic == 0x10b else 112)
    rva, size = struct.unpack_from('<II', data, directory+8)
    table = optional+optional_size
    def offset(address):
        for n in range(sections):
            start = table+40*n
            virtual_size, virtual, raw_size, raw = struct.unpack_from('<IIII', data, start+8)
            if virtual <= address < virtual+max(virtual_size, raw_size):
                result = raw+address-virtual
                if result >= len(data): raise ValueError('import outside PE')
                return result
        raise ValueError('unmapped import RVA')
    names = []
    if rva:
        pos = offset(rva)
        for n in range(size//20):
            descriptor = struct.unpack_from('<IIIII', data, pos+20*n)
            if not any(descriptor): break
            name_offset = offset(descriptor[3])
            end = data.index(b'\0', name_offset)
            names.append(data[name_offset:end].decode('ascii').lower())
    return machine, names


def validate(stage):
    files = {p.name.lower(): p for p in stage.iterdir() if p.is_file()}
    application = files.get('leominigames.exe')
    if not application: raise ValueError('LeoMiniGames.exe missing')
    target_machine, _ = imports(application.read_bytes())
    required = set()
    for path in stage.rglob('*'):
        if not path.is_file() or path.suffix.lower() not in ('.exe', '.dll'): continue
        machine, dependencies = imports(path.read_bytes())
        if machine != target_machine: raise ValueError(f'{path.name}: wrong PE architecture')
        required.update(dep for dep in dependencies if re.match(r'^(vcruntime|msvcp|concrt|vcomp)\d', dep))
    for dep in sorted(required):
        if dep not in files: raise ValueError(f'missing app-local runtime: {dep}')
    if not required: raise ValueError('no MSVC runtime imports found; check package')
    return required


def deploy_runtime(stage, crt):
    """Copy only DLLs for the application's ISA, then enforce import closure.

    VS ARM64 CRT directories can also contain x64/ARM64EC helper DLLs. They
    must not be copied indiscriminately into a native ARM64 application.
    """
    import shutil
    target, _ = imports((stage/'LeoMiniGames.exe').read_bytes())
    for path in crt.glob('*.dll'):
        machine, _ = imports(path.read_bytes())
        if machine == target:
            shutil.copy2(path, stage/path.name)
        else:
            print(f'Skip {path.name}: PE machine 0x{machine:04x}, target 0x{target:04x}')
    return validate(stage)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(); parser.add_argument('stage', type=Path)
    parser.add_argument('--deploy-crt', type=Path)
    args = parser.parse_args()
    result = deploy_runtime(args.stage, args.deploy_crt) if args.deploy_crt else validate(args.stage)
    print('PASS: MSVC runtime closure:', ', '.join(sorted(result)))
