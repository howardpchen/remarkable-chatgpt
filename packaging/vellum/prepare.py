#!/usr/bin/env python3
"""Stage only generic artifacts and exact checksums for the maintained vbuild tool."""
import argparse,hashlib,pathlib,shutil,struct,subprocess,tempfile
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--device',choices=['chiappa','ferrari'],default='chiappa')
p.add_argument('--module',type=pathlib.Path)
p.add_argument('--cli',type=pathlib.Path)
p.add_argument('--document-view',type=pathlib.Path)
p.add_argument('--release',type=int,default=0)
p.add_argument('--qmldiff',required=True,type=pathlib.Path)
p.add_argument('--hashtab',required=True,type=pathlib.Path)
p.add_argument('--output',required=True,type=pathlib.Path)
a=p.parse_args();repo=pathlib.Path(__file__).resolve().parents[2];out=a.output.resolve()
out.mkdir(parents=True,exist_ok=False,mode=0o700)
ferrari=a.device=='ferrari'
assert a.release>=0
module=a.module or repo/('build/paper-pro-session-ferrari/ink-diary-paper-pro-live.so' if ferrari else 'build/move-session-single-capture-budget/ink-diary-move-live.so')
cli=a.cli or repo/('build/paper-pro-session-ferrari/remarkable-ai-ferrari' if ferrari else 'build/remarkable-ai/remarkable-ai')
for source,name in [(module,'remarkable-ai.so'),(cli,'remarkable-ai')]:
    data=source.read_bytes();assert data[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',data,18)[0]==183
    own=b'/home/root/xovi/paper-pro-session-portrait-v1' if ferrari else b'/home/root/xovi/move-session-small-templates-v4'
    assert own in data, 'Wrong device artifact'
    shutil.copyfile(source,out/name)
widget='\n'.join(l for l in (repo/'tools/move-session/MoveLive.qml').read_text().splitlines() if not l.startswith('import '))
qmd=('VERSION 3.28.0.172\nAFFECT /qml/device/view/documentview/DocumentView.qml\n IMPORT net.asivery.XoviMessageBroker 2.0\n TRAVERSE FocusScope#root\n LOCATE AFTER ALL\n INSERT {\n'+(repo/'tools/move-demo/NativePoseBindings.qml.inc').read_text()+'\n'+widget+'\n }\n END TRAVERSE\nEND AFFECT\n')
(out/'MoveLive.qmd').write_text(qmd)
subprocess.run([str(a.qmldiff.resolve()),'hash-diffs',str(a.hashtab.resolve()),str(out/'MoveLive.qmd')],check=True)
if ferrari:
    assert a.document_view, '--document-view required for reviewed Ferrari resource'
    with tempfile.TemporaryDirectory() as temp:
        qout=pathlib.Path(temp)/'checked'
        subprocess.run(['python3',str(repo/'tools/paper-pro-portrait/session/build-qmd.py'),'--qmldiff',str(a.qmldiff.resolve()),'--hashtab',str(a.hashtab.resolve()),'--document-view',str(a.document_view.resolve()),'--output',str(qout)],check=True)
        shutil.copyfile(qout/'paper-pro-live.qmd',out/'MoveLive.qmd')
for src,name in [(repo/'LICENSE','LICENSE'),(repo/('packaging/vellum/SETUP-FERRARI.md' if ferrari else 'packaging/vellum/SETUP.md'),'SETUP.md')]:shutil.copyfile(src,out/name)
names=['remarkable-ai.so','remarkable-ai','MoveLive.qmd','LICENSE','SETUP.md']
checks='\n'.join(hashlib.sha512((out/n).read_bytes()).hexdigest()+'  '+n for n in names)
recipe=(repo/'packaging/vellum/VELBUILD').read_text().replace('# @SHA512SUMS@',"sha512sums='"+checks+"'")
recipe=recipe.replace('pkgrel=0','pkgrel='+str(a.release))
if ferrari:
    recipe=recipe.replace('remarkable-ai-move','remarkable-ai-ferrari').replace('rmppm remarkable-os','rmpp remarkable-os').replace('ink-diary-move-live.so','ink-diary-paper-pro-live.so').replace('ink-diary-ui-probe.qmd','paper-pro-live.qmd').replace('/bin/remarkable-ai"','/bin/remarkable-ai-ferrari"').replace('/share/remarkable-ai/','/share/remarkable-ai-ferrari/').replace('reMarkable Move firmware','reMarkable Paper Pro Ferrari firmware')
(out/'VELBUILD').write_text(recipe)
print('Staged generic package input at',out)
