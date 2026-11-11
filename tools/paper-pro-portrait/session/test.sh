#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../../.."
out="$PWD/build/paper-pro-session-host"
mkdir -p "$out"
c++ -O2 -std=c++17 -fPIC tools/paper-pro-portrait/session/test-template-profile.cpp -o "$out/test-template-profile" $(pkg-config --cflags --libs Qt6Gui Qt6Core)
"$out/test-template-profile"
c++ -O2 -std=c++17 -fPIC tools/paper-pro-portrait/session/test-template-ink.cpp -o "$out/test-template-ink" $(pkg-config --cflags --libs Qt6Gui Qt6Core)
"$out/test-template-ink"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/paper-pro-portrait/session/test-runtime.cpp -o "$out/test-runtime.moc"
c++ -O2 -std=c++17 -fPIC -DINK_DIAGNOSTIC_FERRARI -DINK_DIARY_EXPERIMENTAL_PAPER_PRO -DINK_PAPER_PRO_LIVE_TEST -DINK_PAPER_PRO_PER_REQUEST -DINK_PAPER_PRO_DOLLAR_BUDGET tools/paper-pro-portrait/session/test-runtime.cpp -I"$out" -o "$out/test-runtime" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Network Qt6Test) -ldl
for scenario in landscape-split-grid-missing landscape-split-grid-zoom-template-change-api landscape-split-dots-missing landscape-split-dots-zoom-template-change-api landscape-split-reverse-grid-missing landscape-split-reverse-grid-zoom-template-change-api landscape-split-reverse-dots-missing landscape-split-reverse-dots-zoom-template-change-api landscape-split-grid-zoom-in landscape-split-grid-zoom-out landscape-split-grid-zoom-max landscape-split-dots-zoom-in landscape-split-dots-zoom-out landscape-split-dots-zoom-max landscape-split-reverse-grid-zoom-in landscape-split-reverse-grid-zoom-out landscape-split-reverse-grid-zoom-max landscape-split-reverse-dots-zoom-in landscape-split-reverse-dots-zoom-out landscape-split-reverse-dots-zoom-max translated-before-start zoom-in zoom-out zoom-max zoom-grid zoom-dots zoom-change-capture zoom-pan-change-api landscape-split-reverse-zoom-in landscape-split-reverse-zoom-out landscape-split-reverse-zoom-max landscape-split-reverse-zoom-change-api landscape-split-reverse-zoom-change-capture landscape-split-zoom-in landscape-split-zoom-out landscape-split-zoom-max landscape-split-zoom-change-capture landscape-split-zoom-change-api landscape-split landscape-split-reverse landscape-split-missing landscape-split-change-api landscape-split-change-capture landscape-split-frame-mismatch landscape-split-reverse-change-api landscape-split-reverse-change-capture dots-before-start grid-before-start grid-table-before-start dots-change-api dots-empty grid-empty count-over-cap count-malformed count-http-error count-timeout cancel-count page-change-count success cancel cancel-hidden toolbar-unavailable toolbar-already-held missing-profile public-profile zoomed-before-start template-before-start native-pose-change page-change template-change zoom-change changed-pixels same-page-ink capture-timeout overall-timeout malformed unsupported needs-web clarification unknown-kind too-large http-error api-timeout cancel-api page-change-api template-change-api zoom-change-api shutdown-api teardown-api teardown-hidden; do
 test_dir=$(mktemp -d "$out/run.XXXXXX")
 INK_WORKFLOW_TEST_DIR="$test_dir" QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-runtime" "$scenario"
done
c++ -O2 -std=c++17 -fPIC tools/paper-pro-portrait/session/test-native-pose.cpp -o "$out/test-native-pose" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-native-pose"
c++ -O2 -std=c++17 -fPIC tools/paper-pro-portrait/session/admin.cpp -o "$out/remarkable-ai-ferrari" $(pkg-config --cflags --libs Qt6Core)
"$out/remarkable-ai-ferrari" --help

c++ -O2 -std=c++17 -fPIC tools/paper-pro-portrait/session/test-budget-policy.cpp -o "$out/test-budget-policy" $(pkg-config --cflags --libs Qt6Core)
"$out/test-budget-policy"
