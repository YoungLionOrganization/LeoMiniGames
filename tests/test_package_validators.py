import importlib.util
from pathlib import Path
import struct
import hashlib
import json
import zipfile
import tempfile
import unittest
import io
import tarfile

ROOT = Path(__file__).resolve().parents[1]
def module(name, path):
    spec=importlib.util.spec_from_file_location(name, ROOT/path)
    obj=importlib.util.module_from_spec(spec); spec.loader.exec_module(obj); return obj
android = module('android', 'tools/validate_android_package.py')
assets = module('assets', 'tools/release/validate_release_assets.py')
windows = module('windows', 'tools/validate_windows_runtime.py')
qa = module('qa', 'tools/validate_platform_qa.py')
repositories = module('repositories', 'tools/release/validate_update_repositories.py')


def elf(abi, name, needed=()):
    bits=2 if abi in ('arm64-v8a', 'x86_64') else 1
    data=bytearray(2048); data[:7]=b'\x7fELF'+bytes([bits,1,1])
    struct.pack_into('<H',data,18,android.MACHINES[abi])
    if bits==2:
        struct.pack_into('<Q',data,32,64); struct.pack_into('<HH',data,54,56,2)
        struct.pack_into('<IIQQQQQQ',data,64,1,5,0,0,0,len(data),len(data),16384)
    else:
        struct.pack_into('<I',data,28,64);struct.pack_into('<HH',data,42,32,2)
        struct.pack_into('<IIIIIIII',data,64,1,0,0,0,len(data),len(data),5,16384)
    strings=bytearray(b'\0'); entries=[]
    for tag, value in [(14,name)]+[(1,dep) for dep in needed]:
        entries.append((tag,len(strings)));strings+=value.encode()+b'\0'
    entries += [(5,512),(10,len(strings)),(0,0)]
    fmt='<qQ' if bits==2 else '<iI';step=16 if bits==2 else 8
    for n,entry in enumerate(entries):struct.pack_into(fmt,data,256+n*step,*entry)
    data[512:512+len(strings)]=strings
    if bits==2:struct.pack_into('<IIQQQQQQ',data,120,2,4,256,256,256,len(entries)*step,len(entries)*step,8)
    else:struct.pack_into('<IIIIIIII',data,96,2,256,256,256,len(entries)*step,len(entries)*step,4,4)
    return bytes(data)


def pe(needed=(), machine=0x8664):
    data=bytearray(2048);data[:2]=b'MZ';struct.pack_into('<I',data,0x3c,128);data[128:132]=b'PE\0\0'
    struct.pack_into('<HH',data,132,machine,1);struct.pack_into('<H',data,148,240);struct.pack_into('<H',data,152,0x20b)
    section=392;struct.pack_into('<IIII',data,section+8,1024,4096,1024,512)
    if needed:
        struct.pack_into('<II',data,272,4096,(len(needed)+1)*20)
        position=768
        for n,name in enumerate(needed):
            struct.pack_into('<IIIII',data,512+n*20,0,0,0,4096+position-512,0)
            value=name.encode()+b'\0';data[position:position+len(value)]=value;position+=len(value)
    return bytes(data)


class PackageValidators(unittest.TestCase):
    def test_native_linux_tar_uses_system_qt_and_checks_isa_and_license(self):
        def archive(binary, license=True, escape=False):
            output=io.BytesIO()
            with tarfile.open(fileobj=output,mode='w:gz') as tar:
                for name,data in [('LeoMiniGames',binary)]+([('LICENSE',b'license')] if license else []):
                    info=tarfile.TarInfo(('../' if escape else '')+'native/'+name)
                    info.size=len(data);info.mode=0o755;tar.addfile(info,io.BytesIO(data))
            output.seek(0)
            return tarfile.open(fileobj=output,mode='r:gz')
        binary=elf('x86_64','LeoMiniGames',['libQt6Multimedia.so.6'])
        assets.validate_linux_tar(archive(binary),'Linux-x86_64-native.tar.gz')
        for tar,name in [(archive(binary,license=False),'Linux-x86_64-native.tar.gz'),
                         (archive(binary),'Linux-arm64-native.tar.gz'),
                         (archive(binary,escape=True),'Linux-x86_64-native.tar.gz'),
                         (archive(elf('x86_64','LeoMiniGames')),'Linux-x86_64-native.tar.gz')]:
            with self.assertRaises(ValueError):assets.validate_linux_tar(tar,name)

    def libs(self, abi, crypto='libcrypto_3.so'):
        return {name:elf(abi,name,needed) for name,needed in (
            ('libcrypto_3.so',('libc.so',)),('libssl_3.so',(crypto,'libdl.so')),
            ('libplugins_tls_qopensslbackend.so',('libssl_3.so',)),
            ('libplugins_multimedia_ffmpegmediaplugin.so',()))}
    def test_valid_32_and_64_bit_dependency_closure(self):
        for abi in android.MACHINES:android.check_groups({abi:self.libs(abi)}, [abi], True)
    def test_desktop_media_backend_payload(self):
        for extension in ('dll', 'so', 'dylib'):
            assets.require_audio_backend(['app/plugins/multimedia/libffmpegmediaplugin.'+extension])
        with self.assertRaisesRegex(ValueError, 'Multimedia'):
            assets.require_audio_backend(['app/Qt6Multimedia.dll','app/qml/QtMultimedia/quickplugin.dll'])
    def test_missing_media_backend_is_rejected(self):
        libs=self.libs('arm64-v8a');del libs['libplugins_multimedia_ffmpegmediaplugin.so']
        with self.assertRaisesRegex(ValueError, 'Multimedia'):
            android.check_groups({'arm64-v8a':libs}, ['arm64-v8a'], True)
    def test_published_openssl_name_is_rejected(self):
        for abi in android.MACHINES:
            with self.assertRaises(ValueError):android.check_groups({abi:self.libs(abi,'libcrypto.so')},[abi],True)
    def test_missing_packaged_runtime_is_rejected(self):
        libs=self.libs('arm64-v8a');libs['libgame.so']=elf('arm64-v8a','libgame.so',['libc++_shared.so'])
        with self.assertRaises(ValueError):android.check_groups({'arm64-v8a':libs}, ['arm64-v8a'], True)
    def test_wrong_abi_and_alignment_are_rejected(self):
        with self.assertRaises(ValueError):android.validate_elf(elf('x86','test'), 'armeabi-v7a', 'test')
        data=bytearray(elf('arm64-v8a','test'));struct.pack_into('<Q',data,112,4096)
        with self.assertRaises(ValueError):android.validate_elf(data,'arm64-v8a','test')
    def test_empty_and_corrupt_artifacts_are_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            for content in (b'',b'not a zip'):
                path=Path(folder)/'sample.zip';path.write_bytes(content)
                with self.assertRaises(Exception):assets.validate_content(path,ROOT)
    def test_windows_missing_runtime_and_wrong_architecture(self):
        with tempfile.TemporaryDirectory() as directory:
            stage=Path(directory);(stage/'LeoMiniGames.exe').write_bytes(pe(['VCRUNTIME140.dll','MSVCP140.dll']))
            with self.assertRaises(ValueError):windows.validate(stage)
            for name in ('vcruntime140.dll','msvcp140.dll'):(stage/name).write_bytes(pe())
            self.assertEqual(windows.validate(stage), {'vcruntime140.dll','msvcp140.dll'})
            (stage/'msvcp140.dll').write_bytes(pe(machine=0xaa64))
            with self.assertRaises(ValueError):windows.validate(stage)
    def test_real_manifest_requires_extracted_native_plugins(self):
        texts=('application','extractNativeLibs')
        strings=b'';offsets=[]
        for value in texts:
            offsets.append(len(strings));encoded=value.encode();strings+=bytes([len(value),len(encoded)])+encoded+b'\x00'
        strings+=b'\x00'*((-len(strings))%4)
        pool=struct.pack('<HHIIIIII',1,28,36+len(strings),2,0,256,36,0)+struct.pack('<II',*offsets)+strings
        node=bytearray(56);struct.pack_into('<HHI',node,0,0x102,16,56);struct.pack_into('<II',node,16,0xffffffff,0)
        struct.pack_into('<HHH',node,24,20,20,1);struct.pack_into('<IIIHBBI',node,36,0xffffffff,1,0xffffffff,8,0,18,1)
        manifest=struct.pack('<HHI',3,8,8+len(pool)+len(node))+pool+node
        android.require_extracted_native_libraries(manifest)
        struct.pack_into('<I',node,52,0)
        disabled=struct.pack('<HHI',3,8,len(manifest))+pool+node
        with self.assertRaises(ValueError):android.require_extracted_native_libraries(disabled)
        with self.assertRaises(ValueError):android.require_extracted_native_libraries(b'not a manifest')

    def test_mixed_arm64_crt_copies_only_native_dlls_and_requires_closure(self):
        with tempfile.TemporaryDirectory() as directory:
            stage=Path(directory)/'stage';stage.mkdir()
            crt=Path(directory)/'crt';crt.mkdir()
            (stage/'LeoMiniGames.exe').write_bytes(pe(['vcruntime140.dll','msvcp140.dll'],machine=0xaa64))
            for name in ('vcruntime140.dll','msvcp140.dll'):
                (crt/name).write_bytes(pe(machine=0xaa64))
            (crt/'vcruntime140_1.dll').write_bytes(pe(machine=0x8664))
            windows.deploy_runtime(stage,crt)
            self.assertFalse((stage/'vcruntime140_1.dll').exists())
            (stage/'LeoMiniGames.exe').write_bytes(pe(['vcruntime140_1.dll'],machine=0xaa64))
            with self.assertRaisesRegex(ValueError,'missing app-local runtime'):
                windows.deploy_runtime(stage,crt)

    def test_apk_certificate_identity_and_unsigned_rejection(self):
        def lp(data): return struct.pack('<I',len(data))+data
        certificate=b'regression certificate bytes'
        signer=lp(lp(b'')+lp(lp(certificate))+lp(b''))+lp(b'')+lp(b'')
        value=lp(lp(signer))
        pair=struct.pack('<QI',len(value)+4,0x7109871a)+value
        length=len(pair)+24
        block=struct.pack('<Q',length)+pair+struct.pack('<Q',length)+b'APK Sig Block 42'
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'unsigned.apk'
            with zipfile.ZipFile(path,'w') as archive: archive.writestr('fixture',b'content')
            with self.assertRaises(ValueError):android.signing_certificate_sha256(path)
            original=path.read_bytes();end=original.rfind(b'PK\x05\x06')
            central=struct.unpack_from('<I',original,end+16)[0]
            signed=bytearray(original[:central]+block+original[central:])
            struct.pack_into('<I',signed,end+len(block)+16,central+len(block));path.write_bytes(signed)
            self.assertEqual(android.signing_certificate_sha256(path),hashlib.sha256(certificate).hexdigest())
            signed[central+len(block)-1]=0;path.write_bytes(signed)
            with self.assertRaises(ValueError):android.signing_certificate_sha256(path)

    def test_qa_matches_every_candidate_artifact(self):
        sha='a'*40;digest='b'*64
        report={'version':'0.7.3','source_sha':sha,'targets':{}}
        for name in qa.TARGETS:
            report['targets'][name]={'passed':True,'clean_system':True,'environment':'test fixture',
                'artifact_sha256':digest,'artifact_name':'fixture.zip','evidence':'fixture only',
                'tested_at':'2026-10-01','tester':'test', 'device_abis':['x86_64'],
                'installer_upgrade_modify_uninstall':True,'gatekeeper_assessed':True}
        manifest={'source_sha':sha,'assets':{}}
        for target,row in report['targets'].items():
            candidates=qa.target_artifacts(target)
            if candidates:
                row['artifact_name']=sorted(candidates)[0]
                row['tested_artifacts']={name:digest for name in candidates}
                for name in candidates:manifest['assets'][name]={'sha256':digest}
        qa.validate(report,sha,manifest)
        next(iter(manifest['assets'].values()))['sha256']='c'*64
        with self.assertRaises(ValueError):qa.validate(report,sha,manifest)
        manifest['source_sha']='d'*40
        with self.assertRaises(ValueError):qa.validate(report,sha,manifest)

    def test_update_repository_rejects_stale_version_and_changed_payload(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            for arch in repositories.ARCHES:
                repo=root/'windows'/arch;component=repo/'xyz.younglion.leominigames';component.mkdir(parents=True)
                (repo/'Updates.xml').write_text('<Updates><PackageUpdate><Name>xyz.younglion.leominigames</Name><Version>0.7.3</Version><UpdateFile CompressedSize="7" UncompressedSize="7"/><DownloadableArchives>content.7z</DownloadableArchives></PackageUpdate></Updates>')
                payload=component/'0.7.3content.7z';payload.write_bytes(b'fixture')
                Path(str(payload)+'.sha1').write_text(hashlib.sha1(b'fixture').hexdigest())
            repositories.validate(root,'0.7.3')
            with self.assertRaises(ValueError):repositories.validate(root,'0.7.2')
            payload.write_bytes(b'changed')
            with self.assertRaises(ValueError):repositories.validate(root,'0.7.3')

    def test_qa_cannot_pass_pending_or_wrong_commit(self):
        report={'version':'0.7.3','source_sha':'a','targets':{name:{'passed':False} for name in qa.TARGETS}}
        with self.assertRaises(ValueError):qa.validate(report,'a')
        with self.assertRaises(ValueError):qa.validate(report,'b')

if __name__=='__main__':unittest.main()
