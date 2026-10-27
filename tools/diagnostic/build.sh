#!/usr/bin/env bash
set -euo pipefail
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
: "${SDK_ENV:?Set the exact chiappa 3.28.0.172 SDK environment file}"
: "${XOVI_REPO:?Set XOVI_REPO to the xovi source checkout}"
SDK_ENV=$(realpath "$SDK_ENV")
XOVI_REPO=$(realpath "$XOVI_REPO")
set +u
source "$SDK_ENV"
set -u
case "$($CXX -dumpmachine)" in aarch64*) ;; *) echo 'Requires AArch64 SDK' >&2; exit 1;; esac
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]] || { echo 'Requires Qt 6.10.3 SDK' >&2; exit 1; }
build_dir="$repo/build/diagnostic"
out_dir="$repo/bin/chiappa"
mkdir -p "$build_dir" "$out_dir"
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o "$build_dir/xovi.cpp" -H "$build_dir/xovi.h" "$repo/tools/diagnostic/probe.xovi"
$CXX -std=c++17 -shared -fPIC -fvisibility=default -Wl,-Bsymbolic \
    -Wl,--version-script="$repo/tools/diagnostic/probe.map" \
    "$repo/tools/diagnostic/probe.cpp" "$build_dir/xovi.cpp" -I"$build_dir" \
    -o "$out_dir/ink-diary-probe.so" $(pkg-config --cflags --libs Qt6Core Qt6Gui Qt6Qml Qt6Quick) -ldl
$STRIP --strip-unneeded "$out_dir/ink-diary-probe.so"
python3 - "$repo" <<'PY'
import hashlib,json,pathlib,struct,sys
repo=pathlib.Path(sys.argv[1]); artifact=repo/'bin/chiappa/ink-diary-probe.so'
data=artifact.read_bytes()
assert data[:5]==b'\x7fELF\x02' and data[5]==1 and struct.unpack_from('<H',data,18)[0]==183, 'Not AArch64 ELF64'
source_paths=list((repo/'tools/diagnostic').glob('*'))+[repo/'src/readonly-template.h',repo/'src/readonly-mapping.h']
sources={str(p.relative_to(repo)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(source_paths) if p.is_file()}
manifest={'diagnostic_only':True,'model':'chiappa','release':'3.28.0.172','qt':'6.10.3','artifact':artifact.name,'sha256':hashlib.sha256(data).hexdigest(),'sources':sources}
(repo/'bin/chiappa/diagnostic-build.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(artifact)
print(manifest['sha256'])
PY
