#!/bin/sh
# Read-only metadata inventory: no notebook access, screenshots, config or keys.
# Usage on tablet: sh probe-device.sh
# Fixtures only: sh probe-device.sh --root DIR --architecture ARCH
set -eu
root=
architecture=
while [ "$#" -gt 0 ]; do
    case "$1" in
        --root) [ "$#" -ge 2 ] || exit 2; root=$2; shift 2 ;;
        --architecture) [ "$#" -ge 2 ] || exit 2; architecture=$2; shift 2 ;;
        *) echo 'Usage: probe-device.sh [--root fixture --architecture arch]' >&2; exit 2 ;;
    esac
done
if [ -n "$architecture" ] && [ -z "$root" ]; then
    echo '--architecture is only allowed with a fixture root' >&2; exit 2
fi
[ -n "$architecture" ] || architecture=$(uname -m)
field() {
    printf '%s\t' "$1"
    printf '%s' "$2" | LC_ALL=C tr -cd 'A-Za-z0-9 ._:+/=-'
    printf '\n'
}
read_field() {
    # Never print complete files: only explicitly permitted fields.
    file=$1 key=$2
    if [ -r "$file" ]; then
        sed -n "s/^$key=//p" "$file" | head -n 1
    fi
}
field schema 1
field architecture "$architecture"
machine=
if [ -r "$root/sys/devices/soc0/machine" ]; then
    machine=$(head -n 1 "$root/sys/devices/soc0/machine")
fi
field machine "$machine"
case "$machine" in
    *[Cc][Hh][Ii][Aa][Pp][Pp][Aa]*) target=chiappa ;;
    *[Ff][Ee][Rr][Rr][Aa][Rr][Ii]*) target=ferrari ;;
    *'reMarkable 2'*) target=rm2 ;;
    *) target=unknown ;;
esac
field detected_target "$target"
release=$(read_field "$root/usr/share/remarkable/update.conf" REMARKABLE_RELEASE_VERSION)
if [ -z "$release" ]; then release=$(read_field "$root/etc/os-release" IMG_VERSION); fi
field software_release "$release"
field os_version "$(read_field "$root/etc/os-release" VERSION_ID)"
# Public executable/library hashes, not strings or contents. Do not execute xochitl.
for relative in usr/bin/xochitl home/root/xovi/xovi.so home/root/xovi/extensions.d/rm-shot.so home/root/xovi/extensions.d/framebuffer-spy.so home/root/xovi/extensions.d/xovi-message-broker.so home/root/xovi/extensions.d/qt-resource-rebuilder.so; do
    path="$root/$relative"
    if [ -f "$path" ]; then
        if command -v sha256sum >/dev/null 2>&1; then
            digest=$(sha256sum "$path")
            field "$relative.sha256" "${digest%% *}"
        else
            field "$relative" present-hash-unavailable
        fi
    else
        field "$relative" missing
    fi
done
# File names of Qt libraries identify major/minor ABI without loading a plugin.
for path in "$root"/usr/lib/libQt6Core.so.* "$root"/usr/lib/libQt5Core.so.*; do
    [ -e "$path" ] || continue
    field qt_core_library "${path##*/}"
done
field runtime_insertion_verified no
field runtime_geometry_verified no
