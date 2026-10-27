#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
mkdir -p build/offline-workflow-host
out="$PWD/build/offline-workflow-host"
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/offline-workflow/test-workflow.cpp -o "$out/test-workflow.moc"
c++ -std=c++17 -fPIC tools/offline-workflow/test-workflow.cpp -I"$out" -o "$out/test-workflow" $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core) -ldl
test_dir=$(mktemp -d "$out/run.XXXXXX")
INK_WORKFLOW_TEST_DIR="$test_dir" QT_QPA_PLATFORM=offscreen "$out/test-workflow"
