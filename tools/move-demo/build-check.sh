#!/usr/bin/env bash
# Compile-only SDK check. Not a tablet-loadable artifact or installation command.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact Chiappa SDK required}"
set +u
source "$SDK_ENV"
set -u
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
case "$($CXX -dumpmachine)" in aarch64*) ;; *) exit 1;; esac
mkdir -p build/move-demo
$CXX -std=c++17 -fPIC -DINK_DIARY_EXPERIMENTAL_MOVE -c tools/move-demo/test.cpp -o build/move-demo/sdk-admission-check.o $(pkg-config --cflags Qt6Quick Qt6Qml Qt6Gui Qt6Core)
