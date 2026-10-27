#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
out="$PWD/build/remarkable-ai-host"
mkdir -p "$out"
c++ -std=c++17 -fPIC tools/remarkable-ai/main.cpp -o "$out/remarkable-ai" $(pkg-config --cflags --libs Qt6Core)
c++ -std=c++17 -fPIC tools/remarkable-ai/test-provider.cpp -o "$out/test-provider" $(pkg-config --cflags --libs Qt6Core)
c++ -std=c++17 -fPIC tools/move-live/configure.cpp -o "$out/configure-move-live" $(pkg-config --cflags --libs Qt6Core)
python3 tools/remarkable-ai/test-cli.py "$out/remarkable-ai"
python3 tools/move-live/test-configure.py "$out/configure-move-live"
"$out/test-provider"
if readelf -d "$out/remarkable-ai" | grep -q Qt6Network; then exit 1; fi
printf '%s\n' 'PASS sanitized provider metadata; CLI has no QtNetwork dependency'
