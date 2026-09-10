#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
errors = []

def text(path):
    p = ROOT / path
    if not p.is_file():
        errors.append(f'missing: {path}')
        return ''
    return p.read_text(encoding='utf-8')

def require(path, needle):
    s = text(path)
    if needle not in s:
        errors.append(f'{path}: missing {needle!r}')

# Market models and UI split.
require('src/main.cpp', 'QStringLiteral("ThemesExplore")')
require('src/main.cpp', 'QStringLiteral("ThemesInstalled")')
require('src/core/ThemeInstalledModel.cpp', 'ThemeManager::themesChanged')
require('src/core/ThemeInstalledModel.cpp', 'themeInCatalog')
require('src/core/ThemeFilterProxyModel.cpp', 'themeTags')
require('src/core/ThemeFilterProxyModel.cpp', 'themePublisherStatus')
require('qml/ThemesPage.qml', 'model: ThemesExplore')
require('qml/ThemesPage.qml', 'model: ThemesInstalled')
require('qml/ThemesPage.qml', 'qsTr("EXPLORE")')
require('qml/ThemesPage.qml', 'qsTr("INSTALLED (%1)")')
require('qml/ThemesPage.qml', 'ThemesExplore.searchQuery = text')
require('qml/ThemesPage.qml', 'ThemesInstalled.searchQuery = text')
require('qml/ThemeMarketCard.qml', '!root.builtIn && !root.active')

# Theme Market access exists but themes do not become games.
require('qml/LibraryPage.qml', 'signal themesRequested()')
require('qml/LibraryPage.qml', 'iconName: "theme"')
require('qml/Main.qml', 'onThemesRequested: stack.push(themesComponent)')
themes_page = text('qml/ThemesPage.qml')
if re.search(r'\bGames\b|GameRegistry', themes_page):
    errors.append('ThemesPage must not use the game registry/main game model')

# Single-active invariant lives in ThemeManager, not just QML.
manager = text('src/core/ThemeManager.cpp')
header = text('src/core/ThemeManager.h')
if 'QString m_activeId;' not in header:
    errors.append('ThemeManager must store a single active theme id')
if 'm_activeId = id;' not in manager:
    errors.append('applyTheme must replace the single active theme id')
if 'm_activeId == id' not in manager:
    errors.append('ThemeManager active-theme checks missing')
if 'applyTheme(m_defaultId)' not in manager:
    errors.append('programmatic removal of an active external theme needs built-in fallback')

# Online install hardening.
catalog = text('src/core/ThemeCatalogManager.cpp')
for needle in (
    'entry.packageFormat != QStringLiteral("theme-rcc-v1")',
    'entry.sizeBytes <= 0',
    '^[a-f0-9]{64}$',
    'downloadUrl.scheme() != QStringLiteral("https")',
    'actualSha.compare(job->expectedSha',
    'job->received != job->expectedSize',
    'm_themes->installThemeRcc',
):
    if needle not in catalog:
        errors.append(f'ThemeCatalogManager hardening missing: {needle}')

# Theme package execution isolation.
for needle in ('validateThemeResources', 'isAllowedThemeResource', 'Theme packages may contain data/assets only'):
    if needle not in manager:
        errors.append(f'ThemeManager package isolation missing: {needle}')

if errors:
    print('Theme Market validation FAILED')
    for error in errors:
        print('ERROR:', error)
    sys.exit(1)
print('Theme Market validation OK')
