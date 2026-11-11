#!/usr/bin/env bash
set -euo pipefail
repo=$(cd -- "$(dirname -- "$0")/../../.." && pwd)
: "${SDK_ENV:?Exact Ferrari SDK required}"
: "${XOVI_REPO:?XOVI source required}"
: "${XOVI_EXTENSIONS_REPO:?Pinned xovi extensions source required}"
: "${RM_SHOT_REPO:?Pinned rm-shot source required}"
[[ "${SDK_PRODUCT:-}" == ferrari && "${SDK_RELEASE:-}" == 3.28.0.172 ]]
set +u
source "$SDK_ENV"
set -u
out="${PAPER_PRO_DEPENDENCY_BUILD_DIR:-$repo/build/paper-pro-session-ferrari/dependencies}"
: "${CFLAGS:?SDK optimization flags required}"
: "${LDFLAGS:?SDK linker flags required}"
mkdir -p "$out/framebuffer-spy/src" "$out/rm-shot/src"
cp ${XOVI_EXTENSIONS_REPO}/framebuffer-spy/src/main.c "$out/framebuffer-spy/src/"
cp ${XOVI_EXTENSIONS_REPO}/framebuffer-spy/framebuffer-spy.h "$out/framebuffer-spy/"
cp ${XOVI_EXTENSIONS_REPO}/util.h "$out/"
cp ${RM_SHOT_REPO}/src/{screenshot.c,stb_image_write.h,rm-shot.h} "$out/rm-shot/src/"
python3 ${XOVI_REPO}/util/xovigen.py -a aarch64 -o "$out/framebuffer-spy/xovi.c" -H "$out/framebuffer-spy/xovi.h" ${XOVI_EXTENSIONS_REPO}/framebuffer-spy/framebuffer-spy-rmpp.xovi
$CC $CFLAGS -D_GNU_SOURCE -fPIC -shared "$out/framebuffer-spy/src/main.c" "$out/framebuffer-spy/xovi.c" -o "$out/framebuffer-spy.so" $LDFLAGS
python3 ${XOVI_REPO}/util/xovigen.py -a aarch64 -o "$out/rm-shot/xovi.c" -H "$out/rm-shot/xovi.h" ${RM_SHOT_REPO}/rm-shot.xovi
$CC $CFLAGS -D_GNU_SOURCE -DVERSION='"1.2.0"' -fPIC -shared "$out/rm-shot/src/screenshot.c" "$out/rm-shot/xovi.c" -I"$out/rm-shot" -o "$out/rm-shot.so" -pthread -lm $LDFLAGS
$STRIP --strip-unneeded "$out/framebuffer-spy.so" "$out/rm-shot.so"
file "$out/framebuffer-spy.so" "$out/rm-shot.so"
