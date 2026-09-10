#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
from __future__ import annotations
from pathlib import Path
import hashlib, os, subprocess, sys, zipfile

ROOT=Path(__file__).resolve().parents[2]
VERSION=os.environ.get('LMG_VERSION','0.7.0').strip() or '0.7.0'
OUT=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT.parent/f'LeoMiniGames-v{VERSION}-Source.zip'
PREFIX=f'LeoMiniGames-v{VERSION}-Source'
EXCLUDE_DIRS={'.git','.qtcreator','dist','__pycache__','.pytest_cache','.mypy_cache','.idea','.vscode'}
EXCLUDE_PREFIXES=('build','_cmake_probe')
EXCLUDE_TOP={'deployment'}  # backend/server material is intentionally maintained separately

validators=[
    'tools/source_guard.py','tools/validate_project.py','tools/sanity_check.py',
    'tools/security_audit.py','tools/validate_i18n.py','tools/validate_distribution.py',
    'tools/audit_prebuilt_binaries.py','tools/validate_v070.py'
]
for rel in validators:
    cp=subprocess.run([sys.executable,str(ROOT/rel)],cwd=ROOT)
    if cp.returncode:
        raise SystemExit(f'validator failed: {rel} ({cp.returncode})')

files=[]
for p in ROOT.rglob('*'):
    if not p.is_file():
        continue
    rel=p.relative_to(ROOT)
    parts=rel.parts
    if not parts:
        continue
    if parts[0] in EXCLUDE_TOP:
        continue
    if any(part in EXCLUDE_DIRS or any(part.startswith(pref) for pref in EXCLUDE_PREFIXES) for part in parts):
        continue
    if p.suffix=='.pyc' or p.name in {'.DS_Store'}:
        continue
    files.append((rel,p))
files.sort(key=lambda x:x[0].as_posix())

OUT.parent.mkdir(parents=True,exist_ok=True)
if OUT.exists(): OUT.unlink()
fixed=(2026,9,10,0,0,0)
with zipfile.ZipFile(OUT,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for rel,p in files:
        data=p.read_bytes()
        info=zipfile.ZipInfo(f'{PREFIX}/{rel.as_posix()}',fixed)
        info.compress_type=zipfile.ZIP_DEFLATED
        mode=p.stat().st_mode
        perms=0o755 if (mode & 0o111) else 0o644
        info.external_attr=(perms & 0xFFFF)<<16
        z.writestr(info,data,compress_type=zipfile.ZIP_DEFLATED,compresslevel=9)

sha=hashlib.sha256(OUT.read_bytes()).hexdigest()
sidecar=OUT.with_suffix(OUT.suffix + '.sha256')
sidecar.write_text(f'{sha}  {OUT.name}\n', encoding='utf-8')
sha_sums=OUT.parent/'SHA256SUMS'
sha_sums.write_text(f'{sha}  {OUT.name}\n', encoding='utf-8')
print(f'Created: {OUT}')
print(f'Files: {len(files)}')
print(f'SHA256: {sha}')
print(f'Checksum: {sidecar}')
print(f'SHA256SUMS: {sha_sums}')
