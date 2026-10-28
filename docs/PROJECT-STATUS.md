# Current project checkpoint

## Move toolbar auto-hide (local; not deployed)

Owner-requested toolbar hiding through inference and prior-state restoration is
implemented locally. Both focused capture-mode suites pass 40 request scenarios;
actual-widget/strict-pose checks, inherited live tests and exact Move SDK build
pass. No device change or provider request occurred. Focused SO/QMD/profile
artifacts and next deployment checks are in [Move toolbar auto-hide](MOVE-TOOLBAR-AUTOHIDE.md).
The installed Move still has the Grid-only fix; this update requires fresh
deployment authority. Unrelated Paper Pro work below is preserved.

Updated: 2026-10-27. OpenClaw registered project: **reMarkable AI — Vellum Mod**
(`remarkable-ai-vellum-mod`). Select it in **New session → Folder/Place** to resume.
Registration reuses this repository and does not move files, publish changes or
attach unrelated sessions. Start with `AGENTS.md`, `PROJECT.md` and `ROADMAP.md`.

## Active work: full-size Paper Pro portrait

Owner requested Paper Pro portrait work on 2026-10-27. Read-only inventory now
confirms Ferrari / software 3.28.0.172 / internal OS 5.8.203 / Qt 6.10.3.
The exact official Ferrari SDK is installed locally. Isolated portrait candidate
geometry passes a synthetic crop/ink/render/scaled-placement pipeline and 37
rejection checks; it also cross-compiles with the Ferrari SDK. A separate exact
Ferrari diagnostic target cross-builds, with no runtime/install promotion.
Root host regressions, the rebuilt Move session runtime/UI suite and both
mutually exclusive diagnostic policies pass. Exact-Ferrari-SDK diagnostic and
broker ABI 0.2.0 artifacts are staged in ignored
`build/paper-pro-portrait-stage/` with hashes/source provenance.

No tablet files, keys, usage, notebooks or services were modified, and no app
restart or provider request ran. The device has no broker endpoints or observed
loaded XOVI modules in its inspected current xochitl process. Actual Paper Pro
logical geometry, native resources, capture and insertion remain unvalidated.
Next: review/authorize temporary broker + diagnostic loading before measuring
the live notebook. See [Paper Pro portrait checkpoint](PAPER-PRO-PORTRAIT.md).
This work does not change the accepted Move beta or its packaging milestone below.

## Implemented locally

- `tools/remarkable-ai/`: target/host CLI with `key status [--json]`, hidden `key
  set` and `key remove`. Reuses existing private configuration validation/atomic
  writes, adds serialized writers, preserves unrelated settings and all usage.
- Status separates application-local allowance from unknown OpenAI account
  balance; reports charged/unsettled amounts, next reservation/affordability,
  no independent count cap, and masked SHA-256 credential identity.
- Native module records allowlisted, timestamped provider metadata with the
  credential fingerprint. CLI marks it historical and refuses to associate old
  errors with a different key. No raw provider messages, keys or notebook text
  enter metadata. Status does not call OpenAI. Replacement never refills budget.
- `packaging/vellum/`: VELBUILD, generic artifact staging, pinned builder, signed
  APK build and isolated real APK solver/lifecycle tests. Package pins Move and
  tested firmware; no restart/funding/private-state lifecycle hooks.
- Repository instructions, project context, roadmap and this checkpoint persist
  independently of conversation history.

## Local artifacts and verification

Local artifact: `build/vellum-dist/aarch64/remarkable-ai-move-0.1.0_beta1-r0.apk`.
SHA-256: `227ffdea01d27237e5d9b536ed6e179e0634b54d3d1394c234ad4c383dc2c440`.
A test-only r1 APK exercises upgrades. `build/vellum-dist/SHA256SUMS` records the
bundle hashes. Signing private key stays in ignored `build/vellum-signing/`.

CLI: 20 synthetic status cases, real hidden-TTY set/remove and preservation;
25 existing private-config/TTY cases; provider metadata allowlisting/symlink test;
exact SDK AArch64 compile for CLI and module; CLI has no QtNetwork linkage.
All 37 dollar-budget/single-capture request scenarios and the rebuilt UI suite
pass. The separate live suite and all 21 synthetic transport cases pass, including
provider-observer delivery/cancellation checks. Private workspace report retains
logs and final package lifecycle results.
Signed APK install/upgrade/removal preserves synthetic configuration, calibration,
policy, provider metadata and charges byte-for-byte. Wrong OS/model are rejected.
Payload inspection verifies the exact built binaries, six generic data files,
no private state and no lifecycle scripts.

## Live state is separate

No live device/key/budget modifications or API requests were made for this
packaging/CLI milestone. The earlier narrow UI update is deployed, but its live
expanded-panel visual check still awaits reopening Test and tapping AI, not Start.
Current registered project does not claim the new package has been tablet-tested.
Private device checkpoints/artifacts remain outside this repository under the
workspace's `reports/remarkable-ui-tidy-20261027/` and
`reports/remarkable-vellum-cli-20261027/`. Do not copy their contents into Git.

## Next authorized/resumable work

Inspect this checkpoint and Git state first. Do not assume HEAD was pushed.
The local implementation and packaging checks passed.
Future device migration, package signing-key trust, installation or publication
require scoped owner authorization. First-time automatic calibration remains a
future feature; this package explicitly requires the separately reviewed schema-3
profile and budget setup. Any changed SO/QMD needs re-reviewed profile fingerprints
without resetting calibration or usage. The historical shutdown issue is unresolved.
