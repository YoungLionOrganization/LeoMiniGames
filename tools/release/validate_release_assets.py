#!/usr/bin/env python3
from __future__ import annotations

import argparse
import fnmatch
import json
import hashlib
import re
import sys
import subprocess
import tarfile
import zipfile
import tempfile
import io
import struct
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from validate_audio_deployment import require_audio_backend
from validate_android_package import dynamic_dependencies


def require_linux_binary(data, name):
    if len(data) < 64 or data[:6] != b'\x7fELF\x02\x01': raise ValueError(f'{name}: invalid 64-bit Linux ELF')
    machine = struct.unpack_from('<H', data, 18)[0]
    needed, _ = dynamic_dependencies(data, name)
    expected = 183 if any(x in name for x in ('arm64', 'aarch64')) else 62
    if machine != expected: raise ValueError(f'{name}: wrong Linux ELF architecture')
    if not any(dep.startswith('libQt6Multimedia.so') for dep in needed):
        raise ValueError(f'{name}: native binary is missing Qt Multimedia linkage')


def validate_linux_tar(archive, name, bundled=False, setup=False):
    members = archive.getmembers()
    for member in members:
        path = Path(member.name)
        if path.is_absolute() or '..' in path.parts: raise ValueError(f'{name}: unsafe archive member')
    entries = [m for m in members if m.isfile() and m.size > 0]
    binaries = [m for m in entries if Path(m.name).name == 'LeoMiniGames']
    if len(binaries) != 1: raise ValueError(f'{name}: expected one Linux executable')
    if not binaries[0].mode & 0o111: raise ValueError(f'{name}: Linux executable is not executable')
    require_linux_binary(archive.extractfile(binaries[0]).read(), name)
    if not any(Path(m.name).name == 'LICENSE' for m in entries): raise ValueError(f'{name}: license missing')
    if bundled:
        require_audio_backend(m.name for m in entries)
        if not any(m.name.endswith('/AppRun') and m.mode & 0o111 for m in entries):
            raise ValueError(f'{name}: executable AppRun missing')


def validate_linux_package(path):
    """Inspect package payloads without executing maintainer scripts."""
    if path.suffix == '.deb':
        metadata = subprocess.check_output(['dpkg-deb', '--field', str(path), 'Package'], text=True).strip()
        if metadata != 'leominigames': raise ValueError(f'{path.name}: incorrect DEB package identity')
        payload = subprocess.check_output(['dpkg-deb', '--fsys-tarfile', str(path)])
        with tarfile.open(fileobj=io.BytesIO(payload), mode='r:') as archive:
            validate_linux_tar(archive, path.name)
        return
    if path.suffix == '.rpm':
        identity = subprocess.check_output(['rpm', '-qp', '--qf', '%{NAME}', str(path)], text=True)
        if identity != 'leominigames': raise ValueError(f'{path.name}: incorrect RPM package identity')
        payload = subprocess.check_output(['rpm2cpio', str(path)])
    else:
        payload = path.read_bytes()
    names = subprocess.check_output(['bsdtar', '-tf', '-'], input=payload).decode().splitlines()
    for name in names:
        if name.startswith('/') or '..' in Path(name).parts: raise ValueError(f'{path.name}: unsafe package member')
    binary = next((name for name in names if name.removeprefix('./') == 'usr/bin/LeoMiniGames'), None)
    if not binary or not any(name.removeprefix('./') == 'usr/share/licenses/leominigames/LICENSE' for name in names):
        raise ValueError(f'{path.name}: executable/legal payload missing')
    if path.name.endswith('.pkg.tar.zst'):
        info = subprocess.check_output(['bsdtar', '-xOf', '-', '.PKGINFO'], input=payload).decode()
        if 'pkgname = leominigames\n' not in info: raise ValueError(f'{path.name}: incorrect Arch package identity')
    require_linux_binary(subprocess.check_output(['bsdtar', '-xOf', '-', binary], input=payload), path.name)


def validate_flatpak(path):
    with tempfile.TemporaryDirectory() as folder:
        repo = Path(folder)/'repo'
        subprocess.run(['ostree', f'--repo={repo}', 'init', '--mode=archive'], check=True, capture_output=True)
        subprocess.run(['flatpak', 'build-import', str(repo), str(path)], check=True, capture_output=True)
        refs = subprocess.check_output(['ostree', f'--repo={repo}', 'refs'], text=True).splitlines()
        arch = 'aarch64' if 'aarch64' in path.name else 'x86_64'
        ref = f'app/xyz.younglion.leominigames/{arch}/master'
        if ref not in refs: raise ValueError(f'{path.name}: incorrect Flatpak app identity/architecture')
        data = subprocess.check_output(['ostree', f'--repo={repo}', 'cat', ref, '/files/bin/LeoMiniGames'])
        require_linux_binary(data, path.name)
        subprocess.run(['ostree', f'--repo={repo}', 'cat', ref, '/files/share/doc/leominigames/LICENSE'],
                       check=True, capture_output=True)


def digest(path):
    sha = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''): sha.update(chunk)
    return sha.hexdigest()


def validate_content(path, root):
    if path.stat().st_size == 0: raise ValueError(f'{path.name}: empty artifact')
    with path.open('rb') as stream: signature = stream.read(16)
    if path.suffix in ('.zip', '.apk'):
        with zipfile.ZipFile(path) as archive:
            names = archive.namelist()
            if len(names) != len(set(names)): raise ValueError(f'{path.name}: duplicate archive entries')
            if archive.testzip(): raise ValueError(f'{path.name}: corrupt archive')
            entries = [i for i in archive.infolist() if not i.is_dir() and i.file_size > 0]
            if not entries: raise ValueError(f'{path.name}: empty archive')
            if path.name.endswith('-Source.zip') and not any(n.endswith('/CMakeLists.txt') or n == 'CMakeLists.txt' for n in names): raise ValueError('source archive missing CMakeLists.txt')
            if path.suffix == '.apk':
                if 'AndroidManifest.xml' not in names or not any(n.startswith('classes') and n.endswith('.dex') for n in names): raise ValueError(f'{path.name}: missing Android application metadata/code')
            if '-Windows-' in path.name or '-macOS-' in path.name:
                require_audio_backend(i.filename for i in entries)
            if '-Windows-' in path.name:
                required = {'leominigames.exe', 'vcruntime140.dll', 'msvcp140.dll'}
                if not required <= {Path(n).name.lower() for n in names}: raise ValueError(f'{path.name}: missing executable/runtime')
                with tempfile.TemporaryDirectory() as folder:
                    stage = Path(folder)
                    for name in names:
                        if name.endswith('/') or Path(name).suffix.lower() not in ('.exe', '.dll'): continue
                        relative = Path(name)
                        if relative.is_absolute() or '..' in relative.parts: raise ValueError('unsafe archive member')
                        target = stage/relative; target.parent.mkdir(parents=True, exist_ok=True); target.write_bytes(archive.read(name))
                    subprocess.run([sys.executable, str(root/'tools/validate_windows_runtime.py'), str(stage)], check=True)

            if '-macOS-' in path.name and not any(n.endswith('.app/Contents/MacOS/LeoMiniGames') for n in names): raise ValueError(f'{path.name}: missing macOS executable')
        if path.suffix == '.apk':
            abi = path.stem.split('-Android-', 1)[1]
            abis = 'arm64-v8a,armeabi-v7a,x86_64,x86' if abi == 'universal' else abi
            certificate = json.loads((root/'release/android-signing.json').read_text())['certificate_sha256']
            subprocess.run([sys.executable, str(root/'tools/validate_android_package.py'), str(path), '--abis', abis, '--certificate-sha256', certificate], check=True)
    elif path.name.endswith('.tar.gz'):
        with tarfile.open(path, 'r:gz') as archive:
            validate_linux_tar(archive, path.name, bundled=not path.name.endswith('-native.tar.gz'))
    elif path.suffix == '.run':
        data = path.read_bytes()
        marker = b'\n__LMG_PAYLOAD_BELOW__\n'
        if not data.startswith(b'#!/usr/bin/env bash\n') or marker not in data:
            raise ValueError(f'{path.name}: invalid Linux installer')
        with tarfile.open(fileobj=io.BytesIO(data.split(marker, 1)[1]), mode='r:gz') as archive:
            validate_linux_tar(archive, path.name, bundled=True)
    elif path.suffix in ('.deb', '.rpm') or path.name.endswith('.pkg.tar.zst'):
        validate_linux_package(path)
    elif path.suffix == '.flatpak':
        validate_flatpak(path)
    elif path.suffix == '.exe':
        if signature[:2] != b'MZ': raise ValueError(f'{path.name}: invalid PE')
    elif path.suffix == '.AppImage':
        if signature[:4] != b'\x7fELF' or signature[8:11] not in (b'AI\x01', b'AI\x02'): raise ValueError(f'{path.name}: invalid AppImage')
    elif path.suffix == '.dmg':
        with path.open('rb') as stream:
            if path.stat().st_size < 512: raise ValueError(f'{path.name}: invalid DMG')
            stream.seek(-512, 2)
            if stream.read(4) != b'koly': raise ValueError(f'{path.name}: invalid DMG trailer')
    else: raise ValueError(f'{path.name}: unknown artifact format')


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    parser.add_argument('--remote-json', type=Path, help='GitHub release asset metadata to verify size and digest')
    parser.add_argument('--manifest', type=Path, help='write internal SHA-256 manifest outside the public asset directory')
    parser.add_argument('--source-sha', help='immutable source commit for artifact provenance')
    args = parser.parse_args()
    if args.source_sha and not re.fullmatch(r'[0-9a-f]{40}', args.source_sha): parser.error('source SHA must be a full commit hash')
    root = Path(__file__).resolve().parents[2]
    policy = json.loads((root / "release/assets.json").read_text(encoding="utf-8"))
    version = policy["version"]
    directory = args.directory.resolve()
    files = [p for p in directory.iterdir() if p.is_file()]
    if any(p.is_dir() for p in directory.iterdir()):
        raise SystemExit("release-assets must not contain directories")

    expected_names: set[str] = set()
    for entry in policy["assets"]:
        pattern = entry["pattern"].format(version=version)
        matches = [p for p in files if fnmatch.fnmatchcase(p.name, pattern)]
        minimum = int(entry.get("min_count", 1 if entry.get("required") else 0))
        maximum = int(entry.get("max_count", max(1, minimum)))
        if not minimum <= len(matches) <= maximum:
            raise SystemExit(f"{entry['id']}: expected {minimum}..{maximum}, got {len(matches)}")
        expected_names.update(p.name for p in matches)

    actual_names = {p.name for p in files}
    extras = sorted(actual_names - expected_names)
    if extras:
        raise SystemExit(f"unexpected public assets: {extras}")
    exact = {name.format(version=version) for name in policy['expected_names']}
    if actual_names != exact: raise SystemExit(f'exact asset set mismatch; missing={sorted(exact-actual_names)}, extra={sorted(actual_names-exact)}')
    manifest = {}
    for path in files:
        validate_content(path, root)
        manifest[path.name] = {'size': path.stat().st_size, 'sha256': digest(path)}
    if args.remote_json:
        remote = json.loads(args.remote_json.read_text())
        if len(remote) != len(manifest) or {a['name'] for a in remote} != set(manifest): raise SystemExit('remote asset set mismatch')
        for asset in remote:
            expected = manifest[asset['name']]
            if asset.get('size') != expected['size'] or asset.get('digest') != 'sha256:'+expected['sha256']: raise SystemExit(f"remote size/digest mismatch: {asset['name']}")
    if args.manifest:
        if args.manifest.resolve().is_relative_to(directory): raise SystemExit('internal manifest must be outside public assets')
        args.manifest.write_text(json.dumps({'version': version, 'source_sha': args.source_sha, 'assets': manifest}, indent=2)+'\n')
    for name in actual_names:
        for pattern in policy.get("forbidden_public_patterns", []):
            if fnmatch.fnmatchcase(name, pattern):
                raise SystemExit(f"forbidden public asset: {name}")
    print(f"PASS: {len(files)} validated public release assets")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
