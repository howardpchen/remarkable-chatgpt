#!/usr/bin/env bash
# Build a local APK with upstream vbuild-generated APKBUILD and Alpine abuild.
# Never touches a tablet, trust store, global build config or persistent service.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${VBUILD:?Path to upstream vbuild binary required}"
: "${PACKAGE_INPUT:?Fresh prepared package input directory required}"
: "${VELLUM_BUILDER_IMAGE:=ghcr.io/eeems/vbuild-builder@sha256:eb6117c636104b06ef9cd9036fa605620bc39ba06183d8f3e52c1caff22578d3}"
input=$(realpath "$PACKAGE_INPUT")
dist="$PWD/build/vellum-dist"
signing="$PWD/build/vellum-signing"
mkdir -p "$dist"
if [[ ! -e "$signing" ]]; then mkdir -m 700 "$signing"; fi
[[ ! -L "$signing" && $(stat -c %a "$signing") == 700 ]]
if [[ ! -e "$signing/remarkable-ai-local.rsa" ]]; then
    (umask 077; openssl genrsa -out "$signing/remarkable-ai-local.rsa" 2048)
    openssl rsa -in "$signing/remarkable-ai-local.rsa" -pubout -out "$signing/remarkable-ai-local.rsa.pub" 2>/dev/null
fi
[[ ! -L "$signing/remarkable-ai-local.rsa" && $(stat -c %a "$signing/remarkable-ai-local.rsa") == 600 ]]
if [[ ! -e "$signing/abuild.conf" ]]; then
    (umask 077; set -C; printf '%s\n' 'PACKAGER_PRIVKEY=/root/.abuild/remarkable-ai-local.rsa' > "$signing/abuild.conf")
fi
"$VBUILD" -C "$input" gen
docker run --rm --network=none \
    -e CARCH=aarch64 -e SOURCE_DATE_EPOCH=1793059200 -e REPODEST=/dist \
    -e TASK_UID="$(id -u)" -e TASK_GID="$(id -g)" \
    --mount "type=bind,src=$input,dst=/work" \
    --mount "type=bind,src=$dist,dst=/dist" \
    --mount "type=bind,src=$signing,dst=/root/.abuild,readonly" \
    "$VELLUM_BUILDER_IMAGE" sh -ec '
        cp /root/.abuild/remarkable-ai-local.rsa.pub /etc/apk/keys/
        abuild -C /work -d -F -r all
        chown -R "$TASK_UID:$TASK_GID" /work /dist
    '
# The public key is distributable; the private signing directory is not.
cp "$signing/remarkable-ai-local.rsa.pub" "$dist/remarkable-ai-local.rsa.pub"
