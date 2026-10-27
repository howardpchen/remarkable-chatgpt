# Paper Pro portrait port — current preparation

Updated 2026-10-27. This is an active port, **not working Paper Pro AI support**.
The Move personal beta and rM2 paths remain separate and unchanged.

## Observed device and local toolchain

Read-only SSH inventory independently observed `reMarkable Ferrari`, AArch64,
software `3.28.0.172`, internal OS `5.8.203`, and Qt `6.10.3`. xochitl was active
with zero restarts and the root filesystem read-only. No plugin was deployed,
app restarted, notebook captured/changed, credential read, or API call made.

The existing extension directory contains appload and qt-resource-rebuilder,
but not the message broker or rm-shot. Broker FIFOs were absent and no XOVI
module mapping was observed in the current xochitl process. Installed files do
not prove that an extension is loaded.

The official SDK links table maps this exact Ferrari release to:

<https://storage.googleapis.com/remarkable-codex-toolchain/3.28.0.172/remarkable-production-image-5.8.203-ferrari-public-x86_64-toolchain.sh>

Installed host SDK:
`/home/oc/.cache/remarkable-port/sdk/ferrari-3.28.0.172/`, environment
`environment-setup-cortexa53-crypto-remarkable-linux`. The separate Move SDK is
not used for Ferrari binaries. Local checksum and logs are in the private report.

## Implemented and checked locally

- `tools/paper-pro-portrait/`: isolated Blank portrait candidate geometry using
  explicit physical-to-logical calibration, an independently supplied
  logical-to-scene transform and frozen page/template/viewport. No runtime or
  installation admission is supplied.
- Synthetic screenshot → crop → ink scan → Markdown raster → scaled placement
  pipeline passes, with 37 rejection cases. The 1620×2160 physical candidate
  does not assert real Qt logical size or DPR. See its README for fixture values.
- Geometry test executable compiles as AArch64 with the exact Ferrari SDK;
  this executable has not been run on the tablet.
- `tools/diagnostic/build.sh ferrari`: separate exact-tuple, process-gated
  read-only probe, compiled with the Ferrari SDK. Default Chiappa build stays
  separate; each policy rejects the other model. Both policies pass host tests.
- Ferrari diagnostic deliberately refuses native-pose queries and marks
  template identity unvalidated until the actual embedded resource is reviewed.
  It can inspect window/method/mapping metadata and bounded fixed resource
  chunks, but never captures, inserts, reads credentials or calls a provider.
- Root host and rebuilt Move session runtime/UI regression suites pass.
  Installer/firmware allowlists and Move
  packaging remain unchanged; Ferrari install releases remain empty.

The exact-SDK diagnostic and upstream broker ABI 0.2.0 are locally staged in
ignored `build/paper-pro-portrait-stage/`, with artifact hashes and upstream source
provenance in `manifest.json`. The broker comes from rm-xovi-extensions commit
`6c709dd8416ddf89034831073d76fb4ee3372905` and was compiled from an isolated local
copy using the Ferrari SDK, not copied from the Move. This staging is not
deployment, package installation or runtime verification.

Build instructions: [diagnostic README](../tools/diagnostic/README.md).
Run geometry checks after sourcing the host Qt environment:
`bash tools/paper-pro-portrait/test.sh`.

## Next gate: separately authorized temporary device diagnostic

1. Reinspect firmware, process health, root mount and loaded extensions. Preserve
   exact prior extension/package state and establish supported stock recovery.
   The historical Move shutdown issue is unresolved; do not assume a clean
   Paper Pro restart from host tests.
2. Review temporary installation of the staged broker and diagnostic, preserving
   prior files and existing mods. The inspected supported XOVI launcher targets
   xochitl.service only; its stock command removes its temporary service overlay
   and restarts xochitl. Loading and restoring stock therefore require controlled
   app restarts. Do not start persistent services,
   change boot behavior, update unrelated mods or enable developer mode.
3. With scoped owner authority, add the reviewed dependency and temporarily load
   the matching Ferrari diagnostic (not the AI runtime) using the supported XOVI workflow. A
   controlled xochitl stop/start is a device change, not read-only inventory.
4. Confirm actual loaded artifact and process health. Collect library-window
   metadata, then disposable Blank notebook mapping/method metadata. Read and
   privately retain the bounded embedded DocumentView resource for ABI review.
   Keep proprietary resources, notebook identifiers and device profiles out of Git.
5. Stop if resource/process/mapping evidence is missing, ambiguous or changes.
   No screen capture, insertion, API call, key operation or budget provision is
   authorized by this diagnostic phase. Review rollback and return health state.

## Subsequent implementation

Only after measured physical/logical/scene geometry and native resource bindings
are independently reviewed: construct a Ferrari-specific native-pose map and
private calibration; implement the fixed 360×176 owner boundary at its measured
position; validate UI suppression/restoration and exact capture admission; test
one synthetic offline answer insertion, Undo/Redo and reopen persistence. Then
integrate the existing explicit Start/Cancel request flow and unchanged dollar
ledger, conservative reservations, prompt caching, 800-token limit and no retries.
Real provider testing needs a separately specified spending allowance and scope.

Private evidence directory (not publication material):
`/home/oc/.openclaw/workspace/main/reports/remarkable-paper-pro-portrait-20261027/`.
