#!/usr/bin/env bash
# Cross-build a TEST executable only. Never install or launch on a tablet here.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact Ferrari SDK environment required}"
[[ "${SDK_PRODUCT:-}" == ferrari && "${SDK_RELEASE:-}" == 3.28.0.172 ]]
set +u
source "$SDK_ENV"
set -u
[[ "$($CXX -dumpmachine)" == aarch64* ]]
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
out="$PWD/build/paper-pro-portrait-layout-ferrari"
mkdir -p "$out"
# Never reuse host moc output across Qt versions.
"$OECORE_NATIVE_SYSROOT/usr/libexec/moc" tools/paper-pro-portrait/test-layout-ui.cpp -o "$out/test-layout-ui.moc"
$CXX -std=c++17 -fPIC tools/paper-pro-portrait/test-layout-ui.cpp -I"$out" -o "$out/test-layout-ui" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
python3 - "$out/test-layout-ui" <<'PY'
import pathlib,struct,sys
raw=pathlib.Path(sys.argv[1]).read_bytes()
assert raw[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',raw,18)[0]==183
print('PASS exact Ferrari SDK AArch64 test executable; not run on tablet, not an AI runtime')
PY
