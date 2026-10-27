#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build/move-live-host
out="$PWD/build/move-live-host"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/move-demo/test-ui.cpp -o "$out/test-ui.moc"
c++ -std=c++17 -fPIC -DINK_MOVE_LIVE_TEST tools/move-demo/test-ui.cpp -I"$out" -o "$out/test-ui" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-ui"
echo 'PASS: live QML Start/Cancel, compact idle launcher, toolbar-aware dismissal and capture bindings'
