#!/usr/bin/env python3
from __future__ import annotations
from pathlib import Path
import re, sys, xml.etree.ElementTree as ET
import yaml

ROOT=Path(__file__).resolve().parents[1]
PASS=[]; WARN=[]; ERR=[]
def ok(x): PASS.append(x)
def warn(x): WARN.append(x)
def err(x): ERR.append(x)
def read(rel): return (ROOT/rel).read_text(encoding='utf-8')

def require(rel):
    p=ROOT/rel
    if p.is_file(): ok(f'{rel} exists'); return p
    err(f'{rel} missing'); return p

# CI: parse and enforce the user-requested Section 22 exclusion.
ci=require('.github/workflows/ci.yml')
if ci.is_file():
    try:
        yaml.safe_load(ci.read_text(encoding='utf-8')); ok('CI YAML parses')
    except Exception as e: err(f'CI YAML parse error: {e}')
    s=ci.read_text(encoding='utf-8')
    if re.search(r'(?m)^\s*tags\s*:',s) or 'gh release create' in s or 'action-gh-release' in s:
        err('Section 22 release/tag automation found in CI')
    else: ok('CI has no tag/GitHub Release automation')
    for platform in ['ubuntu-24.04','windows-2025','macos-15','macos-15-intel','target: android','target: ios']:
        (ok if platform in s else err)(f'CI platform token {platform}')

# Installer metadata must force-install the real application component.
for rel in ['installer/config/config.xml','installer/packages/xyz.younglion.leominigames/meta/package.xml','installer/packages/xyz.younglion.leominigames/meta/installscript.qs']:
    require(rel)
try:
    config=ET.parse(ROOT/'installer/config/config.xml').getroot()
    pkg=ET.parse(ROOT/'installer/packages/xyz.younglion.leominigames/meta/package.xml').getroot()
    if config.findtext('Version')=='0.7.0' and pkg.findtext('Version')=='0.7.0': ok('QtIFW version 0.7.0')
    else: err('QtIFW version mismatch')
    if pkg.findtext('ForcedInstallation')=='true' and pkg.findtext('Essential')=='true': ok('QtIFW application component forced/essential')
    else: err('QtIFW may install maintenance tool without application component')
except Exception as e: err(f'QtIFW XML invalid: {e}')

# Package scripts: paths, version and shell syntax contract.
package_files=['tools/package/package_linux.sh','tools/package/package_macos.sh','tools/package/package_android.sh','tools/package/package_ios.sh','tools/package/package_windows.ps1']
for rel in package_files:
    p=require(rel)
    if p.is_file() and '0.7.0' not in p.read_text(encoding='utf-8'): err(f'{rel} missing v0.7.0 artifact version')
if 'linuxdeploy' in read('tools/package/package_linux.sh') and 'AppRun' in read('tools/package/package_linux.sh'): ok('Linux archive uses deployed AppDir runtime')
else: err('Linux portable archive is not runtime-deployed')
if 'windeployqt' in read('tools/package/package_windows.ps1') and 'binarycreator' in read('tools/package/package_windows.ps1'): ok('Windows portable + QtIFW package paths')
else: err('Windows deployment/installer tooling missing')

# F-Droid: upstream metadata exists, submission recipe cannot pretend to have a real commit.
fd=require('fdroid/metadata/xyz.younglion.leominigames.yml.example')
if fd.is_file():
    data=fd.read_text(encoding='utf-8')
    if 'versionName: 0.7.0' in data and 'versionCode: 700' in data: ok('F-Droid template version 0.7.0/700')
    else: err('F-Droid template version mismatch')
    if 'FULL_COMMIT_SHA' in data and 'REPO_URL' in data: ok('F-Droid recipe remains explicit template until canonical full SHA is known')
    else: warn('F-Droid recipe appears activated; verify full immutable commit manually')
for rel in ['fastlane/metadata/android/en-US/title.txt','fastlane/metadata/android/en-US/short_description.txt','fastlane/metadata/android/en-US/full_description.txt','fastlane/metadata/android/en-US/changelogs/700.txt','fastlane/metadata/android/en-US/images/icon.png']:
    require(rel)
for short in sorted((ROOT/'fastlane/metadata/android').glob('*/short_description.txt')):
    value=short.read_text(encoding='utf-8').strip()
    if not value or len(value) >= 80 or value.endswith(('.', '。')):
        err(f'{short.relative_to(ROOT)} must be non-empty, under 80 characters and have no trailing dot')
    else:
        ok(f'{short.relative_to(ROOT)} short-description contract')
ss=ROOT/'fastlane/metadata/android/en-US/images/phoneScreenshots'
if not ss.exists() or not any(ss.glob('*')): warn('No real upstream F-Droid screenshots yet; intentionally not faked')
else: ok('Upstream F-Droid screenshots present')

# Backend/admin/account update packages are never part of the app source tree.
for rel in ['backend_ref','admin_ref','account_ref']:
    if (ROOT/rel).exists(): err(f'reference-only site tree bundled: {rel}')
if not any((ROOT/r).exists() for r in ['backend_ref','admin_ref','account_ref']): ok('reference-only backend/account/admin packages excluded')

print('Distribution validation')
for x in PASS: print('PASS:',x)
for x in WARN: print('WARNING:',x)
for x in ERR: print('ERROR:',x)
print(f'SUMMARY: {len(PASS)} PASS, {len(WARN)} WARNING, {len(ERR)} ERROR')
sys.exit(1 if ERR else 0)
