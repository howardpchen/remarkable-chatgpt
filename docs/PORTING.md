# Paper Pro / Paper Pro Move port preparation

This branch prepares **shared source and separate product builds**. It does not
claim either Paper Pro target works. `compatibility/targets.json` is the
compatibility inventory: Paper Pro and Move intentionally have **empty install
release lists**. Compiling is not runtime validation. Existing rM2 3.27.3.0
installer acceptance is retained as a prototype baseline, not newly certified.

## Builds

Use a Linux host and the official SDK matching **host architecture, exact product
and software release**. The SDK's internal OS version is not necessarily the
software version shown in Settings. Verify the official download mapping rather
than inferring it from a file name. These preparation scripts do not download
SDKs, access devices or install anything on a tablet.

```sh
export SDK_ENV=/path/to/official-sdk/environment-setup-...
export SDK_PRODUCT=ferrari  # rm2, ferrari (Pro), chiappa (Move)
export SDK_RELEASE=3.27.3.0 # example only; use the actual SDK/software mapping
export XOVI_REPO=/path/to/xovi
./build.sh "$SDK_PRODUCT"
```

`SDK_PRODUCT` and `SDK_RELEASE` are explicit operator declarations. The build
checks the compiler architecture but cannot prove the declared product or
firmware mapping from architecture alone. Never substitute a ferrari SDK for
chiappa without ABI investigation. The script runs the SDK compiler and qmake,
not the host compiler, generates XOVI glue per architecture, and uses isolated
`build/<product>` and `bin/<product>` outputs. It writes artifact checksums,
compiler triplet, declared product/release and SDK environment-file checksum to
`bin/<product>/build-metadata.json`, plus a non-executable TSV sidecar for the
tablet installer. This is build provenance, **not a signed
attestation or installation authorization**. No SDK path or environment contents
are recorded. Existing root-level generated glue and bundled binaries are not
replaced. Parallel builds for different products do not share generated files;
parallel builds of the same product are not supported.

The original `SDK_ENV`/`XOVI_REPO` invocation now additionally requires
`SDK_PRODUCT` and `SDK_RELEASE`; this deliberate change prevents ambiguous SDK
selection. The installer prefers `bin/rm2` when present, otherwise its original
bundled `bin` layout. An existing but incomplete or wrong-release target build
blocks installation without falling back to stale binaries. Target checksums and
ARM32 ELF headers are checked before dependencies change. `install.sh ferrari` and `install.sh chiappa` refuse before
any package or file modification. Do not bypass these gates to claim support.
The installer retains the original exact rM2 architecture/firmware checks.

## Read-only evidence collection

Run `sh tools/probe-device.sh` on a device, or stream it through an already
configured SSH connection:

```sh
ssh root@10.11.99.1 sh -s < tools/probe-device.sh > device-probe.tsv
```

The probe reads only model/OS metadata, public executable/dependency hashes and
Qt library names. It does not inspect notebooks, account settings, API keys,
network configuration, logs or framebuffer contents; does not capture a screen;
and does not install, start, stop or modify anything. Missing fields mean
unknown, not support. Keep the output with the validation record. Probe success
cannot establish the private notebook insertion API or coordinate mapping.
No SSH or tablet connection is initiated by the build/test scripts.

The offline compatibility checker accepts only recorded install releases:

```sh
python3 tools/check-compatibility.py ferrari ACTUAL_RELEASE --architecture aarch64
```

It exits nonzero for unknown/unvalidated firmware. It cannot authorize a device
operation; it reports the manifest only. The shell installer remains restricted
to the original rM2 gate even if this JSON is changed. A future promotion must
update runtime, installer and manifest gates together after hardware evidence.

## Earliest hardware go/no-go

For **each exact product + firmware**:

1. Collect metadata and dependency evidence; check actual SDK/Qt ABI alignment.
2. Load a minimal plugin under supported tethered XOVI; verify QML resource
   selectors and message broker without making a network request.
3. In a disposable notebook, calibrate screenshot physical pixels, Qt logical
   viewport, device-pixel ratio, scene coordinates, zoom, pan and rotation. The
   physical widths 1620 (Pro) and 954 (Move) are not logical layout dimensions.
4. Insert a local PNG through the private notebook API; verify page identity,
   placement, undo/redo, reopen persistence, and page-switch/cancellation refusal.
5. Exercise narrow Move layout, portrait with explicit rejection of unsupported orientations, long output, screen edges,
   color capture, memory use, and stock reboot/recovery. Preserve rM2 regression.

Go only after both devices pass independently. An absent/private insertion API
or inconsistent mapping blocks integration, not something a build flag fixes.
Promote exact releases only with recorded evidence, dependency versions,
checksums, and verified rollback; never broadly allow all AArch64 or all 3.x.

Developer mode is a user-managed prerequisite. Official documentation says
enabling it factory-resets the device and changes secure-boot protections. This
port does not enable it, alter security settings, access a tablet, or install a
persistent launcher. Preserve tethered XOVI/stock boot recovery; do not modify
Xochitl boot services to auto-start XOVI on encrypted devices.

## Grounding

- Official product-specific SDK guidance and code names:
  <https://developer.remarkable.com/documentation/sdk>
- Developer mode and reset consequences:
  <https://developer.remarkable.com/documentation/developer-mode>
- Xochitl has no version compatibility guarantee:
  <https://developer.remarkable.com/documentation/xochitl>
- XOVI explicitly designates AArch64 for rMPP/rMPPM:
  <https://github.com/asivery/rm-xovi-extensions/blob/master/README.MD>
- Firmware-sensitive QMD patches and hash-table rebuilds:
  <https://github.com/asivery/rm-xovi-extensions/blob/master/qt-resource-rebuilder/README.MD>
- rm-shot upstream advertises both devices and implements chiappa/ferrari capture:
  <https://github.com/rmitchellscott/rm-shot>
- Vellum models rmpp/rmppm and exact OS dependency gates:
  <https://github.com/vellum-dev/vellum>

## SDK discovery for observed Move firmware (not an installation)

The official [SDK links table](https://developer.remarkable.com/links) lists
chiappa software **3.28.0.172**, internal OS **5.8.203**. Both URLs returned HTTP
200 to a metadata-only HEAD check on 2026-10-25:

- x86_64 host (516,458,637 bytes):
  <https://storage.googleapis.com/remarkable-codex-toolchain/3.28.0.172/remarkable-production-image-5.8.203-chiappa-public-x86_64-toolchain.sh>
- AArch64 host (495,466,546 bytes):
  <https://storage.googleapis.com/remarkable-codex-toolchain/3.28.0.172/remarkable-production-image-5.8.203-chiappa-public-aarch64-toolchain.sh>

Host architecture is the architecture named before `-toolchain.sh`, not the
product's target architecture. The SDK guide's x86_64-only statement is older
than this links table; choose the published SDK matching the actual build host.
These SDKs were discovered, not downloaded or executed by this preparation lane.
A checksum, installed compiler and Qt ABI still need validation. The matching
software version is not evidence that the plugin or its QMD is compatible.
