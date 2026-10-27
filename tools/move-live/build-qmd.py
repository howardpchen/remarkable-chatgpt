#!/usr/bin/env python3
"""Prepare and locally apply the exact-release Move widget patch; never deploy."""
import argparse
from pathlib import Path
import shutil
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--qmldiff', required=True, type=Path)
parser.add_argument('--hashtab', required=True, type=Path)
parser.add_argument('--qml-root', required=True, type=Path)
parser.add_argument('--output', required=True, type=Path, help='New private directory; must not exist')
args = parser.parse_args()
repo = Path(__file__).resolve().parents[2]
qml = '\n'.join(line for line in (repo / 'tools/move-live/MoveLive.qml').read_text().splitlines()
                if not line.startswith('import '))
raw = ('VERSION 3.28.0.172\nAFFECT /qml/device/view/documentview/DocumentView.qml\n'
       ' IMPORT net.asivery.XoviMessageBroker 2.0\n TRAVERSE FocusScope#root\n'
       ' LOCATE AFTER ALL\n INSERT {\n'
       + (repo / 'tools/move-demo/NativePoseBindings.qml.inc').read_text()
       + '\n' + qml + '\n }\n END TRAVERSE\nEND AFFECT\n')
args.output.mkdir(mode=0o700, parents=True, exist_ok=False)
raw_path = args.output / 'live-raw.qmd'
hashed = args.output / 'live.qmd'
raw_path.write_text(raw)
shutil.copyfile(raw_path, hashed)
# Use the firmware's actual table; parser errors fail here before activation.
subprocess.run([str(args.qmldiff), 'hash-diffs', str(args.hashtab), str(hashed)], check=True)
subprocess.run([str(args.qmldiff), 'apply-diffs', '--version', '3.28.0.172', '--hashtab',
                str(args.hashtab), str(args.qml_root), str(args.output / 'patched'), str(hashed)], check=True)
patched = args.output / 'patched/qml/device/view/documentview/DocumentView.qml'
text = patched.read_text()
if text.count('id: moveUiProbe\n') != 1 or text.count('diaryNativePoseObjects:') != 1:
    raise SystemExit('Expected exactly one widget and one fixed lexical map')
print('Prepared exact-release hashed patch locally. No files installed or services restarted.')
