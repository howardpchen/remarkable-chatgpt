#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact Chiappa SDK required}"
: "${XOVI_REPO:?XOVI source required}"
set +u
source "$SDK_ENV"
set -u
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
case "$($CXX -dumpmachine)" in aarch64*) ;; *) exit 1;; esac
mkdir -p build/capture-ui-probe
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o build/capture-ui-probe/xovi.cpp -H build/capture-ui-probe/xovi.h tools/capture-ui-probe/probe.xovi
$CXX -std=c++17 -shared -fPIC -Wl,-Bsymbolic -Wl,--version-script=tools/capture-ui-probe/probe.map tools/capture-ui-probe/probe.cpp build/capture-ui-probe/xovi.cpp -Ibuild/capture-ui-probe -o build/capture-ui-probe/ink-diary-capture-ui.so $(pkg-config --cflags --libs Qt6Core Qt6Gui Qt6Qml Qt6Quick) -ldl
