#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
mkdir -p build-host
c++ -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE tests/cancellation.cpp -o build-host/move-no-network $(pkg-config --cflags --libs Qt6Network Qt6Core)
./build-host/move-no-network
c++ -std=c++17 -fPIC tests/move-runtime.cpp -o build-host/move-runtime $(pkg-config --cflags --libs Qt6Gui Qt6Core)
./build-host/move-runtime
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tests/move-snapshot.cpp -o build-host/move-snapshot.moc
c++ -std=c++17 -fPIC tests/move-snapshot.cpp -Ibuild-host -o build-host/move-snapshot $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/move-snapshot
