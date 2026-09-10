#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
if len(sys.argv)!=2 or not re.fullmatch(r'[0-9a-fA-F]{40}', sys.argv[1]):
    raise SystemExit('usage: prepare_fdroid_metadata.py <40-character-commit-sha>')
src=ROOT/'fdroid/metadata/xyz.younglion.leominigames.yml.example'
out=ROOT/'fdroid/metadata/xyz.younglion.leominigames.yml'
text=src.read_text(encoding='utf-8').replace('FULL_COMMIT_SHA',sys.argv[1].lower())
out.write_text(text,encoding='utf-8')
print(out)
