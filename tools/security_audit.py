#!/usr/bin/env python3
# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
from __future__ import annotations
import re, sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
errors=[]; warnings=[]; passes=[]
def read(rel): return (ROOT/rel).read_text(encoding='utf-8',errors='replace')
def must(rel,*tokens):
    s=read(rel)
    missing=[t for t in tokens if t not in s]
    if missing: errors.append(f"{rel}: missing security invariant(s): {missing}")
    else: passes.append(rel)

main=read('src/main.cpp')
external_block='\n'.join(line for line in main.splitlines() if 'externalServices.insert' in line)
for forbidden in ['QStringLiteral("Paths")','QStringLiteral("Plugins")','QStringLiteral("Mods")','&settings','&audio','&diagnostics','&theme']:
    if forbidden in external_block:
        errors.append(f"external QML receives forbidden raw service: {forbidden}")
if not any('forbidden raw service' in e for e in errors): passes.append('external service allowlist')

must('src/core/ExternalGameRuntime.cpp','new QQmlEngine','RestrictedNamFactory','scheme==QStringLiteral("https")','External games must load from validated qrc:/ resources.')
must('src/core/RccPackageInspector.cpp','outside its own /mods/<id> namespace','multiple package namespaces contain manifest.json')
must('src/core/DeveloperManager.cpp','ManualRedirectPolicy','kMaxAuthResponse','kMaxRccBytes','verified session copy','Authenticate a developer account before importing local RCC packages.')
must('src/core/ModManager.cpp','isOfficialCatalogOrigin','Third-party Catalog','entry.native = false','publisherVerification')
must('src/core/ThemeCatalogManager.cpp','isOfficialCatalogOrigin','Third-party Catalog','SameOriginRedirectPolicy')
must('src/core/GameLoggerFacade.h','Q_INVOKABLE void log')
logger=read('src/core/GameLoggerFacade.h')
if 'export' in logger.lower() or 'clear' in logger.lower(): errors.append('GameLoggerFacade exposes export/clear mutation')
else: passes.append('logger facade has no file export/clear')
must('src/core/LegacyAudioFacade.cpp','qrc:/mods/','normalized')
must('src/core/LegacySettingsFacade.cpp','isReservedHostKey','compat/mods/%1/%2','Copy-on-read migration')
must('src/main.cpp','gameSave.forceSave()')
must('src/core/PluginManager.cpp','kMaxTrustSnapshotBytes','kMaxNativePluginBytes','!defined(Q_OS_IOS)')
must('src/core/GameStats.cpp','kMaxStatsBytes','kMaxStatsEntries','safeMetricName')
must('src/core/Achievements.cpp','kMaxAchievementBytes','kMaxAchievementEntries','safeAchievementId')
must('src/core/GameSave.cpp','return dir.isEmpty()?QString{}','if(dirPath.isEmpty())return out')

# Raw developer secrets must not be stored in settings/files by DeveloperManager.
dev=read('src/core/DeveloperManager.cpp')
if re.search(r'(?i)(setValue|write|QSaveFile).*\b(key|token|authorization)\b',dev):
    warnings.append('DeveloperManager contains a possible secret persistence pattern; review manually')
else: passes.append('no obvious DeveloperManager credential persistence')

# Reject accidentally committed concrete developer API keys/tokens.
secret_re=re.compile(r'\blmg_[0-9a-f]{12}_[A-Za-z0-9_-]{40,60}\b')
for p in ROOT.rglob('*'):
    if not p.is_file() or any(part in {'.git','build','dist','__pycache__'} for part in p.parts): continue
    if p.suffix.lower() in {'.png','.jpg','.jpeg','.wav','.mp3','.ogg','.rcc','.zip','.ico'}: continue
    try: txt=p.read_text(encoding='utf-8')
    except Exception: continue
    if secret_re.search(txt): errors.append(f'concrete developer credential pattern committed in {p.relative_to(ROOT)}')

# Backend/Admin/Account references must not be in the application deliverable.
for name in ['backend_ref','admin_ref','account_ref']:
    if (ROOT/name).exists(): errors.append(f'reference tree bundled: {name}')

for p in passes: print('PASS:',p)
for w in warnings: print('WARNING:',w)
for e in errors: print('ERROR:',e)
print(f'SUMMARY: {len(passes)} PASS, {len(warnings)} WARNING, {len(errors)} ERROR')
sys.exit(1 if errors else 0)
