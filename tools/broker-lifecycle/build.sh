#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact product SDK environment required}"
: "${SDK_PRODUCT:?Explicit product required}"
: "${SDK_RELEASE:?Explicit release required}"
: "${BROKER_REPO:?Pinned rm-xovi-extensions checkout required}"
: "${XOVI_REPO:?XOVI generator checkout required}"
[[ "$SDK_PRODUCT" == ferrari && "$SDK_RELEASE" == 3.28.0.172 ]] || { echo 'Only reviewed Ferrari 3.28.0.172 broker build supported';exit 1; }
out="${BROKER_BUILD_DIR:-$PWD/build/broker-lifecycle-ferrari}"
python3 tools/broker-lifecycle/prepare.py "$BROKER_REPO" "$out"
set +u
source "$SDK_ENV"
set -u
[[ "$($CXX -dumpmachine)" == aarch64* && "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
export XOVI_REPO
(cd "$out"; qmake xovi-message-broker.pro; make -j2)
python3 tools/offline-workflow/check-link-table.py "$out/xovi.cpp"
$STRIP --strip-unneeded "$out/xovi-message-broker.so"
python3 - "$out" <<'PY'
import hashlib,json,pathlib,struct,sys
p=pathlib.Path(sys.argv[1]);b=(p/'xovi-message-broker.so').read_bytes()
assert b[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',b,18)[0]==183
m={'upstream_commit':'6c709dd8416ddf89034831073d76fb4ee3372905','broker_abi':'0.2.0','local_lifecycle_overlay':True,'product':'ferrari','release':'3.28.0.172','sha256':hashlib.sha256(b).hexdigest(),'deployed':False,'sources':{str(f.relative_to(p)):hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted((p/'src').glob('*')) if f.is_file()}}
(p/'lifecycle-build.json').write_text(json.dumps(m,indent=2)+'\n')
PY
