#!/usr/bin/env bash
set -euo pipefail
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
: "${SDK_ENV:?Exact Move SDK required}"
: "${XOVI_REPO:?XOVI source required}"
set +u
source "$SDK_ENV"
set -u
[[ "$($CXX -dumpmachine)" == aarch64* ]]
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
build_dir="$repo/build/offline-spike"
mkdir -p "$build_dir" "$repo/bin/chiappa"
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o "$build_dir/xovi.cpp" -H "$build_dir/xovi.h" "$repo/tools/offline-spike/spike.xovi"
$CXX -std=c++17 -shared -fPIC -Wl,-Bsymbolic -Wl,--version-script="$repo/tools/offline-spike/spike.map" \
 "$repo/tools/offline-spike/spike.cpp" "$build_dir/xovi.cpp" -I"$build_dir" \
 -o "$repo/bin/chiappa/ink-diary-offline.so" $(pkg-config --cflags --libs Qt6Core Qt6Gui Qt6Qml Qt6Quick) -ldl
$STRIP --strip-unneeded "$repo/bin/chiappa/ink-diary-offline.so"
python3 - "$repo" <<'PY'
import hashlib,json,pathlib,struct,sys
r=pathlib.Path(sys.argv[1]);p=r/'bin/chiappa/ink-diary-offline.so';d=p.read_bytes()
assert d[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',d,18)[0]==183
sources={str(p.relative_to(r)):hashlib.sha256(p.read_bytes()).hexdigest() for folder in ['tools/offline-spike','tools/diagnostic'] for p in sorted((r/folder).glob('*')) if p.is_file()}
m={'offline_only':True,'model':'chiappa','release':'3.28.0.172','qt':'6.10.3','sha256':hashlib.sha256(d).hexdigest(),'sources':sources}
(r/'bin/chiappa/offline-build.json').write_text(json.dumps(m,indent=2)+'\n')
print(m['sha256'])
PY
