# Local Vellum packages — native-bounds beta2

Separate experimental signed APKs, not Vellum catalog entries:

| Device | Package | CLI | Model dependency |
|---|---|---|---|
| Chiappa / Move | remarkable-ai-move | remarkable-ai | rmppm |
| Ferrari / Paper Pro | remarkable-ai-ferrari | remarkable-ai-ferrari | rmpp |

Both pin `remarkable-os=3.28.0.172-r0` and use exact product AArch64 / Qt6.10.3
SDK builds. Do not loosen firmware/model dependency guards or exchange binaries.
Both implement template-independent native-bounds placement. Calibration and
native-pose/resource guards remain product-specific; see
[parity and hardware evidence](../../docs/VELLUM-PARITY.md).

## Build

Use the maintained upstream vbuild/abuild pipeline, with exact private firmware
hashtab, qmldiff and (Ferrari only) reviewed original DocumentView resource.
These private inputs and licensed fonts are never packaged or published.

```sh
# Chiappa runtime and offline CLI; use the exact installed Chiappa SDK.
SDK_ENV=/path/to/chiappa/environment-setup-cortexa55-remarkable-linux \
  XOVI_REPO=/path/to/xovi SINGLE_CAPTURE=1 DOLLAR_BUDGET=1 \
  bash tools/move-session/build.sh
SDK_ENV=/path/to/chiappa/environment-setup-cortexa55-remarkable-linux \
  bash tools/remarkable-ai/build.sh

# Ferrari runtime and CLI; never use the Chiappa SDK.
SDK_ENV=/path/to/ferrari/environment-setup-cortexa53-crypto-remarkable-linux \
  SDK_PRODUCT=ferrari SDK_RELEASE=3.28.0.172 XOVI_REPO=/path/to/xovi \
  bash tools/paper-pro-portrait/session/build.sh

python3 packaging/vellum/prepare.py --device chiappa --release 0 \
  --qmldiff /path/to/qmldiff --hashtab /private/chiappa/hashtab \
  --output build/package-chiappa-r0
python3 packaging/vellum/prepare.py --device ferrari --release 0 \
  --qmldiff /path/to/qmldiff --hashtab /private/ferrari/hashtab \
  --document-view /private/ferrari/DocumentView.qml \
  --output build/package-ferrari-r0
VBUILD=/path/to/vbuild PACKAGE_INPUT="$PWD/build/package-chiappa-r0" \
  bash packaging/vellum/build-package.sh
```

Repeat the last command for Ferrari. Inputs must be fresh directories; select
nondefault exact-SDK artifacts with `--module` / `--cli`. `prepare.py` validates
AArch64 ELF and product-specific state identity, stages generic files and fixed
SHA-512 checksums, and creates firmware-hashed QMD. Ferrari's exact-resource
application is locally checked without publishing the original resource.

For isolated worktree validation, `VELLUM_DIST_DIR` and `VELLUM_SIGNING_DIR`
select existing local output/signing directories for both build and lifecycle
scripts; no signing key need be copied into the worktree.

Outputs are `build/vellum-dist/aarch64/remarkable-ai-{move,ferrari}-0.2.0_beta2-r0.apk`.
Generate release1 counterparts with identical code to test upgrade. Never reuse an
already distributed release number for changed payload. This local builder has
no network and mounts only scoped inputs/output/signing state. A private signing
key remains under ignored `build/vellum-signing/` (0700 directory /0600 key); only
the public key may be delivered. No trust-store bypass or automatic key install.

## Audit and lifecycle

```sh
python3 packaging/vellum/audit-package.py --device chiappa \
  --input build/package-chiappa-r0 \
  build/vellum-dist/aarch64/remarkable-ai-move-0.2.0_beta2-r0.apk
TEST_VERSION=0.2.0_beta2 bash packaging/vellum/run-lifecycle.sh chiappa build/lifecycle-chiappa
TEST_VERSION=0.2.0_beta2 bash packaging/vellum/run-lifecycle.sh ferrari build/lifecycle-ferrari
```

Lifecycle checks use signed APKs, isolated synthetic state and the real APK
solver. Verify wrong-model/firmware refusal, install/upgrade/remove preservation,
exact generic payload and absence of links/hooks/private data. An APK solver
check is not runtime resource/ABI verification or live migration success.

## Installation boundary

No lifecycle hooks: no restart, API request, automatic budget or private-state
writes. No credentials, calibration or fonts bundled. Separate device deployment
requires scoped owner authority, library readiness, rollback and independent
loaded-resource/state verification; package preparation or source push is not
that authority. See [Move setup](SETUP.md) and [Ferrari setup](SETUP-FERRARI.md).
