#!/usr/bin/env python3
from __future__ import annotations
import json, re, subprocess, sys
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
passes=[]; warnings=[]; errors=[]

def ok(msg): passes.append(msg)
def warn(msg): warnings.append(msg)
def err(msg): errors.append(msg)
def text(rel): return (ROOT/rel).read_text(encoding='utf-8')
def require(rel, needles):
    p=ROOT/rel
    if not p.is_file(): err(f'missing {rel}'); return
    s=p.read_text(encoding='utf-8')
    for n in needles:
        if n not in s: err(f'{rel}: missing contract token {n!r}')
    if all(n in s for n in needles): ok(f'{rel} contract')

cmake=text('CMakeLists.txt')
if 'project(LeoMiniGames VERSION 0.7.0' in cmake and 'LEOMINIGAMES_ANDROID_VERSION_CODE 700' in cmake: ok('version 0.7.0 / Android 700')
else: err('version mismatch: expected 0.7.0 / 700')
if 'QUIET COMPONENTS Multimedia' in cmake and 'LMG_HAS_MULTIMEDIA' in cmake: ok('Qt Multimedia capability-probed')
else: err('Qt Multimedia must remain optional/capability-probed')

# Section 22 was explicitly excluded by the user: CI is allowed, automated tag/release workflow is not.
wf=ROOT/'.github/workflows'
if wf.exists():
    release_files=[p for p in wf.glob('*.y*ml') if 'release' in p.name.lower()]
    tag_triggers=[]
    for p in wf.glob('*.y*ml'):
        s=p.read_text(encoding='utf-8')
        if re.search(r'(?m)^\s*tags\s*:', s) or 'softprops/action-gh-release' in s or 'gh release create' in s:
            tag_triggers.append(p.name)
    if release_files or tag_triggers: err(f'Section 22 exclusion violated: release automation present: {[p.name for p in release_files]+tag_triggers}')
    else: ok('Section 22 excluded: no tag/GitHub Release automation')
else: ok('Section 22 excluded: no release workflow directory')

require('src/core/LegacySettingsFacade.h',['Q_PROPERTY(bool soundEnabled','setValue','contains','remove'])
require('src/core/LegacyLanguageFacade.h',['availableLanguages','setLanguage','text('])
require('src/core/LegacyAudioFacade.h',['playUrl','preload','stopAll'])
require('src/core/GameLifecycleFacade.h',['void load()','void start()','void save()','void unload()'])
require('src/core/LegacyAppFacade.h',['currentGameId','currentGameUrl','closeGame'])
require('src/core/GameThemeFacade.h',['activeThemeId','activeThemeName','themeCount','applyTheme','surface'])
require('src/core/GameAudio.h',['void preload(const QUrl &url,qreal legacyGain)','void activate(const QString &gameId, bool restrictedExternal)'])
require('src/core/ExternalGameRuntime.cpp',['new QQmlEngine','RestrictedNamFactory','External games must load from validated qrc:/ resources'])
require('src/core/RccPackageInspector.cpp',['Legacy RCC tries to expose resources outside its own /mods/<id> namespace.','Legacy RCC is ambiguous: multiple package namespaces contain manifest.json.','result.mountRoot = QStringLiteral("/mods/%1")'])
require('src/core/GameRuntime.cpp',['legacy 0.5/0.6','save.atomic','legacy_rcc_mount'])
main=text('src/main.cpp')
for token in ['externalServices.insert(QStringLiteral("App")','legacySettings.activate(id)','legacyAudio.activate(id)','gameAudio.activate(id, external || developerRcc)','gameSave.forceSave()']:
    if token not in main: err(f'src/main.cpp missing compatibility/lifecycle token: {token}')
else:
    if all(t in main for t in ['externalServices.insert(QStringLiteral("App")','legacySettings.activate(id)','gameAudio.activate(id, external || developerRcc)']): ok('external compatibility facades activated per game')

# Ensure old raw Settings values can migrate but host-reserved keys are blocked.
settings=text('src/core/LegacySettingsFacade.cpp')
if 'Copy-on-read migration' in settings and 'isReservedHostKey' in settings and 'compat/mods/%1/%2' in settings: ok('legacy Settings copy-on-read migration')
else: err('legacy Settings migration/protection missing')

# Regression fixtures: parse manifests and QRC source paths.
fixtures=ROOT/'tests/fixtures/compat'
positive=['v0_5_canonical','v0_5_legacy_prefix','v0_6_services','v0_7_modern']
for name in positive:
    d=fixtures/name
    if not d.is_dir(): err(f'missing compatibility fixture {name}'); continue
    try:
        # manifest can be nested for legacy prefix
        manifests=list(d.rglob('manifest.json'))
        if len(manifests)!=1: raise ValueError(f'expected one manifest, found {len(manifests)}')
        m=json.loads(manifests[0].read_text(encoding='utf-8'))
        if not m.get('id') or not m.get('entry'): raise ValueError('missing id/entry')
        tree=ET.parse(d/'mod.qrc'); root=tree.getroot()
        for file_node in root.findall('.//file'):
            src=(d/(file_node.text or '')).resolve()
            if not src.is_file(): raise ValueError(f'QRC source missing: {file_node.text}')
        ok(f'compat fixture {name}')
    except Exception as e: err(f'fixture {name}: {e}')
mal=fixtures/'malicious_legacy_namespace'/'mod.qrc'
if mal.is_file():
    prefixes=[q.attrib.get('prefix','') for q in ET.parse(mal).getroot().findall('.//qresource')]
    if '/mods/malicious_legacy' in prefixes and '/themes' in prefixes: ok('malicious namespace regression fixture')
    else: err('malicious namespace fixture malformed')
else: err('missing malicious namespace fixture')


# Regression guards for bugs fixed during the v0.7 audit.
mod_manager=text('src/core/ModManager.cpp')
if mod_manager.count('for (const QJsonValue &value : document.array()) {') == 1: ok('installed-mod index is processed once')
else: err('installed-mod index loop duplicated or missing')
if 'object.value(QStringLiteral("required_capabilities"))' in mod_manager and 'networkAllowedFor' in mod_manager: ok('modern required capabilities feed runtime permission model')
else: err('required capabilities are not connected to runtime permissions')
game_save=text('src/core/GameSave.cpp')
if 'return dir.isEmpty()?QString{}' in game_save and 'if(dirPath.isEmpty())return out' in game_save: ok('GameSave empty game-id paths are fail-closed')
else: err('GameSave can derive a slot/list path with an empty game id')
touch=text('qml/TouchTarget.qml')
if not re.search(r'property\s+bool\s+enabled\b', touch): ok('TouchTarget uses inherited Item.enabled without shadowing')
else: err('TouchTarget shadows inherited Item.enabled')

# Publisher trust compatibility: legacy admin-only canonical catalog rows predate
# trust metadata and must still render as Official, while modern/third-party trust
# remains explicit and fail-closed.
require('src/core/PublisherTrustResolver.cpp',['authoritativeCatalog','legacyCatalogIsAdminOnly','legacyAdminFallback','publisher_type'])
require('src/core/ModManager.cpp',['resolvePublisherTrust(object, authoritativeCatalog, true)'])
require('src/core/ThemeCatalogManager.cpp',['resolvePublisherTrust(object, authoritativeCatalog, true)'])
require('tests/test_publisher_trust_resolver.cpp',['legacy admin mod','third-party legacy catalog','modern unverified'])

# i18n files must at least parse and the documented minimum languages must exist.
i18n=ROOT/'resources/i18n'
minimum=['en','tr','az','ru','de','fr','es','pt','zh_cn','ja']
for code in minimum:
    p=i18n/f'{code}.json'
    if not p.is_file(): err(f'missing host locale {code}')
    else:
        try: json.loads(p.read_text(encoding='utf-8'))
        except Exception as e: err(f'invalid locale JSON {code}: {e}')
if not any(f'missing host locale' in e or 'invalid locale' in e for e in errors): ok('minimum host locales parse')
if len(list(i18n.glob('*.json')))>=20: ok('20+ host locale resources')
else: warn('fewer than 20 host locale resources')

# Required SDK/audit outputs.
required_docs=['docs/sdk/OVERVIEW.md','docs/sdk/MANIFEST.md','docs/sdk/RESOURCES.md','docs/sdk/THEME_API.md','docs/sdk/I18N_API.md','docs/sdk/SAVE_API.md','docs/sdk/AUDIO_API.md','docs/sdk/INPUT_API.md','docs/sdk/LIFECYCLE_API.md','docs/sdk/DEVELOPER_MODE.md','docs/sdk/COMPATIBILITY.md','docs/sdk/MIGRATION_0.6_TO_0.7.md','COMPATIBILITY_REPORT.md','BUG_HUNTER_AUDIT.md','GAMER_AUDIT.md','F_DROID_READINESS.md','BUILD_MATRIX.md']
for rel in required_docs:
    if (ROOT/rel).is_file(): ok(f'doc {rel}')
    else: err(f'missing required document {rel}')

# No accidentally bundled reference backend/site trees.
for rel in ['backend_ref','admin_ref','account_ref']:
    if (ROOT/rel).exists(): err(f'backend/site reference tree must not be bundled: {rel}')
ok('backend/admin/account sources are not modified/bundled') if not any((ROOT/x).exists() for x in ['backend_ref','admin_ref','account_ref']) else None

# Run base static guards.
for tool in ['tools/source_guard.py','tools/validate_project.py','tools/sanity_check.py']:
    cp=subprocess.run([sys.executable,str(ROOT/tool)],cwd=ROOT,text=True,capture_output=True)
    if cp.returncode==0: ok(f'{tool} PASS')
    else: err(f'{tool} failed: {(cp.stdout+cp.stderr).strip()}')

print('LeoMiniGames v0.7.0 validator')
for x in passes: print('PASS:',x)
for x in warnings: print('WARNING:',x)
for x in errors: print('ERROR:',x)
print(f'SUMMARY: {len(passes)} PASS, {len(warnings)} WARNING, {len(errors)} ERROR')
sys.exit(1 if errors else 0)
