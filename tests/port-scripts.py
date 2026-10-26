#!/usr/bin/env python3
"""Exercise release gates, metadata-only probe and isolated builds with fake SDKs.

Fake compilers test orchestration only: they are not evidence of a valid binary.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]


def run(*args, **kwargs):
    return subprocess.run(args, text=True, capture_output=True, **kwargs)


class PortScripts(unittest.TestCase):
    def test_release_gate_fails_closed(self):
        manifest = json.loads((REPO / 'compatibility/targets.json').read_text())
        for target in ('ferrari', 'chiappa'):
            self.assertEqual(manifest['targets'][target]['install_releases'], [])
            result = run('python3', str(REPO / 'tools/check-compatibility.py'), target,
                         '3.27.3.0', '--architecture', 'aarch64')
            self.assertEqual(result.returncode, 1)
            self.assertFalse(json.loads(result.stdout)['install_allowed'])
        for release, arch, expected in [('3.27.3.0', 'armv7l', 0),
                                         ('3.27.3.1', 'armv7l', 1),
                                         ('3.27.3.0', 'aarch64', 1)]:
            result = run('python3', str(REPO / 'tools/check-compatibility.py'), 'rm2',
                         release, '--architecture', arch)
            self.assertEqual(result.returncode, expected, result.stderr)

    def test_unvalidated_install_stops_before_mutations(self):
        with tempfile.TemporaryDirectory() as tmp:
            for command in ('id', 'uname', 'cp', 'mkdir', 'chmod', 'mv'):
                executable = Path(tmp, command)
                executable.write_text('#!/bin/sh\necho MUTATION-OR-PROBE >&2\nexit 88\n')
                executable.chmod(0o755)
            for target in ('ferrari', 'chiappa', 'unknown'):
                result = run('/bin/sh', str(REPO / 'install.sh'), target,
                             env={**os.environ, 'PATH': tmp})
                self.assertNotEqual(result.returncode, 0)
                self.assertNotIn('MUTATION-OR-PROBE', result.stderr)

    def test_install_target_artifact_validation_precedes_mutation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            shutil.copy(REPO / 'install.sh', root/'install.sh')
            commands = root/'commands'
            commands.mkdir()
            for name, body in [('id', 'echo 0'), ('uname', 'echo armv7l'), ('grep', 'exit 0')]:
                path = commands/name
                path.write_text('#!/bin/sh\n' + body + '\n')
                path.chmod(0o755)
            artifacts = root/'bin/rm2'
            artifacts.mkdir(parents=True)
            env = {**os.environ, 'PATH': str(commands)+':'+os.environ['PATH']}
            def attempt(expected):
                result = run('sh', str(root/'install.sh'), env=env)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(expected, result.stdout + result.stderr)
            attempt('Missing configuration helper')
            for name in ('config-model', 'ink-diary.so'):
                (artifacts/name).write_bytes(b'not ELF')
            attempt('Not a little-endian ARM32 ELF')
            header = bytearray(64)
            header[:6] = bytes.fromhex('7f454c460101')
            header[18:20] = bytes.fromhex('2800')
            for name in ('config-model', 'ink-diary.so'):
                (artifacts/name).write_bytes(header)
            attempt('missing build metadata')
            (artifacts/'build-metadata.json').write_text('{}')
            manifest = artifacts/'build-manifest.tsv'
            manifest.write_text('product\trm2\nsoftware_release\t3.28.0.172\n')
            attempt('does not match rM2')
            manifest.write_text('product\trm2\nsoftware_release\t3.27.3.0\nink-diary.so.sha256\t' + '0'*64 + '\n')
            attempt('Artifact checksum mismatch')

    def test_probe_reports_metadata_and_never_content(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            files = {
                'sys/devices/soc0/machine': 'reMarkable Chiappa\n',
                'usr/share/remarkable/update.conf': 'REMARKABLE_RELEASE_VERSION=3.99.0.1\nSECRET=DO_NOT_REPORT\n',
                'etc/os-release': 'VERSION_ID="5.7.999"\nIMG_VERSION="3.28.0.172"\nSECRET=DO_NOT_REPORT\n',
                'usr/lib/libQt6Core.so.6.5.9': 'binary',
                'usr/bin/xochitl': 'public binary',
                'home/root/.config/ink-diary/config.json': 'DO_NOT_REPORT',
                'home/root/.local/share/remarkable/xochitl/notebook': 'DO_NOT_REPORT',
            }
            for name, contents in files.items():
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(contents)
            before = {name: hashlib.sha256((root/name).read_bytes()).hexdigest() for name in files}
            result = run('sh', str(REPO / 'tools/probe-device.sh'), '--root', tmp,
                         '--architecture', 'aarch64')
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('detected_target\tchiappa', result.stdout)
            self.assertIn('software_release\t3.99.0.1', result.stdout)
            self.assertIn('qt_core_library\tlibQt6Core.so.6.5.9', result.stdout)
            self.assertIn('runtime_insertion_verified\tno', result.stdout)
            self.assertNotIn('DO_NOT_REPORT', result.stdout)
            after = {name: hashlib.sha256((root/name).read_bytes()).hexdigest() for name in files}
            self.assertEqual(before, after)
            (root/'usr/share/remarkable/update.conf').unlink()
            fallback = run('sh', str(REPO / 'tools/probe-device.sh'), '--root', tmp,
                           '--architecture', 'aarch64')
            self.assertEqual(fallback.returncode, 0, fallback.stderr)
            self.assertIn('software_release\t3.28.0.172', fallback.stdout)

    def test_build_isolation_and_sdk_validation(self):
        with tempfile.TemporaryDirectory(prefix='port-build-') as tmp:
            root = Path(tmp)
            shutil.copy(REPO / 'build.sh', root / 'build.sh')
            (root/'src').mkdir()
            (root/'src/config-model.cpp').write_text('// fake input\n')
            (root/'ink-diary.xovi').write_text('version 0.1.0\n')
            (root/'xovi.cpp').write_text('tracked-source-sentinel')
            commands = root/'commands'
            commands.mkdir()
            # Fixed ABI fixtures, not runnable programs. They model the actual
            # distinction we care about: qmake can emit host ELF despite the
            # SDK CXX reporting the correct target triplet.
            fixtures = root/'elf-fixtures'
            fixtures.mkdir()
            for product, prefix in {
                'rm2': '7f454c4601010100000000000000000003002800',
                'ferrari': '7f454c460201010000000000000000000300b700',
                'chiappa': '7f454c460201010000000000000000000300b700',
                'host': '7f454c4602010100000000000000000003003e00',
            }.items():
                (fixtures/(product + '.elf')).write_bytes(bytes.fromhex(prefix) + bytes(44))
            scripts = {
                'compiler': '''#!/bin/sh
if [ "$1" = -dumpmachine ]; then echo "$FAKE_ARCH-linux"; exit; fi
while [ "$#" -gt 0 ]; do
 if [ "$1" = -o ]; then shift; cp "$FAKE_ELF_FIXTURES/${HELPER_FIXTURE_PRODUCT:-$SDK_PRODUCT}.elf" "$1"; exit; fi
 shift
done
exit 1
''',
                'strip': '#!/bin/sh\nexit 0\n',
                'qmake': '#!/bin/sh\ntest -s ink-diary.pro\n',
                'make': '#!/bin/sh\ncp "$FAKE_ELF_FIXTURES/${QMAKE_FIXTURE_PRODUCT:-$SDK_PRODUCT}.elf" ink-diary.so\n',
                'pkg-config': '#!/bin/sh\nexit 0\n',
            }
            for name, script in scripts.items():
                path = commands/name
                path.write_text(script)
                path.chmod(0o755)
            xovi = root/'xovi'
            (xovi/'util').mkdir(parents=True)
            (xovi/'util/xovigen.py').write_text('''import pathlib,sys
args=sys.argv
for opt in ('-o','-H'):
 pathlib.Path(args[args.index(opt)+1]).write_text(args[args.index('-a')+1])
''')
            sdk = root/'sdk-env'
            sdk.write_text('export CXX=compiler\nexport STRIP=strip\n')
            env = {**os.environ, 'PATH': str(commands)+':'+os.environ['PATH'],
                   'SDK_ENV': str(sdk), 'SDK_RELEASE': '3.27.3.0', 'XOVI_REPO': str(xovi),
                   'FAKE_ELF_FIXTURES': str(fixtures)}
            for target, arch in [('rm2', 'armv7'), ('ferrari', 'aarch64'), ('chiappa', 'aarch64')]:
                result = run('bash', str(root/'build.sh'), target,
                             env={**env, 'SDK_PRODUCT': target, 'FAKE_ARCH': arch})
                self.assertEqual(result.returncode, 0, result.stderr)
                metadata = json.loads((root/'bin'/target/'build-metadata.json').read_text())
                self.assertEqual(metadata['product'], target)
                self.assertFalse(metadata['runtime_validated'])
                self.assertEqual((root/'build'/target/'xovi.h').read_text(), arch)
                for artifact, checksum in metadata['artifacts'].items():
                    self.assertEqual(hashlib.sha256((root/'bin'/target/artifact).read_bytes()).hexdigest(), checksum)
            self.assertEqual((root/'xovi.cpp').read_text(), 'tracked-source-sentinel')
            for override, artifact in [('QMAKE_FIXTURE_PRODUCT', 'ink-diary.so'),
                                       ('HELPER_FIXTURE_PRODUCT', 'config-model')]:
                # Wrong real output with a correct SDK triplet must fail before
                # publishing successful-build metadata. Use a clean output dir.
                shutil.rmtree(root/'bin/chiappa')
                result = run('bash', str(root/'build.sh'), 'chiappa',
                             env={**env, 'SDK_PRODUCT': 'chiappa', 'FAKE_ARCH': 'aarch64',
                                  override: 'host'})
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('Output is not the expected chiappa ELF: ' + artifact, result.stderr)
                self.assertNotIn('SDK compiler architecture does not match', result.stderr)
                self.assertFalse((root/'bin/chiappa/build-metadata.json').exists())
                self.assertFalse((root/'bin/chiappa/build-manifest.tsv').exists())
            for product, arch in [('ferrari', 'armv7'), ('rm2', 'aarch64')]:
                result = run('bash', str(root/'build.sh'), 'ferrari',
                             env={**env, 'SDK_PRODUCT': product, 'FAKE_ARCH': arch})
                self.assertNotEqual(result.returncode, 0)


if __name__ == '__main__':
    unittest.main()
