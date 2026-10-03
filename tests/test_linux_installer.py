#!/usr/bin/env python3
"""Installer lifecycle tests, optionally using a real packaged AppDir."""
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class LinuxInstallerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.home = self.root/'home'; self.home.mkdir()
        self.prefix = self.home/'app with spaces'
        self.env = dict(os.environ, HOME=str(self.home),
                        XDG_DATA_HOME=str(self.home/'data'),
                        XDG_CONFIG_HOME=str(self.home/'config'),
                        XDG_CACHE_HOME=str(self.home/'cache'),
                        XDG_BIN_HOME=str(self.home/'bin'))
        self.setup=self.root/'Setup.run'
        if os.environ.get('LMG_TEST_SETUP'):
            shutil.copyfile(os.environ['LMG_TEST_SETUP'],self.setup)
            return
        appdir = self.root/'LeoMiniGames.AppDir'
        if os.environ.get('LMG_TEST_APPDIR'):
            shutil.copytree(os.environ['LMG_TEST_APPDIR'], appdir, symlinks=True)
        else:
            appdir.mkdir()
            (appdir/'AppRun').write_text('#!/usr/bin/env bash\nprintf "fixture ran: %s\\n" "$*"\n')
            (appdir/'AppRun').chmod(0o755)
        (appdir/'old-file').write_text('old payload')
        payload=self.root/'payload.tar.gz'
        with tarfile.open(payload,'w:gz') as archive:
            archive.add(appdir,arcname=appdir.name)
        self.setup.write_bytes((ROOT/'installer/linux/install.sh').read_bytes()+payload.read_bytes())

    def run_setup(self, *args, success=True):
        result=subprocess.run(['bash',str(self.setup),'--prefix',str(self.prefix),*args],
                              env=self.env,capture_output=True,text=True,timeout=40)
        if success: self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        else: self.assertNotEqual(result.returncode,0)
        return result

    def test_install_upgrade_launch_uninstall_preserves_user_data(self):
        self.run_setup()
        launcher=self.home/'bin/leominigames'
        desktop=self.home/'data/applications/xyz.younglion.leominigames.desktop'
        self.assertTrue(launcher.is_file());self.assertTrue(desktop.is_file())
        save=self.home/'data/YoungLion/LeoMiniGames/save-marker'
        save.parent.mkdir(parents=True);save.write_text('keep my save')
        # A file absent from the replacement must disappear during upgrade.
        (self.prefix/'stale-local-file').write_text('obsolete')
        self.run_setup()
        self.assertFalse((self.prefix/'stale-local-file').exists())
        env=dict(self.env,QT_QPA_PLATFORM=os.environ.get('LMG_TEST_QPA_PLATFORM','offscreen'),QT_QUICK_BACKEND='software')
        for key in ('LD_LIBRARY_PATH','QT_PLUGIN_PATH','QT_QPA_PLATFORM_PLUGIN_PATH','QML2_IMPORT_PATH','QML_IMPORT_PATH'):
            env.pop(key,None)
        result=subprocess.run([str(launcher),'--smoke-test'],env=env,
                              capture_output=True,text=True,timeout=35)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        if os.environ.get('LMG_TEST_SETUP') or os.environ.get('LMG_TEST_APPDIR'):
            self.assertIn('PASS: six builtin QML sessions opened and closed',result.stdout+result.stderr)
        result=subprocess.run(['bash',str(self.prefix/'uninstall.sh'),'--prefix',str(self.prefix),'--uninstall'],
                              env=self.env,capture_output=True,text=True,timeout=10)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertFalse(self.prefix.exists());self.assertFalse(launcher.exists());self.assertFalse(desktop.exists())
        self.assertEqual(save.read_text(),'keep my save')

    def test_no_integration_and_reject_unmanaged_directory_or_symlink(self):
        self.prefix.mkdir();(self.prefix/'important').write_text('keep')
        self.run_setup(success=False)
        self.assertEqual((self.prefix/'important').read_text(),'keep')
        shutil.rmtree(self.prefix)
        self.prefix.symlink_to(self.home,target_is_directory=True)
        self.run_setup(success=False)
        self.prefix.unlink()
        self.run_setup('--no-integration')
        self.assertFalse((self.home/'bin').exists())
        self.run_setup('--uninstall')

    def test_corrupt_payload_does_not_replace_existing_install(self):
        self.run_setup('--no-integration')
        self.setup.write_bytes((ROOT/'installer/linux/install.sh').read_bytes()+b'bad gzip')
        self.run_setup('--no-integration',success=False)
        self.assertTrue((self.prefix/'AppRun').is_file())

    def test_foreign_launcher_keeps_existing_payload(self):
        self.run_setup('--no-integration')
        (self.prefix/'preserve-on-failure').write_text('keep')
        launcher=self.home/'bin/leominigames';launcher.parent.mkdir()
        launcher.write_text('unrelated launcher')
        self.run_setup(success=False)
        self.assertEqual(launcher.read_text(),'unrelated launcher')
        self.assertTrue((self.prefix/'preserve-on-failure').is_file())

    def test_uninstall_keeps_launcher_reassigned_to_similar_prefix(self):
        self.run_setup()
        launcher=self.home/'bin/leominigames'
        launcher.write_text(f'#!/usr/bin/env bash\n# LeoMiniGames prefix: {self.prefix}-other\n')
        self.run_setup('--uninstall')
        self.assertTrue(launcher.is_file())


if __name__=='__main__': unittest.main()
