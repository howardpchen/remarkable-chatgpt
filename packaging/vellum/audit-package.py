#!/usr/bin/env python3
"""Check signed APK payload against its prepared generic input (no extraction)."""
import argparse, hashlib, json, pathlib, tarfile
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--device', choices=['chiappa','ferrari'], required=True)
p.add_argument('--input',type=pathlib.Path,required=True)
p.add_argument('apk',type=pathlib.Path)
a=p.parse_args(); ferrari=a.device=='ferrari'
name='remarkable-ai-ferrari' if ferrari else 'remarkable-ai-move'
cli='remarkable-ai-ferrari' if ferrari else 'remarkable-ai'
expected={
 'home/root/xovi/extensions.d/'+('ink-diary-paper-pro-live.so' if ferrari else 'ink-diary-move-live.so'):'remarkable-ai.so',
 'home/root/xovi/exthome/qt-resource-rebuilder/'+('paper-pro-live.qmd' if ferrari else 'ink-diary-ui-probe.qmd'):'MoveLive.qmd',
 'home/root/.vellum/bin/'+cli:'remarkable-ai',
 'home/root/.vellum/share/'+('remarkable-ai-ferrari' if ferrari else 'remarkable-ai')+'/SETUP.md':'SETUP.md',
 'home/root/.vellum/licenses/'+name+'/LICENSE':'LICENSE',
 'home/root/.vellum/licenses/'+name+'/SOURCES':None,
}
files={}; controls={}
with tarfile.open(a.apk,'r:gz',ignore_zeros=True) as t:
 for m in t:
  assert not m.issym() and not m.islnk(), 'Unexpected link'
  if not m.isfile():continue
  key=m.name.removeprefix('./');data=t.extractfile(m).read()
  assert key not in files and key not in controls,'Duplicate entry'
  if key.startswith('.'):controls[key]=data
  else:files[key]=data
assert set(files)==set(expected), (set(files)-set(expected),set(expected)-set(files))
assert any(k.startswith('.SIGN.RSA.') for k in controls),'Missing signature'
assert set(controls)-{'.PKGINFO'}=={k for k in controls if k.startswith('.SIGN.RSA.')},'Lifecycle scripts or unexpected control data'
info=controls['.PKGINFO'].decode();assert 'pkgname = '+name+'\n' in info
assert 'depend = '+('rmpp' if ferrari else 'rmppm')+'\n' in info
assert 'depend = remarkable-os=3.28.0.172-r0\n' in info
for dst,src in expected.items():
 if src:assert files[dst]==(a.input/src).read_bytes(), dst+' differs from prepared input'
 else:assert files[dst]==b'https://github.com/howardpchen/remarkable-chatgpt\n'
print(json.dumps({'apk':str(a.apk),'sha256':hashlib.sha256(a.apk.read_bytes()).hexdigest(),'device':a.device,'payloadFiles':len(files),'noHooksOrPrivateState':True,'exactPreparedPayload':True},indent=2))
