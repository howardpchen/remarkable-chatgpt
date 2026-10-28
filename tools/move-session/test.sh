#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
out="$PWD/build/move-session-host"
trial_flags=()
if [[ "${SINGLE_CAPTURE:-0}" == 1 ]]; then
    out="$PWD/build/move-session-single-capture-host"
    trial_flags=(-DINK_MOVE_SINGLE_CAPTURE)
fi
if [[ "${DOLLAR_BUDGET:-0}" == 1 ]]; then
    out="$out-budget"
    trial_flags+=(-DINK_MOVE_DOLLAR_BUDGET)
fi
mkdir -p "$out"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/move-demo/test-runtime.cpp -o "$out/test-runtime.moc"
c++ "${trial_flags[@]}" -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE -DINK_MOVE_LIVE_TEST -DINK_MOVE_PER_REQUEST tools/move-demo/test-runtime.cpp -I"$out" -o "$out/test-runtime" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Network Qt6Test) -ldl
c++ -std=c++17 -fPIC tools/move-session/test-quota.cpp -o "$out/test-quota" $(pkg-config --cflags --libs Qt6Core)
"$out/test-quota"
c++ -std=c++17 -fPIC tools/move-session/test-budget.cpp -o "$out/test-budget" $(pkg-config --cflags --libs Qt6Core)
"$out/test-budget"
c++ -std=c++17 -fPIC -DINK_MOVE_PER_REQUEST tools/move-session/test-page-profile.cpp -o "$out/test-page-profile" $(pkg-config --cflags --libs Qt6Gui Qt6Core)
"$out/test-page-profile"
c++ -std=c++17 -fPIC tools/move-session/test-template-ink.cpp -o "$out/test-template-ink" $(pkg-config --cflags --libs Qt6Gui Qt6Core)
"$out/test-template-ink"
for scenario in zoom-in zoom-out zoom-max zoom-grid zoom-dots zoom-pan-change-capture zoom-pan-change-api landscape-split-zoom-in landscape-split-zoom-out landscape-split-zoom-max landscape-split-zoom-pan-change-capture landscape-split-zoom-pan-change-api landscape-split-preview landscape-split landscape-split-change-api landscape-split-change-capture landscape-split-frame-mismatch landscape-split-missing landscape landscape-cw landscape-ccw landscape-missing landscape-change-api landscape-change-capture landscape-frame-mismatch old-reference missing-profile public-profile toolbar-unavailable toolbar-already-held zoomed-before-start template-before-start translated-before-start dots-before-start grid-before-start grid-table-before-start dots-change-api dots-empty grid-empty success cancel cancel-hidden page-change template-change zoom-change native-pose-change changed-pixels same-page-ink capture-timeout overall-timeout malformed unsupported needs-web clarification unknown-kind too-large http-error api-timeout cancel-api page-change-api template-change-api zoom-change-api shutdown-api teardown-api teardown-hidden; do
    test_dir=$(mktemp -d "$out/run.XXXXXX")
    INK_WORKFLOW_TEST_DIR="$test_dir" QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-runtime" "$scenario"
done
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/move-demo/test-ui.cpp -o "$out/test-ui.moc"
c++ -std=c++17 -fPIC -DINK_MOVE_LIVE_TEST -DINK_MOVE_PER_REQUEST tools/move-demo/test-ui.cpp -I"$out" -o "$out/test-ui" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-ui"
