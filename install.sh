#!/bin/sh
set -eu
cd -- "$(dirname -- "$0")"
target=${1:-rm2}
case "$target" in
    rm2) ;;
    ferrari|chiappa) echo 'Paper Pro / Move installation is blocked pending per-device firmware validation; see docs/PORTING.md' >&2; exit 1 ;;
    *) echo 'Usage: ./install.sh [rm2|ferrari|chiappa]' >&2; exit 2 ;;
esac
[ "$(id -u)" = 0 ] || { echo 'Run this on the tablet as root'; exit 1; }
grep -Fqx 'REMARKABLE_RELEASE_VERSION=3.27.3.0' /usr/share/remarkable/update.conf || { echo 'Requires OS 3.27.3.0'; exit 1; }
[ "$(uname -m)" = armv7l ] || { echo 'Requires reMarkable 2'; exit 1; }
# Prefer the isolated target build. Retain the original bundled rM2 layout.
artifacts=bin
if [ -d bin/rm2 ]; then artifacts=bin/rm2; fi
[ -f "$artifacts/config-model" ] || { echo 'Missing configuration helper'; exit 1; }
[ -f "$artifacts/ink-diary.so" ] || { echo 'Build the plugin first'; exit 1; }
# Check every binary before any dependency or filesystem mutation. Both the
# plugin and helper must be 32-bit little-endian ARM, not host/AArch64 binaries.
for binary in "$artifacts/ink-diary.so" "$artifacts/config-model"; do
    ident=$(od -An -tx1 -N6 "$binary" | tr -d ' \n')
    machine=$(od -An -tx1 -j18 -N2 "$binary" | tr -d ' \n')
    [ "$ident" = 7f454c460101 ] && [ "$machine" = 2800 ] || {
        echo "Not a little-endian ARM32 ELF: $binary" >&2; exit 1;
    }
done
if [ "$artifacts" = bin/rm2 ]; then
    [ -f "$artifacts/build-metadata.json" ] && [ -f "$artifacts/build-manifest.tsv" ] || {
        echo 'Target build is incomplete: missing build metadata' >&2; exit 1;
    }
    value() { sed -n "s/^$1$(printf '\t')//p" "$artifacts/build-manifest.tsv"; }
    [ "$(value product)" = rm2 ] && [ "$(value software_release)" = 3.27.3.0 ] || {
        echo 'Target build metadata does not match rM2 software 3.27.3.0' >&2; exit 1;
    }
    for name in ink-diary.so config-model; do
        expected=$(value "$name.sha256")
        [ "${#expected}" -eq 64 ] || { echo 'Invalid artifact checksum metadata' >&2; exit 1; }
        actual=$(sha256sum "$artifacts/$name")
        [ "${actual%% *}" = "$expected" ] || { echo "Artifact checksum mismatch: $name" >&2; exit 1; }
    done
fi
/home/root/.vellum/bin/vellum add 'rm-shot>=1.2.0' xovi-message-broker framebuffer-spy qt-resource-rebuilder
base=/home/root/.config/ink-diary
mkdir -p "$base"
chmod 700 "$base"
cp icons/chatgpt.svg "$base/chatgpt.svg"
chmod 644 "$base/chatgpt.svg"
cp fonts/DejaVuSansMono.ttf "$base/DejaVuSansMono.ttf"
chmod 644 "$base/DejaVuSansMono.ttf"
cp fonts/Caveat.ttf "$base/Caveat.ttf"
chmod 644 "$base/Caveat.ttf"
cp "$artifacts/config-model" "$base/config-model"
chmod 700 "$base/config-model"
cp configure.sh "$base/configure.sh"
chmod 700 "$base/configure.sh"
cp "$artifacts/ink-diary.so" /home/root/xovi/extensions.d/ink-diary.so.new
chmod 755 /home/root/xovi/extensions.d/ink-diary.so.new
mv /home/root/xovi/extensions.d/ink-diary.so.new /home/root/xovi/extensions.d/ink-diary.so
cp inkDiary.qmd /home/root/xovi/exthome/qt-resource-rebuilder/inkDiary.qmd
chmod 644 /home/root/xovi/exthome/qt-resource-rebuilder/inkDiary.qmd
printf 'Installed. Configure with: sh /home/root/.config/ink-diary/configure.sh\nRestart xochitl through /home/root/xovi/start to load the extension.\n'
