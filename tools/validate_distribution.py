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

# Workflows -----------------------------------------------------------------
ci=require('.github/workflows/ci.yml')
if ci.is_file():
    try: yaml.safe_load(ci.read_text(encoding='utf-8')); ok('CI YAML parses')
    except Exception as e: err(f'CI YAML parse error: {e}')
    s=ci.read_text(encoding='utf-8')
    if re.search(r'(?m)^\s*tags\s*:',s) or 'gh release create' in s or 'action-gh-release' in s:
        err('Section 22 release/tag automation found in CI')
    else: ok('CI has no tag/GitHub Release automation')
    required=[
        'ubuntu-22.04','ubuntu-24.04','ubuntu-24.04-arm',
        'windows-2022','win64_msvc2022_64','win64_llvm_mingw',
        'win64_msvc2022_arm64_cross_compiled','tools_llvm_mingw1706,qt.tools.win64_llvm_mingw1706','macos-15','macos-15-intel',
        'android_arm64_v8a','android_armv7','android_x86_64','android_x86',
        'target: ios','CMAKE_OSX_ARCHITECTURES=${{ matrix.arch }}',
        'bash "$QT_ROOT_DIR/bin/qt-cmake"','actions/setup-java@v5',
    ]
    for token in required:
        (ok if token in s else err)(f'CI token {token}')
    if '#include <QNativeInterface>' in read('src/core/Haptics.cpp'):
        err('Android Haptics still includes standalone QNativeInterface header')
    else: ok('Android Haptics uses Qt Core application header for native interface')

art=require('.github/workflows/build-artifacts.yml')
if art.is_file():
    try: yaml.safe_load(art.read_text(encoding='utf-8')); ok('Artifact workflow YAML parses')
    except Exception as e: err(f'Artifact workflow YAML parse error: {e}')
    a=art.read_text(encoding='utf-8')
    if re.search(r'(?m)^\s*(push|pull_request|release):',a) or re.search(r'(?m)^\s*tags\s*:',a) or 'gh release create' in a:
        err('Build artifacts workflow must remain manual-only')
    else: ok('Build artifacts workflow is manual-only')
    required=[
        'workflow_dispatch','package_windows.ps1','package_linux.sh','package_macos.sh',
        'package_android.sh','package_ios.sh','Windows-ARM64','ubuntu-24.04-arm',
        'android_arm64_v8a','android_armv7','android_x86_64','android_x86',
        'iOS-simulator','actions/upload-artifact@v7'
    ]
    for token in required:
        (ok if token in a else err)(f'Artifact workflow token {token}')

# Installer -----------------------------------------------------------------
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

# Packaging -----------------------------------------------------------------
for rel in ['tools/package/package_linux.sh','tools/package/package_macos.sh','tools/package/package_android.sh','tools/package/package_ios.sh','tools/package/package_windows.ps1']:
    require(rel)
if 'linuxdeploy' in read('tools/package/package_linux.sh') and 'AppRun' in read('tools/package/package_linux.sh') and 'LMG_PLATFORM_SUFFIX' in read('tools/package/package_linux.sh'):
    ok('Linux packaging is deployed and architecture-aware')
else: err('Linux portable packaging contract incomplete')
if 'windeployqt' in read('tools/package/package_windows.ps1') and 'binarycreator' in read('tools/package/package_windows.ps1') and 'LMG_PLATFORM_SUFFIX' in read('tools/package/package_windows.ps1'):
    ok('Windows portable + QtIFW packaging is architecture-aware')
else: err('Windows deployment/installer tooling incomplete')
if 'LMG_ANDROID_ABI' in read('tools/package/package_android.sh') and 'LMG_BUILD_AAB' in read('tools/package/package_android.sh'):
    ok('Android packaging names artifacts by ABI and supports APK/AAB selection')
else: err('Android packaging is not ABI-aware')
if 'LMG_PLATFORM_SUFFIX' in read('tools/package/package_ios.sh'):
    ok('iOS simulator packaging is architecture-aware')
else: err('iOS packaging missing architecture suffix')

# F-Droid -------------------------------------------------------------------
fd=require('fdroid/metadata/xyz.younglion.leominigames.yml.example')
if fd.is_file():
    data=fd.read_text(encoding='utf-8')
    checks={
        'version 0.7.0/700': 'versionName: 0.7.0' in data and 'versionCode: 700' in data,
        'canonical SourceCode URL': 'SourceCode: https://github.com/YoungLionOrganization/LeoMiniGames' in data,
        'canonical Git URL': 'Repo: https://github.com/YoungLionOrganization/LeoMiniGames.git' in data,
        'canonical issue tracker': 'IssueTracker: https://github.com/YoungLionOrganization/LeoMiniGames/issues' in data,
        'exact SHA placeholder only': 'FULL_COMMIT_SHA' in data and 'REPO_URL' not in data,
        'Qt source srclib precedent': 'Qt5@v6.10.1' in data,
        'NDK r27c': 'ndk: 27.2.12479018' in data,
    }
    for label,val in checks.items(): (ok if val else err)(f'F-Droid {label}')
require('tools/prepare_fdroid_metadata.py')
for rel in ['fastlane/metadata/android/en-US/title.txt','fastlane/metadata/android/en-US/short_description.txt','fastlane/metadata/android/en-US/full_description.txt','fastlane/metadata/android/en-US/changelogs/700.txt','fastlane/metadata/android/en-US/images/icon.png']:
    require(rel)
for short in sorted((ROOT/'fastlane/metadata/android').glob('*/short_description.txt')):
    value=short.read_text(encoding='utf-8').strip()
    if not value or len(value) >= 80 or value.endswith(('.', '。')):
        err(f'{short.relative_to(ROOT)} must be non-empty, under 80 chars and have no trailing dot')
    else: ok(f'{short.relative_to(ROOT)} short-description contract')
ss=ROOT/'fastlane/metadata/android/en-US/images/phoneScreenshots'
if not ss.exists() or not any(ss.glob('*')): warn('No real upstream F-Droid screenshots yet; intentionally not faked')
else: ok('Upstream F-Droid screenshots present')

# Docs should no longer claim repo URL is unknown.
for rel in ['README.md','RELEASES.md','F_DROID_READINESS.md','fdroid/README.md']:
    text=read(rel)
    if 'canonical public repository' in text and 'unknown' in text.lower():
        err(f'{rel} may still contain stale unknown-repository F-Droid wording')
    else: ok(f'{rel} F-Droid repository status not stale')

# Backend refs remain excluded.
for rel in ['backend_ref','admin_ref','account_ref']:
    if (ROOT/rel).exists(): err(f'reference-only site tree bundled: {rel}')
if not any((ROOT/r).exists() for r in ['backend_ref','admin_ref','account_ref']): ok('reference-only backend/account/admin packages excluded')

print('Distribution validation')
for x in PASS: print('PASS:',x)
for x in WARN: print('WARNING:',x)
for x in ERR: print('ERROR:',x)
print(f'SUMMARY: {len(PASS)} PASS, {len(WARN)} WARNING, {len(ERR)} ERROR')
sys.exit(1 if ERR else 0)
