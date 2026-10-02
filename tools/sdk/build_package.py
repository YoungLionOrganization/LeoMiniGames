#!/usr/bin/env python3
"""Validate and compile a canonical mod/theme SDK project using Qt 6 rcc."""
from pathlib import Path, PurePosixPath
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

ID = re.compile(r'^[a-z0-9][a-z0-9_.-]{1,63}$')
VERSION = re.compile(r'^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-[0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*)?(?:\+[0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*)?$')
THEME_ASSETS = {'json','png','webp','jpg','jpeg','svg','ttf','otf','woff2'}


def relative(value):
    return isinstance(value, str) and bool(value) and '\\' not in value and ':' not in value and '..' not in value and not value.startswith('/') and all(p not in ('', '.', '..') for p in value.split('/'))


def validate(project):
    project = Path(project).resolve()
    kind = 'mod' if (project/'mod.qrc').is_file() else 'theme'
    qrc = project/('mod.qrc' if kind == 'mod' else 'theme.qrc')
    manifest = project/('manifest.json' if kind == 'mod' else 'theme/theme.json')
    document = json.loads(manifest.read_text(encoding='utf-8'))
    if not isinstance(document, dict): raise ValueError('Manifest must be an object')
    identifier = document.get('id', '')
    pattern = ID if kind == 'mod' else re.compile(r'^[a-z0-9][a-z0-9_.-]{1,127}$')
    if not isinstance(identifier,str) or not pattern.fullmatch(identifier): raise ValueError('Invalid package id')
    if not isinstance(document.get('name'),str) or not document['name'].strip(): raise ValueError('Package name is required')
    version = document.get('version', '1.0.0' if kind == 'theme' else '')
    if not isinstance(version,str) or not VERSION.fullmatch(version): raise ValueError('Use a SemVer package version')
    if '-' in version.split('+')[0]:
        for part in version.split('+')[0].split('-',1)[1].split('.'):
            if part.isdigit() and len(part)>1 and part.startswith('0'): raise ValueError('Invalid numeric prerelease version')
    names = {}
    for group in ET.parse(qrc).getroot().findall('qresource'):
        prefix=group.attrib.get('prefix','/')
        if kind=='mod' and prefix!='/': raise ValueError('New mod SDK projects use qresource prefix="/"; the host mounts the namespace')
        if kind=='theme' and prefix not in ('/','/theme'): raise ValueError('Theme SDK resources must remain below /theme')
        if group.attrib.get('lang'): raise ValueError('Use i18n JSON resources instead of language-dependent RCC entries')
        for entry in group.findall('file'):
            source = (entry.text or '').strip()
            alias = entry.attrib.get('alias', source)
            if not relative(alias): raise ValueError(f'Invalid RCC alias: {alias}')
            path = (project/source).resolve()
            if not path.is_relative_to(project) or not path.is_file(): raise ValueError(f'Missing/outside project resource: {source}')
            if kind=='theme' and prefix=='/theme': alias='theme/'+alias
            if alias in names: raise ValueError(f'Duplicate RCC alias: {alias}')
            if kind == 'theme' and (not alias.startswith('theme/') or PurePosixPath(alias).suffix[1:].lower() not in THEME_ASSETS): raise ValueError(f'Forbidden theme resource: {alias}')
            if path.suffix.lower()=='.json': json.loads(path.read_text(encoding='utf-8'))
            if kind == 'mod' and path.suffix.lower()=='.qml' and re.search(r'^\s*import\s+LeoMiniGames\b',path.read_text(encoding='utf-8'),re.M): raise ValueError('External games cannot import the private LeoMiniGames QML module')
            names[alias] = path
    required = ['manifest.json', document.get('entry')] if kind == 'mod' else ['theme/theme.json']
    if kind == 'mod':
        if not relative(document.get('entry')): raise ValueError('A safe relative entry file is required')
        if document.get('package_format','rcc-v1') != 'rcc-v1': raise ValueError('Unsupported package format')
        for key in ('entry','icon_path','license_file'):
            if key in document:
                if not relative(document[key]): raise ValueError(f'Invalid {key}')
                required.append(document[key])
        schema = document.get('save_version',1)
        if type(schema) is not int or schema<1: raise ValueError('save_version must be a positive integer')
        for key in ('api_version','min_api_version'):
            if key in document and (not isinstance(document[key],str) or not re.fullmatch(r'0\.\d+(?:\.\d+)?',document[key])): raise ValueError(f'Invalid {key}')
        for key in ('capabilities','required_capabilities','locales','tags'):
            if key in document and (not isinstance(document[key],list) or any(not isinstance(x,str) or not x for x in document[key]) or len(set(document[key]))!=len(document[key])): raise ValueError(f'Invalid {key} list')
        for locale in document.get('locales',[]): required.append(f'i18n/{locale}.json')
    else:
        if type(document.get('theme_api_version',1)) is not int or document.get('theme_api_version',1)!=1: raise ValueError('Theme API must be integer 1')
        if 'tokens' not in document: raise ValueError('Theme tokens object is required')
        for key in ('tokens','aliases','surfaces'):
            if not isinstance(document.get(key,{}),dict): raise ValueError(f'{key} must be an object')
    for alias in required:
        if alias not in names: raise ValueError(f'Required file missing from RCC: {alias}')
    return kind, document, qrc


def build(project, output, rcc):
    kind, document, qrc = validate(project)
    probe=subprocess.run([rcc,'--version'],capture_output=True,text=True,check=True)
    if not re.search(r'\b6\.',probe.stdout+probe.stderr): raise ValueError('Use Qt 6 rcc; Qt 5 is not supported')
    output=Path(output).resolve();output.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='lmg-sdk-',dir=output.parent) as folder:
        temporary=Path(folder)/'package.rcc'
        # No zstd compression: do not require a host Qt build with optional zstd.
        subprocess.run([rcc,'--binary','--compress-algo','zlib',str(qrc),'-o',str(temporary)],cwd=qrc.parent,check=True)
        temporary.replace(output)
    checksum=hashlib.sha256(output.read_bytes()).hexdigest()
    Path(str(output)+'.sha256').write_text(f'{checksum}  {output.name}\n',encoding='utf-8')
    print(f'Built {kind}: {output}\nSHA256: {checksum}')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('project',type=Path)
    parser.add_argument('--output',type=Path)
    parser.add_argument('--rcc',default=shutil.which('rcc') or shutil.which('rcc6') or 'rcc')
    parser.add_argument('--check',action='store_true',help='validate source/QRC resources without compiling')
    args=parser.parse_args()
    try:
        kind, document, _=validate(args.project)
        if args.check: print(f'PASS: {kind} SDK project {document["id"]}');return 0
        output=args.output or args.project/'build'/f'{document["id"]}-{document.get("version","1.0.0")}.rcc'
        build(args.project,output,args.rcc)
    except (ValueError,OSError,ET.ParseError,subprocess.CalledProcessError) as error:
        print(f'SDK build failed: {error}',file=sys.stderr);return 1
    return 0


if __name__=='__main__':
    raise SystemExit(main())
