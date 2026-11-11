#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../../.."
: "${SDK_ENV:?Exact Ferrari SDK environment required}"
: "${XOVI_REPO:?XOVI source required}"
[[ "${SDK_PRODUCT:-}" == ferrari && "${SDK_RELEASE:-}" == 3.28.0.172 ]]
set +u
source "$SDK_ENV"
set -u
[[ "$($CXX -dumpmachine)" == aarch64* ]]
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
out="${PAPER_PRO_BUILD_DIR:-$PWD/build/paper-pro-session-ferrari}"
: "${CXXFLAGS:?SDK optimization flags required}"
: "${LDFLAGS:?SDK linker flags required}"
mkdir -p "$out"
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o "$out/xovi.cpp" -H "$out/xovi.h" tools/paper-pro-portrait/session/live.xovi
python3 tools/offline-workflow/check-link-table.py "$out/xovi.cpp"
$CXX $CXXFLAGS -std=c++17 -shared -fPIC -Wl,-Bsymbolic -Wl,--version-script=tools/paper-pro-portrait/session/live.map tools/paper-pro-portrait/session/entry.cpp "$out/xovi.cpp" -I"$out" -o "$out/ink-diary-paper-pro-live.so" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Network) -ldl $LDFLAGS
$STRIP --strip-unneeded "$out/ink-diary-paper-pro-live.so"
$CXX $CXXFLAGS -std=c++17 -fPIC tools/paper-pro-portrait/session/admin.cpp -o "$out/remarkable-ai-ferrari" $(pkg-config --cflags --libs Qt6Core) $LDFLAGS
$STRIP --strip-unneeded "$out/remarkable-ai-ferrari"
python3 - "$out/ink-diary-paper-pro-live.so" <<'PY'
import pathlib,struct,sys,hashlib
raw=pathlib.Path(sys.argv[1]).read_bytes()
assert raw[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',raw,18)[0]==183
assert b'/home/root/xovi/paper-pro-session-portrait-v1\0' in raw
assert b'/home/root/xovi/move-session-small-templates-v4\0' not in raw
print('PASS exact Ferrari SDK runtime candidate AArch64:',hashlib.sha256(raw).hexdigest())
PY
