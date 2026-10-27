#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
out="$PWD/build/paper-pro-portrait-host"
mkdir -p "$out"
c++ -std=c++17 -fPIC tools/paper-pro-portrait/test-geometry.cpp -o "$out/test-geometry" $(pkg-config --cflags --libs Qt6Gui Qt6Core)
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software "$out/test-geometry"
