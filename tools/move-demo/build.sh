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
mkdir -p build/move-demo
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o build/move-demo/xovi.cpp -H build/move-demo/xovi.h tools/move-demo/demo.xovi
python3 tools/offline-workflow/check-link-table.py build/move-demo/xovi.cpp
$CXX -std=c++17 -DINK_DIARY_EXPERIMENTAL_MOVE -shared -fPIC -Wl,-Bsymbolic -Wl,--version-script=tools/move-demo/demo.map tools/move-demo/runtime.cpp build/move-demo/xovi.cpp -Ibuild/move-demo -o build/move-demo/ink-diary-move-demo.so $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core) -ldl
$STRIP --strip-unneeded build/move-demo/ink-diary-move-demo.so
sha256sum build/move-demo/ink-diary-move-demo.so
