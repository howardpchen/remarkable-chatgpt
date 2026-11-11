#!/usr/bin/env python3
"""Apply the Ferrari widget to an exact private resource, locally only."""
import argparse,hashlib,os,re,shutil,subprocess
from pathlib import Path
os.umask(0o077)
a=argparse.ArgumentParser(description=__doc__)
a.add_argument('--qmldiff',required=True,type=Path)
a.add_argument('--hashtab',required=True,type=Path)
a.add_argument('--document-view',required=True,type=Path)
a.add_argument('--output',required=True,type=Path)
a=a.parse_args()
s=Path(__file__).resolve().parent
original=a.document_view.read_bytes()
if hashlib.sha256(original).hexdigest()!='867b30a725a4ae97e3459f07a60cd72bcafb21db9a0b374f02d75c5cd8d83269':
 raise SystemExit('Refusing an unreviewed Ferrari DocumentView resource')
bridge=(s/'NativePoseBindings.qml.inc').read_text()
ids=re.findall(r'^\s*(\w+): (\w+),?$',bridge,re.M)
if len(ids)!=23 or not all(x==y and re.search(r'\bid\s*:\s*'+re.escape(y)+r'\b',original.decode()) for x,y in ids):
 raise SystemExit('Ferrari native bridge missing or ambiguous')
if not re.search(r'\bproperty bool subDialogOpen\b',original.decode()):
 raise SystemExit('Ferrari dialog ABI mismatch')
a.output.mkdir(mode=0o700,parents=True,exist_ok=False)
root=a.output/'original/qml/device/view/documentview';root.mkdir(parents=True)
(root/'DocumentView.qml').write_bytes(original)
qml='\n'.join(line for line in (s/'PaperProLive.qml').read_text().splitlines() if not line.startswith('import '))
raw='VERSION 3.28.0.172\nAFFECT /qml/device/view/documentview/DocumentView.qml\n IMPORT net.asivery.XoviMessageBroker 2.0\n TRAVERSE FocusScope#root\n LOCATE AFTER ALL\n INSERT {\n'+bridge+'\n'+qml+'\n }\n END TRAVERSE\nEND AFFECT\n'
p=a.output/'paper-pro-live.qmd';p.write_text(raw)
subprocess.run([str(a.qmldiff),'hash-diffs',str(a.hashtab),str(p)],check=True)
subprocess.run([str(a.qmldiff),'apply-diffs','--version','3.28.0.172','--hashtab',str(a.hashtab),str(a.output/'original'),str(a.output/'patched'),str(p)],check=True)
patched=(a.output/'patched/qml/device/view/documentview/DocumentView.qml').read_text()
if patched.count('id: moveUiProbe\n')!=1 or patched.count('diaryNativePoseObjects:')!=1:
 raise SystemExit('Expected exactly one unchanged-size widget and native bridge')
print('PASS local exact-resource Ferrari QMD; no device action')
