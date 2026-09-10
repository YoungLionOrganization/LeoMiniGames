#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
# Source packages may contain media assets. These executable/library formats require manual F-Droid review.
risky={'.so','.dll','.dylib','.exe','.aar','.jar','.apk','.aab','.ipa','.appimage','.bin'}
found=[]
for p in ROOT.rglob('*'):
    if p.is_file() and p.suffix.lower() in risky and not any(x in p.parts for x in ('build','dist')):
        found.append(str(p.relative_to(ROOT)))
if found:
    print('WARNING: prebuilt binary-like files found:')
    for x in found: print(' -',x)
    sys.exit(2)
print('PASS: no committed executable/library binary artifacts found in source tree')
