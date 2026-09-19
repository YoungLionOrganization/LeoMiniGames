#!/usr/bin/env python3
# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
# Source packages may contain media assets. These executable/library formats require manual release/legal review.
risky={'.so','.dll','.dylib','.exe','.aar','.jar','.apk','.aab','.ipa','.appimage','.bin','.zip','.7z','.rar'}
found=[]
for p in ROOT.rglob('*'):
    if p.is_file() and p.suffix.lower() in risky and not any(x in p.parts for x in ('build','dist')):
        found.append(str(p.relative_to(ROOT)))
if found:
    print('WARNING: prebuilt binary-like files found:')
    for x in found: print(' -',x)
    sys.exit(2)
print('PASS: no committed prebuilt binary or nested archive artifacts found in source tree')
