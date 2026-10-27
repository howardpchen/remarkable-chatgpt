#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build/move-demo-host
out="$PWD/build/move-demo-host"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/move-demo/test-runtime.cpp -o "$out/test-runtime.moc"
c++ -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE tools/move-demo/test-runtime.cpp -I"$out" -o "$out/test-runtime" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test) -ldl
for scenario in success cancel cancel-hidden page-change template-change zoom-change native-pose-change changed-pixels capture-timeout overall-timeout; do
    test_dir=$(mktemp -d "$out/run.XXXXXX")
    INK_WORKFLOW_TEST_DIR="$test_dir" QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-runtime" "$scenario"
done
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/move-demo/test-ui.cpp -o "$out/test-ui.moc"
c++ -std=c++17 -fPIC tools/move-demo/test-ui.cpp -I"$out" -o "$out/test-ui" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-ui"
echo 'PASS: actual demo QML pointer Start/Cancel, no on-load Start, exact control geometry and suppression binding'
