#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build/move-demo
c++ -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE tools/move-demo/test.cpp -o build/move-demo/test $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software build/move-demo/test
# Prove that even a direct API helper call in the future Move demo fails before
# network creation. Existing test covers cancellation and request-counter zero.
c++ -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE tests/cancellation.cpp -o build/move-demo/no-network $(pkg-config --cflags --libs Qt6Network Qt6Core)
build/move-demo/no-network
bash tools/move-demo/test-runtime.sh
c++ -std=c++17 -fPIC tools/diagnostic/test-native-pose.cpp -o build/move-demo/native-pose-test $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software build/move-demo/native-pose-test
