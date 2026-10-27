#!/usr/bin/env python3
"""Stage only generic artifacts and exact checksums for the maintained vbuild tool."""
import argparse,hashlib,pathlib,shutil,struct,subprocess
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--qmldiff',required=True,type=pathlib.Path)
p.add_argument('--hashtab',required=True,type=pathlib.Path)
p.add_argument('--output',required=True,type=pathlib.Path)
a=p.parse_args();repo=pathlib.Path(__file__).resolve().parents[2];out=a.output.resolve()
out.mkdir(parents=True,exist_ok=False,mode=0o700)
for source,name in [(repo/'build/move-session-single-capture-budget/ink-diary-move-live.so','remarkable-ai.so'),(repo/'build/remarkable-ai/remarkable-ai','remarkable-ai')]:
    data=source.read_bytes();assert data[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',data,18)[0]==183
    shutil.copyfile(source,out/name)
widget='\n'.join(l for l in (repo/'tools/move-session/MoveLive.qml').read_text().splitlines() if not l.startswith('import '))
qmd=('VERSION 3.28.0.172\nAFFECT /qml/device/view/documentview/DocumentView.qml\n IMPORT net.asivery.XoviMessageBroker 2.0\n TRAVERSE FocusScope#root\n LOCATE AFTER ALL\n INSERT {\n'+(repo/'tools/move-demo/NativePoseBindings.qml.inc').read_text()+'\n'+widget+'\n }\n END TRAVERSE\nEND AFFECT\n')
(out/'MoveLive.qmd').write_text(qmd)
subprocess.run([str(a.qmldiff.resolve()),'hash-diffs',str(a.hashtab.resolve()),str(out/'MoveLive.qmd')],check=True)
for src,name in [(repo/'LICENSE','LICENSE'),(repo/'packaging/vellum/SETUP.md','SETUP.md')]:shutil.copyfile(src,out/name)
names=['remarkable-ai.so','remarkable-ai','MoveLive.qmd','LICENSE','SETUP.md']
checks='\n'.join(hashlib.sha512((out/n).read_bytes()).hexdigest()+'  '+n for n in names)
recipe=(repo/'packaging/vellum/VELBUILD').read_text().replace('# @SHA512SUMS@',"sha512sums='"+checks+"'")
(out/'VELBUILD').write_text(recipe)
print('Staged generic package input at',out)
