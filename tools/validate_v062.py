#!/usr/bin/env python3
from pathlib import Path
import json
import re
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
errors = []

def require(path, needle=None):
    p = ROOT / path
    if not p.is_file():
        errors.append(f'missing: {path}')
        return
    if needle is not None and needle not in p.read_text(encoding='utf-8'):
        errors.append(f'{path}: missing {needle!r}')

require('CMakeLists.txt', 'project(LeoMiniGames VERSION 0.6.2')
require('CMakeLists.txt', 'LEOMINIGAMES_ANDROID_VERSION_CODE 620')
require('src/main.cpp', 'QStringLiteral("0.6.2")')
require('android/AndroidManifest.xml', 'android:versionName="0.6.2"')
require('android/AndroidManifest.xml', 'android:versionCode="620"')
require('qml/Constants.qml', 'BEGIN GENERATED SEMANTIC ALIASES 0.6.2')
require('src/core/ThemeManager.h', 'updateThemeRcc')
require('src/core/ThemeManager.cpp', 'Theme packages may contain data/assets only')
require('docs/THEMING.md')
require('docs/UI_COMPONENTS.md')
require('V0.6.2_CHANGES.md')
require('qml/LibraryPage.qml', 'signal themesRequested()')
require('qml/ThemesPage.qml', 'model: ThemesInstalled')
require('qml/ThemesPage.qml', 'model: ThemesExplore')
require('src/core/ThemeFilterProxyModel.h', 'class ThemeFilterProxyModel')
require('src/core/ThemeInstalledModel.h', 'class ThemeInstalledModel')

# New QML files must all be in the module.
qml_cmake = (ROOT / 'qml/CMakeLists.txt').read_text(encoding='utf-8')
for p in (ROOT / 'qml').glob('*.qml'):
    if p.name not in qml_cmake:
        errors.append(f'QML not listed in module: {p.name}')

# Theme package example must remain data-only.
allowed = {'.json', '.png', '.webp', '.jpg', '.jpeg', '.svg', '.ttf', '.otf', '.woff2', '.qrc', '.md'}
for p in (ROOT / 'theme-sdk/ExampleTheme').rglob('*'):
    if p.is_file() and p.suffix.lower() not in allowed:
        errors.append(f'example theme contains executable/unsupported file: {p.relative_to(ROOT)}')

# JSON/XML and simple QML delimiter smoke tests.
for p in ROOT.rglob('*.json'):
    try:
        json.loads(p.read_text(encoding='utf-8'))
    except Exception as exc:
        errors.append(f'invalid JSON {p.relative_to(ROOT)}: {exc}')
try:
    ET.parse(ROOT / 'android/AndroidManifest.xml')
except Exception as exc:
    errors.append(f'invalid Android manifest: {exc}')
for p in (ROOT / 'qml').glob('*.qml'):
    text = p.read_text(encoding='utf-8')
    if text.count('{') != text.count('}'):
        errors.append(f'brace mismatch: {p.name}')

# Protect RCC v1 compatibility and existing context APIs.
for rel in ('mod-sdk/ExampleHelloMod/manifest.json', 'mod-sdk/ExampleModernMod/manifest.json'):
    manifest = json.loads((ROOT / rel).read_text(encoding='utf-8'))
    if manifest.get('package_format', 'rcc-v1') != 'rcc-v1':
        errors.append(f'RCC format regression: {rel}')
main = (ROOT / 'src/main.cpp').read_text(encoding='utf-8')
for context in ('Audio', 'Settings', 'Plugins', 'Games', 'GameSave', 'GameSettings', 'GameTheme', 'ThemesExplore', 'ThemesInstalled'):
    if f'setContextProperty(QStringLiteral("{context}")' not in main:
        errors.append(f'missing context API: {context}')

if errors:
    print('LeoMiniGames v0.6.2 validation FAILED')
    for error in errors:
        print('ERROR:', error)
    sys.exit(1)
print('LeoMiniGames v0.6.2 validation OK')
