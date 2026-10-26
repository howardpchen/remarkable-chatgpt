#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
target=${1:-rm2}
case "$target" in
    rm2) xovi_arch=armv7 ;;
    ferrari|chiappa) xovi_arch=aarch64 ;;
    *) echo 'Usage: ./build.sh [rm2|ferrari|chiappa]' >&2; exit 2 ;;
esac
: "${SDK_ENV:?Set SDK_ENV to the matching official product/firmware SDK environment file}"
: "${SDK_PRODUCT:?Set SDK_PRODUCT to rm2, ferrari or chiappa matching the installed SDK}"
: "${SDK_RELEASE:?Set SDK_RELEASE to the tablet software release matching this SDK}"
: "${XOVI_REPO:?Set XOVI_REPO to the xovi source checkout}"
[[ "$SDK_PRODUCT" == "$target" ]] || { echo 'SDK_PRODUCT does not match requested target' >&2; exit 1; }
[[ "$SDK_RELEASE" =~ ^[0-9]+(\.[0-9]+){2,3}$ ]] || { echo 'SDK_RELEASE must be a software release number' >&2; exit 1; }
SDK_ENV=$(realpath "$SDK_ENV")
XOVI_REPO=$(realpath "$XOVI_REPO")
set +u
source "$SDK_ENV"
set -u
: "${CXX:?SDK did not configure CXX}"
: "${STRIP:?SDK did not configure STRIP}"
# Yocto CXX/STRIP include compiler flags; intentionally split the SDK-provided words.
compiler_arch=$($CXX -dumpmachine)
case "$target:$compiler_arch" in
    rm2:arm*) ;;
    ferrari:aarch64*|chiappa:aarch64*) ;;
    *) echo "SDK compiler architecture does not match $target: $compiler_arch" >&2; exit 1 ;;
esac
repo=$PWD
build_dir="$repo/build/$target"
out_dir="$repo/bin/$target"
mkdir -p "$build_dir" "$out_dir"
# Generate architecture-specific glue away from tracked source and other targets.
python3 "$XOVI_REPO/util/xovigen.py" -a "$xovi_arch" -o "$build_dir/xovi.cpp" -H "$build_dir/xovi.h" ink-diary.xovi
test -s "$build_dir/xovi.cpp" && test -s "$build_dir/xovi.h"
cat > "$build_dir/ink-diary.pro" <<'PRO'
TEMPLATE = lib
TARGET = ink-diary
CONFIG += shared plugin no_plugin_name_prefix c++17
QT += core gui qml quick svg network
SOURCES += $$quote($$(INK_DIARY_SOURCE)/src/diary.cpp) $$quote($$PWD/xovi.cpp)
INCLUDEPATH += $$quote($$PWD)
QMAKE_CXXFLAGS += -fPIC
QMAKE_LFLAGS += -Wl,-Bsymbolic -Wl,--version-script=$$quote($$(INK_DIARY_SOURCE)/ink-diary.map)
PRE_TARGETDEPS += $$quote($$(INK_DIARY_SOURCE)/ink-diary.map)
PRO
export INK_DIARY_SOURCE="$repo"
(
    cd "$build_dir"
    qmake ink-diary.pro CONFIG+=release
    make -j"${JOBS:-4}"
)
cp "$build_dir/ink-diary.so" "$out_dir/ink-diary.so"
$STRIP --strip-unneeded "$out_dir/ink-diary.so"
$CXX -std=c++17 -fPIC src/config-model.cpp -o "$out_dir/config-model" $(pkg-config --cflags --libs Qt6Core)
$STRIP --strip-unneeded "$out_dir/config-model"
python3 - "$target" "$SDK_RELEASE" "$compiler_arch" "$SDK_ENV" "$out_dir" <<'PY'
import hashlib, json, pathlib, sys
product, release, compiler, environment, output = sys.argv[1:]
folder = pathlib.Path(output)
# qmake can select a different compiler than CXX: verify actual output, not
# merely the SDK's declared triplet, before publishing build metadata.
expected_class, expected_machine = (1, 40) if product == "rm2" else (2, 183)
for name in ("ink-diary.so", "config-model"):
    header = (folder / name).read_bytes()[:20]
    if (len(header) != 20 or header[:4] != b"\x7fELF" or
            header[4] != expected_class or header[5] != 1 or
            int.from_bytes(header[18:20], "little") != expected_machine):
        raise SystemExit(f"Output is not the expected {product} ELF: {name}")
metadata = {"schema": 1, "product": product, "software_release": release,
            "compiler_triplet": compiler, "runtime_validated": False,
            "sdk_environment_sha256": hashlib.sha256(pathlib.Path(environment).read_bytes()).hexdigest(),
            "artifacts": {name: hashlib.sha256((folder / name).read_bytes()).hexdigest()
                          for name in ("ink-diary.so", "config-model")}}
(folder / "build-metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
# A minimal non-executable sidecar lets the tablet's POSIX installer validate
# release and checksums without requiring Python or evaluating shell content.
fields = {"product": product, "software_release": release,
          "compiler_triplet": compiler,
          **{name + ".sha256": digest for name, digest in metadata["artifacts"].items()}}
(folder / "build-manifest.tsv").write_text("".join(key + "\t" + value + "\n" for key, value in fields.items()))
PY
printf 'Built %s for declared software release %s in bin/%s (not runtime validated).\n' "$target" "$SDK_RELEASE" "$target"
