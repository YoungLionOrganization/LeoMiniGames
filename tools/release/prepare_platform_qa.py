#!/usr/bin/env python3
# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
"""Prepare candidate hashes without claiming any platform tests passed."""
from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
from validate_platform_qa import TARGETS, target_artifacts


def prepare(manifest: dict, template: dict, policy: dict, build_run: str) -> dict:
    if not isinstance(manifest, dict):
        raise ValueError('Candidate manifest must be a JSON object')
    version = template['version']
    if manifest.get('version') != version or policy.get('version') != version:
        raise ValueError('Candidate, QA template and asset policy versions differ')
    if not re.fullmatch(r'[0-9a-f]{40}', str(manifest.get('source_sha', ''))):
        raise ValueError('Candidate requires a full source SHA')
    if not re.fullmatch(r'[1-9][0-9]*', build_run):
        raise ValueError('Build run must be a numeric GitHub Actions run ID')
    expected = {name.format(version=version) for name in policy['expected_names']}
    assets = manifest.get('assets', {})
    if not isinstance(assets, dict):
        raise ValueError('Candidate assets must be a JSON object')
    if set(assets) != expected:
        raise ValueError('Candidate manifest does not match the exact public asset set')
    for name, record in assets.items():
        if not isinstance(record, dict):
            raise ValueError(f'{name}: package record must be a JSON object')
        if not re.fullmatch(r'[0-9a-f]{64}', str(record.get('sha256', ''))):
            raise ValueError(f'{name}: invalid SHA-256')
        size = record.get('size')
        if type(size) is not int or size <= 0:
            raise ValueError(f'{name}: invalid package size')
    if set(template.get('targets', {})) != set(TARGETS):
        raise ValueError('QA template target coverage differs from the validator')

    report = copy.deepcopy(template)
    report['source_sha'] = manifest['source_sha']
    report['candidate_build_run'] = build_run
    report['review_note'] = (
        'Candidate hashes identify packages to test. No platform test results '
        'are supplied by this tool; all pass and clean-system flags are false.')
    for target in TARGETS:
        row = report['targets'][target]
        # Clear any stale results even if the input template was edited.
        for field, value in list(row.items()):
            if isinstance(value, bool):
                row[field] = False
        row['passed'] = row['clean_system'] = False
        for field in ('environment', 'evidence', 'tested_at', 'tester',
                      'artifact_name', 'artifact_sha256'):
            row[field] = ''
        if 'device_abis' in row:
            row['device_abis'] = []
        names = sorted(target_artifacts(target, version))
        if names:
            row['tested_artifacts'] = {name: assets[name]['sha256'] for name in names}
            row['artifact_name'] = names[0]
            row['artifact_sha256'] = assets[names[0]]['sha256']
        else:
            # Apple developer outputs are outside the public asset manifest.
            row['tested_artifacts'] = {}
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('manifest', type=Path)
    parser.add_argument('--build-run', required=True)
    parser.add_argument('--output', type=Path, required=True,
                        help='new pending JSON file; existing files are never overwritten')
    args = parser.parse_args()
    try:
        report = prepare(
            json.loads(args.manifest.read_text(encoding='utf-8')),
            json.loads((ROOT/'release/platform-qa.template.json').read_text(encoding='utf-8')),
            json.loads((ROOT/'release/assets.json').read_text(encoding='utf-8')),
            args.build_run)
        with args.output.open('x', encoding='utf-8') as stream:
            json.dump(report, stream, indent=2)
            stream.write('\n')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))
    print(f'Prepared {len(TARGETS)} pending targets; no test results marked passed: {args.output}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
