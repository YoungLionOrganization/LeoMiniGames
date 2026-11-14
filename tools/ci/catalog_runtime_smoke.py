#!/usr/bin/env python3
"""Exercise bounded, hash-verified live catalog RCCs through the real host."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import urllib.parse
import urllib.request

BASE = 'https://leominigames.younglion.xyz'


def fetch(url, limit):
    parsed = urllib.parse.urlparse(url)
    if parsed.scheme != 'https' or parsed.username is not None:
        raise ValueError('Catalog requests must use credential-free HTTPS')
    with urllib.request.urlopen(url, timeout=30) as response:
        if urllib.parse.urlparse(response.geturl()).scheme != 'https':
            raise ValueError('Download redirected outside HTTPS')
        data = response.read(limit+1)
    if len(data) > limit:
        raise ValueError('Catalog response exceeds the test size limit')
    return data


def run(app, folder, env):
    try:
        result = subprocess.run([app, '--smoke-test'], env=env, capture_output=True, timeout=45)
        output = result.stdout + result.stderr
        passed = result.returncode == 0
        status = str(result.returncode)
    except subprocess.TimeoutExpired as error:
        output = (error.stdout or b'') + (error.stderr or b'')
        passed, status = False, 'timeout'
    (folder/'runtime.log').write_bytes(output)
    marker = b'PASS: validated external RCC' if 'LMG_SMOKE_MOD_RCC' in env else b'PASS: six builtin QML'
    return {'passed': passed and marker in output and any(folder.glob('*.png')), 'exit': status}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--app', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--sizes', nargs='+', default=['1024x720', '640x360'])
    args = parser.parse_args()
    app = str(Path(args.app).resolve())
    root = args.output.resolve()
    root.mkdir(parents=True, exist_ok=False)
    results, total = [], 0
    for kind in ('themes', 'mods'):
        catalog = json.loads(fetch(BASE+'/api/v1/'+kind+'?limit=100', 2*1024*1024))['data']
        if isinstance(catalog, dict): catalog = catalog.get('items', catalog.get(kind, []))
        if not isinstance(catalog, list) or not 1 <= len(catalog) <= 100:
            raise ValueError('Empty or invalid catalog coverage')
        for entry in catalog:
            package_id = entry['id']
            if not re.fullmatch(r'[a-z0-9][a-z0-9_.-]{1,127}', package_id):
                raise ValueError('Unsafe catalog identity')
            folder = root/kind/package_id
            folder.mkdir(parents=True, exist_ok=True)
            record = {'kind': kind, 'id': package_id, 'version': entry['version'], 'passed': False}
            try:
                size, digest = entry['size_bytes'], entry['sha256']
                if type(size) is not int or not 0 < size <= 4*1024*1024 or not re.fullmatch('[0-9a-f]{64}', digest):
                    raise ValueError('Invalid candidate size/digest')
                total += size
                if total > 64*1024*1024: raise ValueError('Catalog exceeds the total test download budget')
                endpoint = urllib.parse.urljoin(BASE, entry.get('download_endpoint', f'/api/v1/{kind}/{package_id}/download'))
                if urllib.parse.urlparse(endpoint).netloc != urllib.parse.urlparse(BASE).netloc:
                    raise ValueError('Ticket endpoint changed catalog origin')
                ticket = json.loads(fetch(endpoint, 512*1024))['data']
                if ticket.get('sha256') != digest or ticket.get('size_bytes') != size:
                    raise ValueError('Ticket differs from catalog identity')
                data = fetch(ticket['url'], size)
                if len(data) != size or hashlib.sha256(data).hexdigest() != digest:
                    raise ValueError('Downloaded package differs from catalog digest')
                package = folder/'candidate.rcc'
                package.write_bytes(data)
                record['sha256'] = digest
                record['sizes'] = {}
                for dimensions in args.sizes:
                    if not re.fullmatch(r'[1-9][0-9]{1,3}x[1-9][0-9]{1,3}', dimensions):
                        raise ValueError('Invalid screenshot dimensions')
                    frames = folder/dimensions
                    frames.mkdir(exist_ok=True)
                    env = os.environ.copy()
                    for key in ('LMG_SMOKE_THEME_RCC', 'LMG_SMOKE_MOD_RCC', 'LMG_SMOKE_ONLY_MOD'):
                        env.pop(key, None)
                    env.update(QT_QUICK_BACKEND='software', QSG_RENDER_LOOP='basic',
                               LMG_SMOKE_OUTPUT=str(frames), LMG_SMOKE_SIZE=dimensions)
                    env['LMG_SMOKE_THEME_RCC' if kind == 'themes' else 'LMG_SMOKE_MOD_RCC'] = str(package)
                    if kind == 'mods': env['LMG_SMOKE_ONLY_MOD'] = '1'
                    record['sizes'][dimensions] = run(app, frames, env)
                record['passed'] = all(value['passed'] for value in record['sizes'].values())
            except (OSError, ValueError, KeyError, TypeError) as error:
                record['error'] = type(error).__name__ + ': ' + str(error)
            results.append(record)
            print(json.dumps(record), flush=True)
            (root/'results.json').write_text(json.dumps({'source_sha': os.environ.get('GITHUB_SHA', ''), 'results': results}, indent=2), encoding='utf-8')
    return 0 if results and all(item['passed'] for item in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
