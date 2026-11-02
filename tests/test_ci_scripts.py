# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
"""Control-flow regressions using fake tools; these do not test Android devices."""
import json
import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import textwrap
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('android_logcat', ROOT/'tools/ci/validate_android_logcat.py')
android_logcat = importlib.util.module_from_spec(spec)
spec.loader.exec_module(android_logcat)
spec = importlib.util.spec_from_file_location('audio_capture', ROOT/'tools/ci/validate_audio_capture.py')
audio_capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audio_capture)


class AudioCaptureTests(unittest.TestCase):
    def test_real_pcm_accepts_both_wave_polarities(self):
        self.assertEqual(audio_capture.validate(b'\x00\x20\x00\xe0'*100)['audible'], 200)

    def test_silence_and_single_spike_cannot_pass(self):
        for data in (b'\0\0'*1000, b'\xff\x7f'+b'\0\0'*1000):
            with self.assertRaisesRegex(ValueError, 'silent or incomplete'):
                audio_capture.validate(data)

    def test_empty_or_truncated_pcm_cannot_pass(self):
        for data in (b'', b'\0'):
            with self.assertRaises(ValueError):
                audio_capture.validate(data)


class AndroidLogcatTests(unittest.TestCase):
    def test_system_library_failure_is_not_application_crash(self):
        text = '10-02 22:34:13.872 1538 1758 D nativeloader: dlopen failed: library libgeller.so not found'
        self.assertEqual(android_logcat.application_crashes(text, ['2495']), [])

    def test_java_and_native_application_crashes_fail(self):
        for error in ('FATAL EXCEPTION: main', 'Fatal signal 11', 'UnsatisfiedLinkError', 'dlopen failed', 'CANNOT LINK EXECUTABLE'):
            text = f'10-02 22:34:13.872 2495 2495 E AndroidRuntime: {error}'
            self.assertTrue(android_logcat.application_crashes(text, ['2495']))

    def test_crash_before_process_restart_still_fails(self):
        text = '''10-02 22:34:10.015 557 591 I ActivityManager: Start proc 2495:xyz.younglion.leominigames/u0a209
10-02 22:34:13.872 2495 2495 E AndroidRuntime: FATAL EXCEPTION: main
10-02 22:34:14.015 557 591 I ActivityManager: Start proc 3000:xyz.younglion.leominigames/u0a209'''
        self.assertTrue(android_logcat.application_crashes(text, ['3000']))

    def test_native_tombstone_identifies_the_application(self):
        text = '10-02 22:34:13.872 100 100 F DEBUG: pid: 2495, tid: 2496, name: QtThread >>> xyz.younglion.leominigames <<<'
        self.assertTrue(android_logcat.application_crashes(text, ['3000']))


@unittest.skipUnless(sys.platform == 'linux', 'Exercises Linux process environment limits')
class ReleasePublisherTests(unittest.TestCase):
    def run_release_plan(self, releases):
        workflow = (ROOT/'.github/workflows/publish-release.yml').read_text(encoding='utf-8')
        step = workflow.split('      - name: Prepare draft and upload exact asset set\n', 1)[1]
        script = textwrap.dedent(step.split('        run: |\n', 1)[1].split('\n      - name:', 1)[0])
        # Run the actual preparation/plan shell, stopping before any mutation.
        script = script.split('while IFS= read -r stale_id;', 1)[0] + '\nprintf "%s" "$plan"\n'
        with tempfile.TemporaryDirectory() as folder:
            tmp = Path(folder)
            (tmp/'bin').mkdir()
            gh = tmp/'bin/gh'
            gh.write_text('''#!/usr/bin/env python3
import os, sys
from pathlib import Path
if sys.argv[1:] != ['api', 'repos/fixture/repo/releases?per_page=100']:
    raise SystemExit('Unexpected API mutation or request')
sys.stdout.buffer.write(Path(os.environ['RELEASE_FIXTURE']).read_bytes())
''', encoding='utf-8')
            gh.chmod(0o755)
            fixture = tmp/'releases.json'
            fixture.write_text(json.dumps(releases), encoding='utf-8')
            (tmp/'notes.md').write_text('Release notes', encoding='utf-8')
            (tmp/'_validation-summary.md').write_text('## Validation and device-test limits\nUntested hardware.', encoding='utf-8')
            (tmp/'_release-manifest.json').write_text(json.dumps({'source_sha': 'a'*40, 'assets': {}}), encoding='utf-8')
            env = dict(os.environ, PATH=str(tmp/'bin')+os.pathsep+os.environ['PATH'],
                       RELEASE_FIXTURE=str(fixture), GITHUB_REPOSITORY='fixture/repo',
                       NOTES='notes.md', TARGET_SHA='a'*40, TAG='v0.7.3')
            return subprocess.run(['bash', '-e', '-o', 'pipefail'], input=script, cwd=tmp,
                                  env=env, text=True, capture_output=True, timeout=15)

    def test_large_release_collection_recovers_exact_draft(self):
        # Existing repository responses exceed Linux's 128 KiB per-variable limit.
        releases = [{'id': 1, 'tag_name': 'v0.7.2', 'draft': False, 'body': 'x'*300000},
                    {'id': 2, 'tag_name': 'v0.7.3', 'draft': True, 'target_commitish': 'b'*40},
                    {'id': 3, 'tag_name': 'v0.7.3', 'draft': True, 'target_commitish': 'a'*40}]
        result = self.run_release_plan(releases)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout), {'keep': 3, 'delete': [2]})

    def test_large_collection_still_rejects_public_release(self):
        result = self.run_release_plan([{'id': 1, 'tag_name': 'v0.7.3', 'draft': False, 'body': 'x'*300000}])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('a non-draft release with this tag already exists', result.stderr)


class EmulatorWrapperTests(unittest.TestCase):
    def run_wrapper(self, abi='x86_64', failure=False, missing=False, avd_failure=False, empty_avd=False, stubborn=False):
        with tempfile.TemporaryDirectory() as folder:
            tmp = Path(folder)
            bin_dir = tmp/'bin'
            bin_dir.mkdir()
            emulator_dir = tmp/'sdk/emulator'
            emulator_dir.mkdir(parents=True)
            marker = tmp/'emulator.json'
            log = tmp/'tools.log'
            def script(path, body):
                path.write_text(body)
                path.chmod(0o755)
            script(bin_dir/'sdkmanager', '#!/usr/bin/env bash\nexit 0\n')
            script(bin_dir/'avdmanager', '''#!/usr/bin/env bash
printf "%s\\n" "$*" >> "$TEST_LOG"
[[ "$TEST_AVD_FAILURE" == 0 ]] || { echo 'fixture AVD failure' >&2; exit 7; }
[[ "$TEST_EMPTY_AVD" == 0 ]] || exit 0
mkdir -p "$ANDROID_AVD_HOME/lmg-smoke.avd"
printf 'path=fixture\\n' > "$ANDROID_AVD_HOME/lmg-smoke.ini"
printf 'abi.type=fixture\\n' > "$ANDROID_AVD_HOME/lmg-smoke.avd/config.ini"
''')
            script(bin_dir/'sleep', '#!/usr/bin/env bash\nexit 0\n')
            script(bin_dir/'sudo', '#!/usr/bin/env bash\nexit 0\n')
            script(emulator_dir/'emulator', '''#!/usr/bin/env python3
import os, json, time, sys, signal
from pathlib import Path
keys = ['LD_LIBRARY_PATH', 'QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH',
        'QML2_IMPORT_PATH', 'QML_IMPORT_PATH', 'QT_QPA_PLATFORM', 'QT_QPA_PLATFORMTHEME']
if any(key in os.environ for key in keys):
    print('contaminated Qt environment', flush=True); sys.exit(19)
if os.environ['TEST_FAILURE'] == '1':
    print('fixture emulator startup failure', flush=True); sys.exit(17)
Path(os.environ['TEST_MARKER']).write_text(json.dumps(sys.argv[1:]))
if os.environ['TEST_STUBBORN'] == '1': signal.signal(signal.SIGTERM, signal.SIG_IGN)
while True: time.sleep(0.01)
''')
            script(bin_dir/'adb', '''#!/usr/bin/env python3
import os, sys, time, signal
from pathlib import Path
args = ' '.join(sys.argv[1:])
with open(os.environ['TEST_LOG'], 'a') as f: f.write('adb '+args+'\\n')
if 'emu kill' in args and os.environ['TEST_STUBBORN'] == '1':
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    while True: time.sleep(0.1)
if 'getprop sys.boot_completed' in args:
    print('1' if Path(os.environ['TEST_MARKER']).exists() else '0')
elif 'resolve-activity' in args:
    print('xyz.younglion.leominigames/org.qtproject.qt.android.bindings.QtActivity')
elif 'pidof' in args: print('123')
elif 'logcat -d' in args:
    print('PASS: HTTPS catalog probe; certificate validated; HTTP 200')
''')
            apk = tmp/'fixture.apk'
            if not missing: apk.write_bytes(b'control flow fixture only')
            env = dict(os.environ, PATH=str(bin_dir)+os.pathsep+os.environ['PATH'],
                       ANDROID_SDK_ROOT=str(tmp/'sdk'), TEST_MARKER=str(marker),
                       TEST_LOG=str(log), TEST_FAILURE='1' if failure else '0',
                       TEST_STUBBORN='1' if stubborn else '0',
                       LMG_AVDMANAGER=str(bin_dir/'avdmanager'),
                       TEST_AVD_FAILURE='1' if avd_failure else '0',
                       TEST_EMPTY_AVD='1' if empty_avd else '0')
            for key in ('LD_LIBRARY_PATH','QT_PLUGIN_PATH','QT_QPA_PLATFORM_PLUGIN_PATH',
                        'QML2_IMPORT_PATH','QML_IMPORT_PATH','QT_QPA_PLATFORM','QT_QPA_PLATFORMTHEME'):
                env[key] = '/fixture/android/qt'
            result = subprocess.run(['bash', str(ROOT/'tools/ci/run_android_emulator_smoke.sh'),
                                     str(apk), abi, str(tmp/'out')], env=env,
                                    capture_output=True, text=True, timeout=12)
            return result, log.read_text() if log.exists() else '', json.loads(marker.read_text()) if marker.exists() else []

    def test_boot_and_tls_probe_for_supported_abis(self):
        for abi in ('x86', 'x86_64', 'universal'):
            with self.subTest(abi=abi):
                result, log, args = self.run_wrapper(abi)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn('certificate-validated HTTPS', result.stdout)
                self.assertIn('--device pixel_2', log)
                self.assertIn('--ez xyz.younglion.leominigames.tlsSmokeTest true', log)
                self.assertIn('5554', args)
                self.assertIn('-no-snapshot', args)

    def test_dead_emulator_reports_startup_error_without_boot_timeout(self):
        result, _, _ = self.run_wrapper(failure=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('exited before boot', result.stderr)
        self.assertIn('fixture emulator startup failure', result.stderr)

    def test_hung_adb_shutdown_and_sigterm_ignored_emulator_are_bounded(self):
        result, log, args = self.run_wrapper(stubborn=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertIn('certificate-validated HTTPS',result.stdout)
        self.assertIn('emu kill',log)
        self.assertIn('5554',args)

    def test_missing_apk_fails_before_sdk_install(self):
        result, log, _ = self.run_wrapper(missing=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('APK not found', result.stderr)
        self.assertEqual(log, '')

    def test_failed_or_empty_avd_never_starts_emulator(self):
        for flags in ({'avd_failure': True}, {'empty_avd': True}):
            result, log, args = self.run_wrapper(**flags)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(args, [])
            self.assertNotIn('adb ', log)
            self.assertTrue('fixture AVD failure' in result.stderr or 'usable configuration' in result.stderr)


if __name__ == '__main__':
    unittest.main()
