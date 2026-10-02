#!/usr/bin/env python3
"""SDK authoring failures should be actionable and preserve existing packages."""
from pathlib import Path
import importlib.util
import json
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('sdk_builder',ROOT/'tools/sdk/build_package.py')
builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)

class SdkTools(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.project=Path(self.temp.name)
        self.manifest={'id':'sdk_test','name':'SDK test','version':'1.0.0','entry':'Main.qml','api_version':'0.7'}
        (self.project/'Main.qml').write_text('import QtQuick\nItem {}\n')
        (self.project/'mod.qrc').write_text('<RCC><qresource prefix="/"><file>Main.qml</file><file>manifest.json</file></qresource></RCC>')
        self.write()
    def write(self):(self.project/'manifest.json').write_text(json.dumps(self.manifest))
    def test_real_examples_are_source_valid(self):
        for folder in ('mod-sdk/ExampleHelloMod','mod-sdk/ExampleModernMod','theme-sdk/ExampleTheme'):builder.validate(ROOT/folder)
    def test_missing_declared_resource_is_rejected(self):
        self.manifest['icon_path']='assets/icon.png';self.write()
        with self.assertRaisesRegex(ValueError,'missing from RCC'):builder.validate(self.project)
    def test_missing_declared_locale_is_rejected(self):
        self.manifest['locales']=['en'];self.write()
        with self.assertRaisesRegex(ValueError,'i18n/en'):builder.validate(self.project)
    def test_duplicate_rcc_alias_is_rejected(self):
        p=self.project/'mod.qrc';p.write_text(p.read_text().replace('</qresource>','<file alias="Main.qml">Main.qml</file></qresource>'))
        with self.assertRaisesRegex(ValueError,'Duplicate'):builder.validate(self.project)
    def test_unsafe_entry_and_id_are_rejected(self):
        for key,value in [('id','../bad'),('entry','../Main.qml'),('entry','a/../Main.qml')]:
            old=self.manifest[key];self.manifest[key]=value;self.write()
            with self.assertRaises(ValueError):builder.validate(self.project)
            self.manifest[key]=old
    def test_bad_schema_and_capability_types_are_rejected(self):
        for key,value in [('save_version',True),('save_version',0),('required_capabilities','theme'),('required_capabilities',['theme',4])]:
            self.manifest[key]=value;self.write()
            with self.assertRaises(ValueError):builder.validate(self.project)
            self.manifest.pop(key)
    def test_private_qml_import_is_rejected(self):
        (self.project/'Main.qml').write_text('import LeoMiniGames\nItem {}\n')
        with self.assertRaisesRegex(ValueError,'private'):builder.validate(self.project)
    def test_failed_compile_preserves_existing_output(self):
        output=self.project/'old.rcc';output.write_bytes(b'existing package')
        def run(args,**kwargs):
            if '--version' in args:return subprocess.CompletedProcess(args,0,'rcc 6.10.2','')
            raise subprocess.CalledProcessError(1,args)
        with patch.object(builder.subprocess,'run',side_effect=run):
            with self.assertRaises(subprocess.CalledProcessError):builder.build(self.project,output,'rcc')
        self.assertEqual(output.read_bytes(),b'existing package')
    def test_qt5_compiler_is_rejected(self):
        with patch.object(builder.subprocess,'run',return_value=subprocess.CompletedProcess([],0,'rcc 5.15.2','')):
            with self.assertRaisesRegex(ValueError,'Qt 6'):builder.build(self.project,self.project/'out.rcc','rcc')
    def test_windows_wrappers_have_real_newlines(self):
        for folder in ('mod-sdk/ExampleHelloMod','mod-sdk/ExampleModernMod','theme-sdk/ExampleTheme'):
            for path in (ROOT/folder).glob('*.bat'):
                self.assertNotIn(bytes([92,114,92,110]),path.read_bytes());self.assertGreaterEqual(path.read_bytes().count(bytes([10])),6)

if __name__=='__main__':unittest.main()
