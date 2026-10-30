# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('prepare_qa', ROOT/'tools/release/prepare_platform_qa.py')
prepare_qa = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prepare_qa)
from validate_platform_qa import validate


class PrepareQaTests(unittest.TestCase):
    def setUp(self):
        self.template = json.loads((ROOT/'release/platform-qa.template.json').read_text())
        self.policy = json.loads((ROOT/'release/assets.json').read_text())
        self.manifest = {'version': self.template['version'], 'source_sha': 'a'*40,
                         'assets': {name.format(version=self.template['version']):
                                    {'size': 1, 'sha256': 'b'*64}
                                    for name in self.policy['expected_names']}}

    def test_candidate_hashes_do_not_grant_test_results(self):
        original = copy.deepcopy(self.template)
        report = prepare_qa.prepare(self.manifest, self.template, self.policy, '123')
        self.assertEqual(self.template, original)
        self.assertEqual(report['source_sha'], 'a'*40)
        self.assertEqual(len(report['targets']), 25)
        for target, row in report['targets'].items():
            self.assertFalse(row['passed'])
            self.assertFalse(row['clean_system'])
            self.assertEqual(row['evidence'], '')
            if target.startswith(('iOS-', 'iPadOS-')):
                self.assertEqual(row['artifact_sha256'], '')
            else:
                self.assertTrue(row['tested_artifacts'])
                self.assertEqual(row['artifact_sha256'], 'b'*64)
        with self.assertRaisesRegex(ValueError, 'clean launch not passed'):
            validate(report, 'a'*40, self.manifest)

    def test_stale_template_passes_and_abi_claims_are_cleared(self):
        for row in self.template['targets'].values():
            for field, value in list(row.items()):
                if isinstance(value, bool): row[field] = True
            row['tester'] = 'old tester'
            row['evidence'] = 'old evidence'
            if 'device_abis' in row: row['device_abis'] = ['invented ABI']
        report = prepare_qa.prepare(self.manifest, self.template, self.policy, '123')
        for row in report['targets'].values():
            self.assertTrue(all(value is False for value in row.values() if isinstance(value, bool)))
            self.assertEqual(row['tester'], '')
            self.assertEqual(row.get('device_abis', []), [])

    def test_invalid_candidate_never_produces_pending_report(self):
        for mutate in (
            lambda m: m.update(source_sha='short'),
            lambda m: m.update(version='0.0.0'),
            lambda m: m['assets'].pop(next(iter(m['assets']))),
            lambda m: m['assets'].update({'unexpected.zip': {'size': 1, 'sha256': 'b'*64}}),
            lambda m: next(iter(m['assets'].values())).update(sha256='invalid'),
            lambda m: next(iter(m['assets'].values())).update(size=True),
        ):
            manifest = copy.deepcopy(self.manifest)
            mutate(manifest)
            with self.assertRaises(ValueError):
                prepare_qa.prepare(manifest, self.template, self.policy, '123')


if __name__ == '__main__':
    unittest.main()
