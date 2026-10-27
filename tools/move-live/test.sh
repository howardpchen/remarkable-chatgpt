#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build/move-live-host
out="$PWD/build/move-live-host"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/move-demo/test-runtime.cpp -o "$out/test-runtime.moc"
c++ -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE -DINK_MOVE_LIVE_TEST tools/move-demo/test-runtime.cpp -I"$out" -o "$out/test-runtime" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Network Qt6Test) -ldl
for scenario in success cancel cancel-hidden page-change template-change zoom-change native-pose-change changed-pixels capture-timeout overall-timeout malformed unsupported too-large http-error api-timeout cancel-api page-change-api template-change-api zoom-change-api teardown-api teardown-hidden; do
    test_dir=$(mktemp -d "$out/run.XXXXXX")
    INK_WORKFLOW_TEST_DIR="$test_dir" QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-runtime" "$scenario"
done
c++ -std=c++17 -fPIC tools/move-live/test-client.cpp -o "$out/test-client" $(pkg-config --cflags --libs Qt6Gui Qt6Core Qt6Network Qt6Test)
"$out/test-client"
bash tools/move-live/test-ui.sh
