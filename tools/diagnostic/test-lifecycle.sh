#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build-host
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/diagnostic/test-lifecycle.cpp -o build-host/test-lifecycle.moc
c++ -std=c++17 -fPIC tools/diagnostic/test-lifecycle.cpp -Ibuild-host -o build-host/diagnostic-lifecycle $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/diagnostic-lifecycle
