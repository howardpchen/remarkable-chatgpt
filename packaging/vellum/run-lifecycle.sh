#!/usr/bin/env bash
# Host wrapper: isolated synthetic lifecycle, never a tablet operation.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
device=${1:?chiappa or ferrari}; output=${2:?Fresh disposable output directory}
case "$device" in
 chiappa) package=remarkable-ai-move ;;
 ferrari) package=remarkable-ai-ferrari ;;
 *) exit 2 ;;
esac
: "${TEST_VERSION:=0.2.0_beta1}"
: "${VELLUM_BUILDER_IMAGE:=ghcr.io/eeems/vbuild-builder@sha256:eb6117c636104b06ef9cd9036fa605620bc39ba06183d8f3e52c1caff22578d3}"
mkdir -m 700 "$output"
output=$(realpath "$output")
docker run --rm --network=none \
 -e TASK_UID="$(id -u)" -e TASK_GID="$(id -g)" -e TEST_DEVICE="$device" \
 -e TEST_APK="/packages/aarch64/$package-$TEST_VERSION-r0.apk" \
 -e TEST_UPGRADE_APK="/packages/aarch64/$package-$TEST_VERSION-r1.apk" \
 -e TEST_PUBLIC_KEY=/root/.abuild/remarkable-ai-local.rsa.pub \
 --mount "type=bind,src=$output,dst=/test" \
 --mount "type=bind,src=$PWD/build/vellum-dist,dst=/packages,readonly" \
 --mount "type=bind,src=$PWD/build/vellum-signing,dst=/root/.abuild,readonly" \
 --mount "type=bind,src=$PWD/packaging/vellum/test-lifecycle.sh,dst=/lifecycle.sh,readonly" \
 "$VELLUM_BUILDER_IMAGE" sh /lifecycle.sh
