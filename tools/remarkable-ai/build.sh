#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact Chiappa SDK required}"
set +u
source "$SDK_ENV"
set -u
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
[[ "$($CXX -dumpmachine)" == aarch64* ]]
mkdir -p build/remarkable-ai
$CXX -std=c++17 -fPIC tools/remarkable-ai/main.cpp -o build/remarkable-ai/remarkable-ai $(pkg-config --cflags --libs Qt6Core)
$STRIP --strip-unneeded build/remarkable-ai/remarkable-ai
python3 - <<'PY'
from pathlib import Path
import struct
b=Path('build/remarkable-ai/remarkable-ai').read_bytes()
assert b[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',b,18)[0]==183
assert b'libQt6Network' not in b
print('PASS: AArch64 CLI, QtCore only')
PY
