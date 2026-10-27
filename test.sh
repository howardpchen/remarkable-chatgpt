#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
mkdir -p build-host
c++ -std=c++17 -fPIC tests/cancellation.cpp -o build-host/cancellation $(pkg-config --cflags --libs Qt6Network Qt6Core)
./build-host/cancellation
c++ -std=c++17 -fPIC tools/diagnostic/test-policy.cpp -o build-host/diagnostic-policy $(pkg-config --cflags --libs Qt6Core)
./build-host/diagnostic-policy
c++ -std=c++17 -fPIC -DINK_DIAGNOSTIC_FERRARI tools/diagnostic/test-policy.cpp -o build-host/diagnostic-policy-ferrari $(pkg-config --cflags --libs Qt6Core)
./build-host/diagnostic-policy-ferrari
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/diagnostic/test-mapping.cpp -o build-host/test-mapping.moc
c++ -std=c++17 -fPIC tools/diagnostic/test-mapping.cpp -Ibuild-host -o build-host/diagnostic-mapping $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/diagnostic-mapping
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/diagnostic/test-template.cpp -o build-host/test-template.moc
c++ -std=c++17 -fPIC tools/diagnostic/test-template.cpp -Ibuild-host -o build-host/diagnostic-template $(pkg-config --cflags --libs Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/diagnostic-template
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/offline-spike/test-spike.cpp -o build-host/test-spike.moc
c++ -std=c++17 -fPIC tools/offline-spike/test-spike.cpp -Ibuild-host -o build-host/offline-spike-test $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core) -ldl
INK_SPIKE_TEST_DIR="$PWD/build-host/offline-test-artifacts" QT_QPA_PLATFORM=offscreen ./build-host/offline-spike-test
c++ -std=c++17 -fPIC tests/capture-layout.cpp -o build-host/capture-layout $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/capture-layout
bash tools/paper-pro-portrait/test.sh
node tests/append.cjs
node tests/viewport.cjs
python3 tests/port-scripts.py
c++ -std=c++17 -fPIC tests/core.cpp -o build-host/core $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/core "$@"
c++ -std=c++17 -fPIC tests/plot.cpp -o build-host/plot $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/plot
c++ -std=c++17 -fPIC tests/web.cpp -o build-host/web $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/web
c++ -std=c++17 -fPIC tests/markdown.cpp -o build-host/markdown $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/markdown
c++ -std=c++17 -fPIC src/config-model.cpp -o build-host/config-model $(pkg-config --cflags --libs Qt6Core)
python3 tests/config-model.py
c++ -std=c++17 -fPIC tests/templates.cpp -o build-host/templates $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/templates
c++ -std=c++17 -fPIC tests/visuals.cpp -o build-host/visuals $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/visuals
c++ -std=c++17 -fPIC tests/calendar.cpp -o build-host/calendar $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen ./build-host/calendar
