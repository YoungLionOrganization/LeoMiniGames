#!/usr/bin/env python3
from pathlib import Path
import collections
import json
import re
import sys

root = Path(__file__).resolve().parents[1]
errors = []
doc = json.loads((root / 'resources/themes/default_theme.json').read_text(encoding='utf-8'))
tokens = doc.get('tokens', {})
aliases = doc.get('aliases', {})
surfaces = doc.get('surfaces', {})
colors = [k for k in tokens if k.startswith('color.')]
metric_aliases = [k for k, v in aliases.items() if isinstance(v, str) and (v.startswith('metric.') or v.startswith('alias.'))]

if len(colors) < 300:
    errors.append(f'base colors < 300: {len(colors)}')
if len(aliases) < 4000:
    errors.append(f'aliases < 4000: {len(aliases)}')
if len(metric_aliases) < 2000:
    errors.append(f'non-color/metric aliases < 2000: {len(metric_aliases)}')
if len(surfaces) < 8:
    errors.append(f'surfaces < 8: {len(surfaces)}')

required_alias_prefixes = (
    'alias.typography.', 'alias.motion.', 'alias.layout.', 'alias.button.',
    'alias.textField.', 'alias.card.', 'alias.gameTile.', 'alias.gameHud.',
    'alias.inventorySlot.', 'alias.virtualJoystick.', 'alias.touch.',
)
for prefix in required_alias_prefixes:
    if not any(k.startswith(prefix) for k in aliases):
        errors.append(f'missing alias family: {prefix}')

required_qml = [
    'Constants.qml', 'ThemeSurface.qml', 'ThemePolygonSurface.qml', 'TokenText.qml',
    'TokenPanel.qml', 'PressableSurface.qml', 'ThemeButton.qml', 'ThemeIconButton.qml',
    'ThemeTextField.qml', 'ThemeBadge.qml', 'ThemeDivider.qml', 'ThemeProgressBar.qml',
    'ThemeEmptyState.qml', 'ThemeToast.qml', 'SafeAreaItem.qml', 'AspectFrame.qml',
    'TouchTarget.qml', 'AdaptiveGrid.qml', 'GameBoardSurface.qml', 'GameTileSurface.qml',
    'GameHudBar.qml', 'InventorySlot.qml', 'VirtualActionPad.qml'
]
for name in required_qml:
    if not (root / 'qml' / name).exists():
        errors.append(f'missing reusable QML item: {name}')

# New and built-in QML must not hard-code colors or presentation numerics.
presentation = re.compile(
    r'(^|[;,{\s])(width|height|implicitWidth|implicitHeight|minimumWidth|minimumHeight|radius|spacing|padding|leftPadding|rightPadding|topPadding|bottomPadding|opacity|scale|duration|font\.pixelSize|border\.width|anchors\.[A-Za-z]+Margin)\s*:\s*-?\d+(?:\.\d+)?',
    re.MULTILINE,
)
for p in (root / 'qml').glob('*.qml'):
    if p.name == 'Constants.qml':
        continue
    text = p.read_text(encoding='utf-8')
    if re.search(r'#[0-9A-Fa-f]{6,8}', text):
        errors.append(f'hard-coded QML color: {p.name}')
    if presentation.search(text):
        errors.append(f'hard-coded QML presentation metric: {p.name}')
    if 'Theme.' in text:
        errors.append(f'legacy Theme ref in {p.name}')

# Constants must not accidentally declare the same property twice.
constants_text = (root / 'qml/Constants.qml').read_text(encoding='utf-8')
properties = re.findall(r'readonly property\s+\w+\s+(\w+)\s*:', constants_text)
for name, count in collections.Counter(properties).items():
    if count > 1:
        errors.append(f'duplicate Constants property: {name} x{count}')


# Every Constants.<name> reference must resolve to a property/function. This catches
# runtime [undefined] assignments that qmlcachegen does not reject.
constants_props = set(re.findall(r'\b(?:readonly\s+)?property\s+\w+\s+(\w+)\s*:', constants_text))
constants_funcs = set(re.findall(r'\bfunction\s+(\w+)\s*\(', constants_text))
constants_api = constants_props | constants_funcs
base_palette_pattern = re.compile(r'Constants\.(?:espresso|bronze|gold|cream|neutral)\w*')
for p in (root / 'qml').glob('*.qml'):
    if p.name == 'Constants.qml':
        continue
    text = p.read_text(encoding='utf-8')
    for match in re.finditer(r'Constants\.(\w+)', text):
        if match.group(1) not in constants_api:
            line = text[:match.start()].count('\n') + 1
            errors.append(f'undefined Constants property: {p.name}:{line} Constants.{match.group(1)}')
    if base_palette_pattern.search(text):
        errors.append(f'direct base-palette Constants usage bypasses semantic theme aliases: {p.name}')

# Alias graph: references must exist and cycles are forbidden.
all_values = {**tokens, **aliases}
for key, value in aliases.items():
    if isinstance(value, str) and value.startswith(('color.', 'metric.', 'alias.')) and value not in all_values:
        errors.append(f'alias missing reference: {key} -> {value}')
    seen = {key}
    current = value
    for _ in range(40):
        if not isinstance(current, str) or current not in all_values:
            break
        if current in seen:
            errors.append(f'alias cycle: {key} -> {current}')
            break
        seen.add(current)
        current = all_values[current]
    else:
        errors.append(f'alias chain too deep: {key}')

# Surface references must resolve.
def walk_surface(surface_key, value):
    if isinstance(value, dict):
        for child in value.values():
            walk_surface(surface_key, child)
    elif isinstance(value, list):
        for child in value:
            walk_surface(surface_key, child)
    elif isinstance(value, str) and value.startswith(('color.', 'metric.', 'alias.')) and value not in all_values:
        errors.append(f'surface missing reference: {surface_key} -> {value}')
for surface_key, spec in surfaces.items():
    walk_surface(surface_key, spec)

manager = (root / 'src/core/ThemeManager.cpp').read_text(encoding='utf-8')
for needle in ('updateThemeRcc', 'validateThemeResources', 'kMaxManifestBytes', 'recordContainsKey', 'resolveNested'):
    if needle not in manager:
        errors.append(f'ThemeManager missing hardening/API: {needle}')
if 'QFileInfo::absoluteFilePath(' in manager:
    errors.append('invalid static QFileInfo::absoluteFilePath call remains')

# Surface values are clamped and textures are local RCC resources only.
surface_qml = (root / 'qml/ThemeSurface.qml').read_text(encoding='utf-8')
for needle in ('safeNumber(', 'safeTextureSource', 'textureSource.indexOf("qrc:/")'):
    if needle not in surface_qml:
        errors.append(f'ThemeSurface missing runtime safety: {needle}')
if 'sanitizedThemeNumber' not in manager:
    errors.append('ThemeManager missing numeric sanitization')

if errors:
    print('Theme architecture validation FAILED')
    for e in errors:
        print('ERROR:', e)
    sys.exit(1)
print(f'Theme architecture OK: {len(colors)} base colors, {len(tokens)} tokens, {len(aliases)} aliases, {len(metric_aliases)} metric aliases, {len(surfaces)} surfaces')
