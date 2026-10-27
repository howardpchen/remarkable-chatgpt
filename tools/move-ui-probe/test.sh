#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build/move-ui-host
c++ -std=c++17 -fPIC tools/move-ui-probe/test-ui.cpp -o build/move-ui-host/test-ui $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software ./build/move-ui-host/test-ui
c++ -std=c++17 -fPIC tools/move-ui-probe/test-capture-ui.cpp -o build/move-ui-host/test-capture-ui $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software ./build/move-ui-host/test-capture-ui
