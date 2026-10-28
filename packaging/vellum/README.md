# Local Vellum package

This directory implements a **local experimental Move package**, not a published
catalog entry. `VELBUILD` follows upstream Vellum conventions; `prepare.py`
stages generic target binaries and firmware-hashed QMD with fixed SHA-512 source
checksums. `build-package.sh` uses upstream **vbuild** to generate APKBUILD and
Alpine **abuild** inside a pinned disposable builder to produce a signed APK.
It does not invent a package format or install a package manager on the host.

## Build

Prerequisites: exact Chiappa OS 3.28.0.172 SDK / Qt 6.10.3, XOVI source,
firmware hashtab, qmldiff, upstream vbuild 0.0.36, Docker and OpenSSL.
Existing privately obtained firmware resources are build inputs, never release
assets. Keep SDKs and extracted resources outside the source repository.

```sh
SDK_ENV=/path/to/sdk/environment-setup-cortexa55-remarkable-linux \
  bash tools/remarkable-ai/build.sh
SDK_ENV=/path/to/sdk/environment-setup-cortexa55-remarkable-linux \
  XOVI_REPO=/path/to/xovi SINGLE_CAPTURE=1 DOLLAR_BUDGET=1 \
  bash tools/move-session/build.sh
python3 packaging/vellum/prepare.py \
  --qmldiff /path/to/qmldiff --hashtab /path/to/firmware/hashtab \
  --output build/vellum-local/remarkable-ai-move
VBUILD=/path/to/vbuild PACKAGE_INPUT="$PWD/build/vellum-local/remarkable-ai-move" \
  bash packaging/vellum/build-package.sh
```

The output is `build/vellum-dist/aarch64/remarkable-ai-move-0.1.0_beta1-r0.apk`.
`build/vellum-signing/` contains a **private local development signing key**; keep
it local and never include it in releases. Only its `.pub` file is distributable.
The builder gets only scoped input/output/signing mounts, no Docker socket,
network, user configuration, credentials or notebook data. No host-wide build
configuration is changed. Review the APK and checksums before distribution.

## Test

`bash tools/remarkable-ai/test.sh` uses host Qt6 via pkg-config, temporary private
fixtures, a real pseudo-terminal and no network. It covers key operations, local
allowance/account-balance distinctions and sanitized historical provider errors.
`tools/move-session/test.sh` and `tools/move-live/test.sh` cover the integrated
mocked native/API paths. Target binaries must also compile with the exact SDK.

`test-lifecycle.sh` is a **container-only** signed APK lifecycle test. Supply
`TEST_APK`, `TEST_UPGRADE_APK`, and `TEST_PUBLIC_KEY`, mount a writable disposable
`/test` and the local signing directory at `/root/.abuild` (read-only), and run
using the same pinned builder image. The second APK must have a higher package
release (`pkgrel=1` in a copied prepared input). It creates synthetic dependency
packages for a matching platform, a wrong OS and a wrong model. The real APK
solver verifies signatures, rejects incompatible dependencies, and installs,
upgrades and removes package-owned files without altering synthetic private state.
No lifecycle hooks are present; tests disable script execution as additional
containment. This is not live tablet or live-provider verification.

Pass `-e TASK_UID="$(id -u)" -e TASK_GID="$(id -g)"` to the container. These
non-root host IDs are mandatory. `/test` must be a dedicated disposable mount
containing only synthetic outputs, not signing keys or real private state.
An exit trap returns root-owned entries on that mount to the host identity on
success, failure or handled termination, without following symlinks or changing
0700/0600 fixture modes. Forced container kills cannot run the trap; inspect
ownership before retaining interrupted outputs. Signing and input mounts remain
separate and read-only.

See [SETUP.md](SETUP.md) for private state, manual migration, calibration and
upgrade/removal requirements. Do not pass a private signing key to a tablet or
silently modify its trust store. An unsigned/untrusted-key bypass is not an
installation procedure for this project.
