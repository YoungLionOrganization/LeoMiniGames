#!/usr/bin/env python3
"""Check local docs links, SDK service coverage and generated API drift."""
from pathlib import Path
from urllib.parse import unquote
import re
import sys
from sdk.generate_api_reference import HEADERS, render

ROOT=Path(__file__).resolve().parents[1]

def main():
    errors=[]
    paths=[ROOT/'README.md']
    for folder in ('docs','mod-sdk','theme-sdk','native-sdk'):paths+=list((ROOT/folder).rglob('*.md'))
    for path in paths:
        source=re.sub(r'```.*?```','',path.read_text(encoding='utf-8'),flags=re.S)
        for target in re.findall(r'!?\[[^\]\n]*\]\(([^)]+)\)',source):
            target=target.strip().split(' "',1)[0].strip('<>')
            if re.match(r'^[a-zA-Z][\w+.-]*:',target) or target.startswith('#'):continue
            relative=unquote(target.split('#',1)[0])
            if relative and not (path.parent/relative).exists():errors.append(f'{path.relative_to(ROOT)}: missing link {target}')
    contexts=set(re.findall(r'externalServices.insert\(QStringLiteral\("([^"]+)"\)',(ROOT/'src/main.cpp').read_text()))
    if contexts != set(HEADERS)|{'ThemeRuntime'}:errors.append('SDK reference service mapping differs from src/main.cpp')
    reference=ROOT/'docs/sdk/API_REFERENCE.md'
    if not reference.exists() or reference.read_text()!=render():errors.append('Generated SDK API reference is stale')
    for project in [ROOT/'mod-sdk/ExampleHelloMod',ROOT/'mod-sdk/ExampleModernMod',ROOT/'theme-sdk/ExampleTheme']:
        for bat in project.glob('*.bat'):
            content=bat.read_bytes()
            if bytes([92,114,92,110]) in content:errors.append(f'{bat.relative_to(ROOT)}: literal newline escapes in batch script')
    if errors:print('\n'.join(errors));return 1
    print(f'PASS: {len(paths)} documentation files; local links, context coverage and API signatures');return 0

if __name__=='__main__':raise SystemExit(main())
