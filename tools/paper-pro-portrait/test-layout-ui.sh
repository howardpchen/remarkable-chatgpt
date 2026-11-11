#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
out="$PWD/build/paper-pro-portrait-layout-host"
mkdir -p "$out"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/paper-pro-portrait/test-layout-ui.cpp -o "$out/test-layout-ui.moc"
c++ -std=c++17 -fPIC tools/paper-pro-portrait/test-layout-ui.cpp -I"$out" -o "$out/test-layout-ui" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-layout-ui"
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-layout-ui" session

QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-layout-ui" session-cw
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-layout-ui" session-ccw
