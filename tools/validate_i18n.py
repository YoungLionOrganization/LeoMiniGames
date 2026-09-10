#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
from __future__ import annotations
import json, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
I18N = ROOT / "resources" / "i18n"
QSTR = re.compile(r'qsTr\(\s*"((?:[^"\\]|\\.)*)"\s*\)')
PH = re.compile(r'%(?:L?\d+|n)|\{[A-Za-z_][A-Za-z0-9_]*\}')


def decode_qml_string(s: str) -> str:
    try:
        return json.loads('"' + s.replace('\\x', '\\\\x') + '"')
    except Exception:
        return s.replace('\\"', '"').replace('\\n', '\n').replace('\\t', '\t').replace('\\\\', '\\')

sources: set[str] = set()
for p in ROOT.rglob("*.qml"):
    text = p.read_text(encoding="utf-8", errors="replace")
    for m in QSTR.finditer(text):
        sources.add(decode_qml_string(m.group(1)))

errors: list[str] = []
warnings: list[str] = []
coverage: list[tuple[str,int,int,float]] = []
for p in sorted(I18N.glob("*.json")):
    code = p.stem
    try:
        obj = json.loads(p.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"{p.name}: invalid JSON: {exc}")
        continue
    if not isinstance(obj, dict):
        errors.append(f"{p.name}: root must be an object")
        continue
    if code == "en":
        coverage.append((code, len(sources), len(sources), 100.0))
        continue
    translated = 0
    for src in sources:
        if src not in obj:
            continue
        value = obj[src]
        if not isinstance(value, str) or not value.strip():
            errors.append(f"{p.name}: translation for {src!r} is empty/non-string")
            continue
        translated += 1
        if sorted(PH.findall(value)) != sorted(PH.findall(src)):
            errors.append(f"{p.name}: placeholder mismatch for {src!r}")
    pct = (translated / len(sources) * 100.0) if sources else 100.0
    coverage.append((code, translated, len(sources), pct))
    if translated < len(sources):
        warnings.append(f"{code}: {translated}/{len(sources)} source strings translated; runtime source-text fallback covers the rest")

print(f"Host source strings: {len(sources)}")
for code, have, total, pct in coverage:
    print(f"{code:6} {have:3}/{total:3} {pct:6.1f}%")
for w in warnings:
    print("WARNING:", w)
for e in errors:
    print("ERROR:", e)
print(f"SUMMARY: {len(errors)} ERROR, {len(warnings)} COVERAGE WARNING")
sys.exit(1 if errors else 0)
