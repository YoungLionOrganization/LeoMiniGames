#!/usr/bin/env python3
from __future__ import annotations
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET
import yaml

ROOT = Path(__file__).resolve().parents[1]
PASS: list[str] = []
WARN: list[str] = []
ERR: list[str] = []

def ok(msg: str) -> None: PASS.append(msg)
def warn(msg: str) -> None: WARN.append(msg)
def err(msg: str) -> None: ERR.append(msg)
def read(rel: str) -> str: return (ROOT / rel).read_text(encoding='utf-8')
def require(rel: str) -> Path:
    p = ROOT / rel
    if p.is_file(): ok(f'{rel} exists')
    else: err(f'{rel} missing')
    return p

def load_workflow(rel: str):
    p = require(rel)
    if not p.is_file(): return None, ''
    raw = p.read_text(encoding='utf-8')
    try:
        data = yaml.load(raw, Loader=yaml.BaseLoader)
        if not isinstance(data, dict): raise ValueError('top level is not a mapping')
        ok(f'{rel} YAML parses')
        return data, raw
    except Exception as ex:
        err(f'{rel} YAML parse error: {ex}')
        return None, raw

def job(data, key: str):
    if not data: return None
    jobs = data.get('jobs')
    return jobs.get(key) if isinstance(jobs, dict) else None

def matrix_values(job_data, field: str) -> set[str]:
    if not isinstance(job_data, dict): return set()
    strategy = job_data.get('strategy')
    matrix = strategy.get('matrix') if isinstance(strategy, dict) else None
    include = matrix.get('include') if isinstance(matrix, dict) else None
    if not isinstance(include, list): return set()
    return {str(row.get(field)) for row in include if isinstance(row, dict) and row.get(field) is not None}

# Workflows -----------------------------------------------------------------
ci, cis = load_workflow('.github/workflows/ci.yml')
if ci:
    if re.search(r'(?m)^\s*tags\s*:', cis) or 'gh release create' in cis or 'action-gh-release' in cis:
        err('CI must not create releases/tags automatically')
    else:
        ok('CI has no tag/GitHub Release automation')

    expected_jobs = {
        'static-validation','linux','linux-distro','windows-msvc-x64',
        'windows-llvm-mingw','windows-arm64','macos','android',
        'android-universal','apple-mobile'
    }
    actual_jobs = set((ci.get('jobs') or {}).keys())
    missing = expected_jobs - actual_jobs
    if missing: err('CI jobs missing: ' + ', '.join(sorted(missing)))
    else: ok('CI contains all expected platform jobs')

    linux = job(ci, 'linux')
    runners = matrix_values(linux, 'runner')
    for v in {'ubuntu-22.04','ubuntu-24.04','ubuntu-24.04-arm'}:
        (ok if v in runners else err)(f'CI Linux runner {v}')

    distro = job(ci, 'linux-distro')
    distro_labels = matrix_values(distro, 'label')
    for label in {'Debian-13-x86_64','Debian-13-arm64','Arch-x86_64','Arch-arm64'}:
        (ok if label in distro_labels else err)(f'CI Linux distro target {label}')

    macos = job(ci, 'macos')
    for label in {'arm64','x86_64','universal'}:
        (ok if label in matrix_values(macos, 'label') else err)(f'CI macOS target {label}')

    android = job(ci, 'android')
    abis = matrix_values(android, 'abi')
    for abi in {'arm64-v8a','armeabi-v7a','x86_64','x86'}:
        (ok if abi in abis else err)(f'CI Android ABI {abi}')

    apple = job(ci, 'apple-mobile')
    apple_labels = matrix_values(apple, 'label')
    for label in {
        'iOS-device-arm64','iOS-simulator-arm64','iOS-simulator-x86_64',
        'iPadOS-device-arm64','iPadOS-simulator-arm64','iPadOS-simulator-x86_64'
    }:
        (ok if label in apple_labels else err)(f'CI Apple-mobile target {label}')

    required_semantics = [
        'win64_msvc2022_64','win64_llvm_mingw','win64_msvc2022_arm64',
        'windows-11-arm','linux_gcc_arm64','Debian-13','Arch-',
        'macos-15-intel','android-actions/setup-android@v4','host: all_os',
        'android-universal','QT_ANDROID_BUILD_ALL_ABIS=TRUE',
        'for qt_arch in android_arm64_v8a android_armv7 android_x86_64 android_x86',
        '--autodesktop','iPadOS-device-arm64','target: ios','arch: ios',
        'actions/setup-java@v5','bash "$QT_ROOT_DIR/bin/qt-cmake"',
    ]
    for token in required_semantics:
        (ok if token in cis else err)(f'CI semantic {token}')

    if re.search(r'install-qt all_os android[^\n]*\n\s*android_arm64_v8a android_armv7 android_x86_64 android_x86', cis):
        err('CI universal Android uses invalid multi-architecture positional aqt syntax')
    else:
        ok('CI universal Android installs one aqt architecture per invocation')

    haptics = read('src/core/Haptics.cpp')
    if '#include <QNativeInterface>' in haptics:
        err('Android Haptics still includes unavailable standalone QNativeInterface header')
    else:
        ok('Android Haptics has no standalone QNativeInterface include')

art, arts = load_workflow('.github/workflows/build-artifacts.yml')
if art:
    on_data = art.get('on')
    if not isinstance(on_data, dict) or set(on_data.keys()) != {'workflow_dispatch'}:
        err('Build artifacts workflow must remain workflow_dispatch-only')
    else:
        ok('Build artifacts workflow is manual-only')

    expected_jobs = {
        'validate','source','windows-x64','windows-arm64','linux-portable',
        'linux-native','macos','android','android-universal','apple-mobile'
    }
    actual_jobs = set((art.get('jobs') or {}).keys())
    missing = expected_jobs - actual_jobs
    if missing: err('Artifact jobs missing: ' + ', '.join(sorted(missing)))
    else: ok('Artifact workflow contains all expected jobs')

    linux = job(art, 'linux-portable')
    for label in {'Ubuntu-22.04-x86_64','Ubuntu-24.04-x86_64','Ubuntu-24.04-arm64'}:
        (ok if label in matrix_values(linux, 'label') else err)(f'Artifact Linux portable target {label}')

    native = job(art, 'linux-native')
    for label in {'Debian-13-x86_64','Debian-13-arm64','Arch-x86_64','Arch-arm64'}:
        (ok if label in matrix_values(native, 'label') else err)(f'Artifact Linux native target {label}')

    macos = job(art, 'macos')
    for label in {'arm64','x86_64','universal'}:
        (ok if label in matrix_values(macos, 'label') else err)(f'Artifact macOS target {label}')

    android = job(art, 'android')
    for abi in {'arm64-v8a','armeabi-v7a','x86_64','x86'}:
        (ok if abi in matrix_values(android, 'abi') else err)(f'Artifact Android ABI {abi}')

    apple = job(art, 'apple-mobile')
    for label in {
        'iOS-device-arm64','iOS-simulator-arm64','iOS-simulator-x86_64',
        'iPadOS-device-arm64','iPadOS-simulator-arm64','iPadOS-simulator-x86_64'
    }:
        (ok if label in matrix_values(apple, 'label') else err)(f'Artifact Apple-mobile target {label}')

    for token in [
        'package_windows.ps1','package_linux.sh','package_linux_native.sh',
        'package_macos.sh','package_android.sh','package_ios.sh',
        'install_qtifw_windows.ps1','windows-11-arm','ubuntu-24.04-arm',
        'Debian-13','Arch-','android-actions/setup-android@v4',
        'AppImage/appimagetool','linuxdeploy-plugin-qt','actions/upload-artifact@v7',
        'host: all_os','android-universal','QT_ANDROID_BUILD_ALL_ABIS=TRUE',
        'for qt_arch in android_arm64_v8a android_armv7 android_x86_64 android_x86',
        '--autodesktop','iPadOS-device-arm64',
        'QT_ANDROID_SIGN_APK=ON','QT_ANDROID_SIGN_AAB=ON',
        'bash tools/package/package_linux.sh build',
        'bash tools/package/package_linux_native.sh build',
        'bash tools/package/package_macos.sh build',
        'bash tools/package/package_android.sh build-android',
        'bash tools/package/package_ios.sh build-apple',
    ]:
        (ok if token in arts else err)(f'Artifact semantic {token}')

    if re.search(r'install-qt all_os android[^\n]*\n\s*android_arm64_v8a android_armv7 android_x86_64 android_x86', arts):
        err('Artifact universal Android uses invalid multi-architecture positional aqt syntax')
    else:
        ok('Artifact universal Android installs one aqt architecture per invocation')

    if re.search(r'(?m)^\s*archive\s*:', arts):
        err('Unsupported upload-artifact input "archive" is present')
    else:
        ok('Artifact upload steps avoid unsupported archive input')

# Installer -----------------------------------------------------------------
installer_required = [
    'installer/config/config.xml',
    'installer/config/leominigames_installer.ico',
    'installer/config/installer_window_icon.png',
    'installer/config/installer_logo.png',
    'installer/config/installer_pagelist.png',
    'installer/config/style.qss',
    'installer/packages/xyz.younglion.leominigames/meta/package.xml',
    'installer/packages/xyz.younglion.leominigames/meta/installscript.qs',
    'installer/packages/xyz.younglion.leominigames/meta/LICENSE.txt',
]
for rel in installer_required: require(rel)
try:
    config = ET.parse(ROOT/'installer/config/config.xml').getroot()
    pkg = ET.parse(ROOT/'installer/packages/xyz.younglion.leominigames/meta/package.xml').getroot()
    if config.findtext('Version') == '0.7.0' and pkg.findtext('Version') == '0.7.0': ok('QtIFW version 0.7.0')
    else: err('QtIFW version mismatch')
    if pkg.findtext('ForcedInstallation') == 'true' and pkg.findtext('Essential') == 'true' and pkg.findtext('Checkable') == 'false':
        ok('QtIFW application component cannot degrade to maintainer-only install')
    else:
        err('QtIFW application component selection contract incomplete')
    expected = {
        'InstallerApplicationIcon':'leominigames_installer',
        'InstallerWindowIcon':'installer_window_icon.png',
        'Logo':'installer_logo.png',
        'PageListPixmap':'installer_pagelist.png',
        'StyleSheet':'style.qss',
        'WizardStyle':'Modern',
    }
    for key, value in expected.items():
        (ok if config.findtext(key) == value else err)(f'QtIFW {key} configured')
except Exception as ex:
    err(f'QtIFW XML invalid: {ex}')

iscript = read('installer/packages/xyz.younglion.leominigames/meta/installscript.qs')
for token in ['iconPath=@TargetDir@/LeoMiniGames.exe','workingDirectory=@TargetDir@','@DesktopDir@/LeoMiniGames.lnk']:
    (ok if token in iscript else err)(f'Installer shortcut semantic {token}')

ifw_helper = require('tools/install_qtifw_windows.ps1')
if ifw_helper.is_file():
    helper = ifw_helper.read_text(encoding='utf-8')
    for token in ['windows_x86','windows_arm64','tools_ifw_411','qt.tools.ifw.411',
                  'Get-FileHash','SHA1','binarycreator.exe','QT_IFW_BIN']:
        (ok if token in helper else err)(f'QtIFW helper semantic {token}')

# App/application icons ------------------------------------------------------
for rel in ['resources/branding/leominigames.ico','resources/branding/leominigames.icns','resources/windows/leominigames.rc.in']:
    require(rel)
cmake = read('CMakeLists.txt')
for token in ['QT_ANDROID_APP_ICON "ic_launcher"','MACOSX_BUNDLE_ICON_FILE "leominigames.icns"','configure_file(resources/windows/leominigames.rc.in']:
    (ok if token in cmake else err)(f'CMake app icon semantic {token}')
for token in ['if(APPLE AND NOT IOS)','if(IOS)','XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY','qt_add_ios_ffmpeg_libraries']:
    (ok if token in cmake else err)(f'CMake Apple-mobile semantic {token}')

# Packaging -----------------------------------------------------------------
package_files = [
    'tools/package/package_linux.sh','tools/package/package_linux_native.sh',
    'tools/package/package_macos.sh','tools/package/package_android.sh',
    'tools/package/package_ios.sh','tools/package/package_windows.ps1'
]
for rel in package_files: require(rel)

linux_pack = read('tools/package/package_linux.sh')
if all(x in linux_pack for x in ['linuxdeploy','AppRun','LMG_PLATFORM_SUFFIX','NOTICE','COPYRIGHT','LICENSE_APPLICATION.md']):
    ok('Linux packaging is deployed, architecture-aware, and carries legal notices')
else: err('Linux portable packaging contract incomplete')

native_pack = read('tools/package/package_linux_native.sh')
if all(x in native_pack for x in ['LMG_PLATFORM_SUFFIX','NOTICE','COPYRIGHT','LICENSE_APPLICATION.md']):
    ok('Linux native packaging carries legal notices')
else: err('Linux native packaging contract incomplete')

win_pack = read('tools/package/package_windows.ps1')
if all(x in win_pack for x in ['windeployqt','binarycreator','LMG_PLATFORM_SUFFIX','NOTICE','COPYRIGHT','LICENSE_APPLICATION.md']):
    ok('Windows portable/installer packaging is architecture-aware and carries legal notices')
else: err('Windows deployment/installer tooling incomplete')

mac_pack = read('tools/package/package_macos.sh')
if all(x in mac_pack for x in ['macdeployqt','LMG_PLATFORM_SUFFIX','NOTICE','COPYRIGHT','LICENSE_APPLICATION.md']):
    ok('macOS packaging carries legal notices')
else: err('macOS packaging contract incomplete')

android_pack = read('tools/package/package_android.sh')
if all(x in android_pack for x in ['LMG_ANDROID_ABI','LMG_BUILD_AAB','Android-Legal.zip','LICENSE_APPLICATION.md']):
    ok('Android packaging is ABI-aware and emits legal sidecar')
else: err('Android packaging contract incomplete')

ios_pack = read('tools/package/package_ios.sh')
if all(x in ios_pack for x in ['LMG_PLATFORM_SUFFIX','LMG_APPLE_PLATFORM','LMG_APPLE_SDK','LICENSE_APPLICATION.md']):
    ok('Apple mobile packaging is platform/SDK-aware and carries legal notices')
else: err('Apple mobile packaging contract incomplete')

# Local Android build --------------------------------------------------------
for rel in ['build_android.bat','build_android.sh','ANDROID_BUILD.md']:
    require(rel)
for token in ['android_arm64_v8a','android_armv7','android_x86_64','android_x86','27.2.12479018','android-36','build-tools\\36.0.0']:
    (ok if token in read('build_android.bat') else err)(f'Windows Android builder semantic {token}')
manifest = read('android/AndroidManifest.xml')
if 'androidx.core.content.FileProvider' in manifest:
    err('Android manifest still has unused AndroidX FileProvider dependency')
else:
    ok('Android manifest has no unused AndroidX FileProvider')

# Licensing -----------------------------------------------------------------
license_required = [
    'LICENSE','LICENSE_APPLICATION.md','NOTICE','COPYRIGHT',
    'licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt',
    'licenses/YOUNGLION_MOD_LICENSE_1.0.txt',
    'licenses/README.md',
    'docs/OPEN_SOURCE_LICENSE_POLICY.md',
    'docs/TRADEMARK_POLICY.md',
    'docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md',
    'docs/LICENSING_ANALYSIS_REPORT.md',
    'docs/LICENSING_IMPLEMENTATION_GUIDE.md',
]
for rel in license_required: require(rel)
lic = read('LICENSE')
if 'GNU GENERAL PUBLIC LICENSE' in lic and 'Version 3, 29 June 2007' in lic and 'LEOMINIGAMES_PLUGIN_EXCEPTION' not in lic:
    ok('Canonical GPLv3 LICENSE remains unmodified by project-specific exception text')
else:
    err('Root LICENSE no longer looks like an unmodified canonical GPLv3 text')

application = read('LICENSE_APPLICATION.md')
for token in ['LeoMiniGames','Copyright (c) 2026 Cavanşir Qurbanzadə',
              'GPL-3.0-or-later','LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt']:
    (ok if token in application else err)(f'LICENSE_APPLICATION semantic {token}')

notice = read('NOTICE')
for token in ['GPL-3.0-or-later','LICENSE_APPLICATION.md',
              'LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt',
              'LicenseRef-YoungLion-Mod-License-1.0','TRADEMARK_POLICY.md']:
    (ok if token in notice else err)(f'NOTICE licensing semantic {token}')

copyright_text = read('COPYRIGHT')
for token in ['Copyright (c) 2026 Cavanşir Qurbanzadə','respective contributors']:
    (ok if token in copyright_text else err)(f'COPYRIGHT semantic {token}')

plugin_exception = read('licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt')
if ('GNU GPLv3' in plugin_exception or 'GNU GPL version 3' in plugin_exception) and ('Section 7' in plugin_exception or 'section 7' in plugin_exception):
    ok('Plugin/Mod Exception is expressed as GPLv3 section 7 additional permission')
else:
    err('Plugin/Mod Exception section-7 framing missing')

if read('src/sdk/IGamePlugin.h').startswith('// SPDX-License-Identifier: MIT OR GPL-3.0-or-later'):
    ok('Public SDK header has intended permissive-or-GPL SPDX expression')
else:
    err('IGamePlugin.h SPDX policy mismatch')

# F-Droid -------------------------------------------------------------------
fd = require('fdroid/metadata/xyz.younglion.leominigames.yml.example')
if fd.is_file():
    data = fd.read_text(encoding='utf-8')
    checks = {
        'version 0.7.0/700': 'versionName: 0.7.0' in data and 'versionCode: 700' in data,
        'canonical SourceCode URL': 'SourceCode: https://github.com/YoungLionOrganization/LeoMiniGames' in data,
        'canonical Git URL': 'Repo: https://github.com/YoungLionOrganization/LeoMiniGames.git' in data,
        'canonical issue tracker': 'IssueTracker: https://github.com/YoungLionOrganization/LeoMiniGames/issues' in data,
        'exact SHA placeholder only': 'FULL_COMMIT_SHA' in data and 'REPO_URL' not in data,
        'Qt source srclib precedent': 'Qt5@v6.10.1' in data,
        'NDK r27c': 'ndk: 27.2.12479018' in data,
    }
    for label, val in checks.items(): (ok if val else err)(f'F-Droid {label}')

require('tools/prepare_fdroid_metadata.py')
for rel in [
    'fastlane/metadata/android/en-US/title.txt',
    'fastlane/metadata/android/en-US/short_description.txt',
    'fastlane/metadata/android/en-US/full_description.txt',
    'fastlane/metadata/android/en-US/changelogs/700.txt',
    'fastlane/metadata/android/en-US/images/icon.png'
]:
    require(rel)

for short in sorted((ROOT/'fastlane/metadata/android').glob('*/short_description.txt')):
    value = short.read_text(encoding='utf-8').strip()
    if not value or len(value) >= 80 or value.endswith(('.', '。')):
        err(f'{short.relative_to(ROOT)} short description contract')
    else: ok(f'{short.relative_to(ROOT)} short-description contract')

ss = ROOT/'fastlane/metadata/android/en-US/images/phoneScreenshots'
if not ss.exists() or not any(ss.glob('*')): warn('No real upstream F-Droid screenshots yet; intentionally not faked')
else: ok('Upstream F-Droid screenshots present')

for rel in ['README.md','RELEASES.md','F_DROID_READINESS.md','fdroid/README.md']:
    text = read(rel)
    if 'canonical public repository' in text and 'unknown' in text.lower(): err(f'{rel} has stale repository wording')
    else: ok(f'{rel} F-Droid repository status not stale')

for rel in ['backend_ref','admin_ref','account_ref']:
    if (ROOT/rel).exists(): err(f'reference-only site tree bundled: {rel}')
if not any((ROOT/r).exists() for r in ['backend_ref','admin_ref','account_ref']): ok('reference-only backend/account/admin packages excluded')

print('Distribution validation')
for x in PASS: print('PASS:', x)
for x in WARN: print('WARNING:', x)
for x in ERR: print('ERROR:', x)
print(f'SUMMARY: {len(PASS)} PASS, {len(WARN)} WARNING, {len(ERR)} ERROR')
sys.exit(1 if ERR else 0)
