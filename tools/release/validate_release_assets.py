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
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from validate_audio_deployment import require_audio_backend


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
            entries = [m for m in archive.getmembers() if m.isfile() and m.size > 0]
            if not entries or not any(Path(m.name).name == 'LeoMiniGames' for m in entries): raise ValueError(f'{path.name}: missing Linux executable')
            require_audio_backend(m.name for m in entries)
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
