#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact Chiappa SDK required}"
: "${XOVI_REPO:?XOVI source checkout required}"
set +u
source "$SDK_ENV"
set -u
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
[[ "$($CXX -dumpmachine)" == aarch64* ]]
mkdir -p build/move-live
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o build/move-live/xovi.cpp -H build/move-live/xovi.h tools/move-live/live.xovi
python3 tools/offline-workflow/check-link-table.py build/move-live/xovi.cpp
$CXX -std=c++17 -DINK_DIARY_EXPERIMENTAL_MOVE -shared -fPIC -Wl,-Bsymbolic -Wl,--version-script=tools/move-live/live.map tools/move-live/runtime.cpp build/move-live/xovi.cpp -Ibuild/move-live -o build/move-live/ink-diary-move-live.so $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Network) -ldl
$STRIP --strip-unneeded build/move-live/ink-diary-move-live.so
$CXX -std=c++17 -fPIC tools/move-live/configure.cpp -o build/move-live/configure-move-live $(pkg-config --cflags --libs Qt6Core)
$STRIP --strip-unneeded build/move-live/configure-move-live
sha256sum build/move-live/ink-diary-move-live.so
python3 - <<'PY'
import hashlib,json,pathlib,struct,re
p=pathlib.Path('build/move-live/ink-diary-move-live.so')
b=p.read_bytes()
assert b[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',b,18)[0]==183, 'Expected AArch64 ELF64'
meta={'artifact':str(p),'sha256':hashlib.sha256(b).hexdigest(),'architecture':'aarch64',
      'qt':'6.10.3','target':'bounded-move-live-text','model':'gpt-6-luna',
      'reasoning_effort':'none','max_requests':1,'max_output_tokens':800,'deployed':False,'live_provider_tested':False}
run=re.findall(r'runDirectory\[\]\s*=\s*"([^"]+)"',pathlib.Path('tools/move-live/run-directory.h').read_text())[-1]
assert run.startswith('/home/root/xovi/') and '..' not in run and run!='/home/root/xovi/move-live-text-v1'
assert run.encode()+b'\0' in b, 'Run path not embedded in artifact'
meta['run_directory']=run
helper=p.parent/'configure-move-live'
hb=helper.read_bytes()
assert hb[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',hb,18)[0]==183
meta['helper']={'artifact':str(helper),'sha256':hashlib.sha256(hb).hexdigest(),'architecture':'aarch64',
                'network':False,'deployed':False}
p.with_suffix('.json').write_text(json.dumps(meta,indent=2)+'\n')
print('PASS: AArch64 artifact and local-only build metadata')
PY
