#!/usr/bin/env python3
"""Fail publication unless clean launch evidence covers the exact candidate."""
import argparse
import json
import re
from pathlib import Path

TARGETS = (
    'Android-arm64-v8a', 'Android-armeabi-v7a', 'Android-x86_64', 'Android-x86', 'Android-universal',
    'Windows-x86_64', 'Windows-x86_64-AVX2', 'Windows-ARM64',
    'Linux-Ubuntu-22.04-x86_64', 'Linux-Ubuntu-24.04-x86_64', 'Linux-Ubuntu-24.04-arm64',
    'Debian-13-x86_64', 'Debian-13-arm64', 'Arch-x86_64',
    'Fedora-43-x86_64', 'Fedora-43-arm64', 'Flatpak-x86_64', 'Flatpak-aarch64',
    'macOS-arm64', 'macOS-x86_64', 'macOS-universal',
    'iOS-device-arm64', 'iOS-simulator-x86_64', 'iPadOS-device-arm64', 'iPadOS-simulator-x86_64',
)

def target_artifacts(target):
    prefix = 'LeoMiniGames-v0.7.3-' + target
    if target.startswith('Android-'): return {prefix + '.apk'}
    if target.startswith('Windows-'): return {prefix + '.zip', prefix + '-Setup.exe'}
    if target.startswith('Linux-'): return {prefix + '.tar.gz', prefix + '.AppImage', prefix + '-Setup.run'}
    if target.startswith('Debian-'): return {prefix + '-native.tar.gz', prefix + '.deb'}
    if target.startswith('Arch-'): return {prefix + '-native.tar.gz', prefix + '.pkg.tar.zst'}
    if target.startswith('Fedora-'): return {prefix + '-native.tar.gz', prefix + '.rpm'}
    if target.startswith('Flatpak-'): return {'LeoMiniGames-v0.7.3-Linux-' + target.removeprefix('Flatpak-') + '.flatpak'}
    if target.startswith('macOS-'): return {prefix + '.zip', prefix + '.dmg'}
    return set()

def validate(report, sha, manifest=None):
    if report.get('source_sha') != sha: raise ValueError('QA source SHA differs from candidate')
    if report.get('version') != '0.7.3': raise ValueError('QA version mismatch')
    if manifest and manifest.get('source_sha') != sha: raise ValueError('artifact manifest source SHA differs from candidate')
    rows=report.get('targets', {})
    if set(rows) != set(TARGETS): raise ValueError('QA target coverage incomplete')
    for target, row in rows.items():
        if row.get('passed') is not True or row.get('clean_system') is not True: raise ValueError(f'{target}: clean launch not passed')
        for field in ('environment', 'artifact_sha256', 'evidence', 'tested_at', 'tester'):
            if not row.get(field): raise ValueError(f'{target}: missing {field}')
        if not re.fullmatch(r'[0-9a-f]{64}', row['artifact_sha256']): raise ValueError(f'{target}: invalid artifact digest')
        expected_artifacts = target_artifacts(target)
        if expected_artifacts:
            tested = row.get('tested_artifacts', {})
            if set(tested) != expected_artifacts: raise ValueError(f'{target}: not all package formats were tested')
            for artifact, digest in tested.items():
                if not re.fullmatch(r'[0-9a-f]{64}', digest): raise ValueError(f'{target}: invalid tested artifact digest')
                if manifest and (artifact not in manifest['assets'] or manifest['assets'][artifact]['sha256'] != digest): raise ValueError(f'{target}: tested package differs from candidate')
        if manifest and not target.startswith(('iOS-', 'iPadOS-')):
            artifact = row.get('artifact_name', '')
            if artifact not in target_artifacts(target) or artifact not in manifest['assets'] or manifest['assets'][artifact]['sha256'] != row['artifact_sha256']: raise ValueError(f'{target}: tested artifact differs from candidate')
        if target.startswith('Android-') and not row.get('device_abis'): raise ValueError(f'{target}: missing actual device ABI')
        if target.startswith('Windows-') and row.get('installer_upgrade_modify_uninstall') is not True: raise ValueError(f'{target}: installer servicing not passed')
        if target.startswith('macOS-') and row.get('gatekeeper_assessed') is not True: raise ValueError(f'{target}: Gatekeeper assessment missing')

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('report', type=Path);parser.add_argument('--sha', required=True);parser.add_argument('--manifest', type=Path)
    args=parser.parse_args(); validate(json.loads(args.report.read_text()), args.sha, json.loads(args.manifest.read_text()) if args.manifest else None)
    print('PASS: all candidate platform QA gates')
