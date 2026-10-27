#!/bin/sh
# Runs ONLY inside a disposable Linux container, with synthetic dependency packages.
set -eu
: "${TEST_APK:?}"
: "${TEST_UPGRADE_APK:?}"
: "${TEST_PUBLIC_KEY:?}"
mkdir -p /test/keys
cp "$TEST_PUBLIC_KEY" /test/keys/
cp "$TEST_PUBLIC_KEY" /etc/apk/keys/
cat > /test/make-fixture.sh <<'SCRIPT'
#!/bin/sh
set -eu
label=$1; model=$2; os=$3
work=/test/fixtures/$label
mkdir -p "$work"
cat > "$work/APKBUILD" <<RECIPE
pkgname=remarkable-ai-test-platform-$label
pkgver=1
pkgrel=0
arch="noarch"
pkgdesc="Synthetic dependencies for isolated tests only"
url="https://example.invalid"
license="MIT"
options="!check !fhs"
provides="$model=1 remarkable-os=$os xovi=0.3.3 xovi-message-broker=19.0.0 qt-resource-rebuilder=19.0.0 rm-shot=1.2.0-r0 framebuffer-spy=19.0.0"
package() { mkdir -p "\$pkgdir"; }
RECIPE
REPODEST=/test/dependencies abuild -C "$work" -F -d -r all > /test/fixture-$label.log 2>&1
SCRIPT
chmod 755 /test/make-fixture.sh
/test/make-fixture.sh right rmppm 3.28.0.172
/test/make-fixture.sh wrongos rmppm 3.29.0.0
/test/make-fixture.sh wrongmodel rmpp 3.28.0.172
# Add real signed package to synthetic platform; use actual APK solver/file ownership.
for label in right wrongos wrongmodel; do
    root=/test/root-$label
    mkdir -p "$root"
    dep=$(find /test/dependencies -name "remarkable-ai-test-platform-$label-*.apk" | head -n 1)
    apk --root "$root" --arch aarch64 --initdb --keys-dir /test/keys --no-network --no-scripts add "$dep" > /test/$label-dependencies.log 2>&1
    if [ "$label" != right ]; then
        if apk --root "$root" --arch aarch64 --keys-dir /test/keys --no-network --no-scripts add "$TEST_APK" > /test/$label-rejection.log 2>&1; then
            echo "ERROR: incompatible platform accepted: $label"; exit 1
        fi
        test ! -e "$root/home/root/.vellum/bin/remarkable-ai"
        continue
    fi
    private="$root/home/root/xovi/move-session-small-templates-v4"
    config="$root/home/root/.config/ink-diary"
    mkdir -p "$private/1" "$config"
    chmod 700 "$private" "$private/1" "$config"
    printf '%s\n' '{"api_key":"synthetic-not-real","model":"gpt-6-luna"}' > "$config/config.json"
    printf '%s\n' '{"synthetic":"private calibration"}' > "$private/device-profile.json"
    printf '%s\n' '{"synthetic":"already funded allowance"}' > "$private/budget-policy.json"
    printf '%s\n' '254210000' > "$private/1/BUDGET_COST"
    printf '%s\n' '{"schema":1,"synthetic":"last observed"}' > "$private/provider-status.json"
    chmod 600 "$config/config.json" "$private"/*.json "$private/1/BUDGET_COST"
    find "$private" "$config" -type f -exec sha256sum '{}' \; | sort > /test/private-before
    # No hooks should be present. Scripts remain disabled as this is not a target OS.
    apk --root "$root" --arch aarch64 --keys-dir /test/keys --no-network --no-scripts add "$TEST_APK" > /test/install.log 2>&1
    test -x "$root/home/root/.vellum/bin/remarkable-ai"
    test -s "$root/home/root/xovi/extensions.d/ink-diary-move-live.so"
    test -s "$root/home/root/xovi/exthome/qt-resource-rebuilder/ink-diary-ui-probe.qmd"
    apk --root "$root" --arch aarch64 --keys-dir /test/keys --no-network --no-scripts add "$TEST_UPGRADE_APK" > /test/upgrade.log 2>&1
    apk --root "$root" --arch aarch64 --keys-dir /test/keys --no-network --no-scripts del remarkable-ai-move > /test/remove.log 2>&1
    test ! -e "$root/home/root/.vellum/bin/remarkable-ai"
    test ! -e "$root/home/root/xovi/extensions.d/ink-diary-move-live.so"
    test ! -e "$root/home/root/xovi/exthome/qt-resource-rebuilder/ink-diary-ui-probe.qmd"
    find "$private" "$config" -type f -exec sha256sum '{}' \; | sort > /test/private-after
    cmp /test/private-before /test/private-after
    test "$(stat -c %a "$config/config.json")" = 600
    test "$(stat -c %a "$private")" = 700
 done
printf '%s\n' 'PASS signed APK install/upgrade/removal: private state byte-identical; wrong OS/model rejected'
