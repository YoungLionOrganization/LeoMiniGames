#!/usr/bin/env python3
"""Generate SDK context-object signatures from the host service declarations."""
from pathlib import Path
import argparse
import json
import re

ROOT=Path(__file__).resolve().parents[2]
HEADERS={
'GameRuntime':'GameRuntime','GameSave':'GameSave','GameSettings':'GameSettings',
'GameAudio':'GameAudio','GameInput':'GameInput','Viewport':'GameViewport',
'GameTheme':'GameThemeFacade','GameI18n':'GameI18n','GameResources':'GameResources',
'Lifecycle':'GameLifecycleFacade','GameClock':'GameClock','GameRandom':'GameRandom',
'GameEvents':'GameEvents','GameStats':'GameStats','Achievements':'Achievements',
'Haptics':'Haptics','GameLogger':'GameLoggerFacade','App':'LegacyAppFacade',
'Settings':'LegacySettingsFacade','Audio':'LegacyAudioFacade','Lang':'LegacyLanguageFacade'}


def render():
    release=json.loads((ROOT/'release/release.json').read_text())
    runtime=(ROOT/'src/core/GameRuntime.h').read_text()
    api=re.search(r'apiVersion\(\) const.*?QStringLiteral\("([^\"]+)"\)',runtime,re.S).group(1)
    parts=[f'# Host service reference — API {api} / application {release["version"]}\n\n',
        'Generated from `src/core` declarations with `python3 tools/sdk/generate_api_reference.py`. Do not edit the signature lists manually. Check drift with `--check`.\n\n',
        'Qt context objects are available directly in QML; do not import the private application module. `QString`/`QUrl` become strings/URLs, `QStringList`/`QVariantList` become lists, `QVariantMap` becomes a JavaScript object, and `qreal`/`qint64` become numbers (JavaScript integer precision is limited to 2^53−1). Overloads listed below preserve legacy call shapes.\n\n',
        '**Properties are accessed without parentheses.** In particular use `GameRuntime.capabilities`, while other services with an invokable `capabilities()` use parentheses. `ready` means the service exists; `GameAudio.available`/`Haptics.available` probe their support. Required host capabilities do not guarantee an attached audio device, hardware input or network permission.\n\n',
        '`ThemeRuntime` in an external engine is an alias for the same scoped facade as `GameTheme`, not the host installer. `App.closeGame()` requests session navigation/teardown. Lifecycle attachment, `Viewport.update/windowInsets` and `GameInput.setFocusRoot/handleKey` are host integration concerns; games should use lifecycle callbacks, read viewport geometry and consume actions.\n\n']
    for context,header in HEADERS.items():
        text=(ROOT/'src/core'/f'{header}.h').read_text()
        text=re.sub(r'//[^\n]*','',text)
        properties=re.findall(r'Q_PROPERTY\(([^\n]*?)\)',text)
        functions=[]
        for match in re.finditer(r'Q_INVOKABLE\s+',text):
            start=match.end();opening=text.index('(',start);depth=1;end=opening+1
            while depth:
                if text[end]=='(':depth+=1
                elif text[end]==')':depth-=1
                end+=1
            tail=re.match(r'\s*const',text[end:])
            if tail:end+=tail.end()
            functions.append(text[start:end])
        slots=re.findall(r'public slots:\s*(.*?)(?=signals:|private:|protected:|$)',text,re.S)
        for block in slots: functions+=re.findall(r'([^;]+);',block)
        signals=[]
        for block in re.findall(r'signals:\s*(.*?)(?=private:|protected:|public:|$)',text,re.S):
            signals+=re.findall(r'([^;]+);',block)
        parts.append(f'\n## `{context}`\n\nSource: [src/core/{header}.h](../../src/core/{header}.h).\n')
        for title,entries in [('Properties',properties),('Methods',functions),('Signals',signals)]:
            if entries:
                parts.append(f'\n{title}:\n\n')
                for entry in entries:parts.append('- `'+re.sub(r'\s+',' ',entry).strip()+'`\n')
    parts.append('\nNative C++ plugins use a separate ABI: [Native SDK](NATIVE_API.md). Workflow examples and behavior details are indexed in [Overview](OVERVIEW.md).\n')
    return ''.join(parts)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    target=ROOT/'docs/sdk/API_REFERENCE.md';expected=render()
    if args.check:
        if not target.exists() or target.read_text()!=expected:raise SystemExit('API reference is stale; run generate_api_reference.py')
        print('PASS: SDK signatures match host headers');return
    target.write_text(expected);print('Generated:',target)

if __name__=='__main__':main()
