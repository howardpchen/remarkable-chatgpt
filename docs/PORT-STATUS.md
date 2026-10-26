# Paper Pro / Move implementation status

Baseline: `7b883aaf7eaf3e0c2f06c101becab6baec17d789`. This branch is **port preparation, not a validated Paper Pro/Move release**. The original notebook-button, screenshot-question, AI routing, rendering and native-image workflow remains the intended design.

## Implemented and checked locally

- Isolated rM2, Paper Pro (`ferrari`) and Move (`chiappa`) build targets; actual plugin/helper ELF architecture and checksums checked before successful build metadata is emitted.
- Fail-closed new-device compatibility inventory; original rM2 installer remains restricted to its existing exact release and now checks target artifacts before mutation.
- Metadata-only read-only tablet inventory script with the observed `IMG_VERSION` firmware source supported.
- Guards after asynchronous capture waits/decoding and immediately before API submission; stale page/viewport contexts are rejected. Viewport snapshot records affine calibration, window size and DPR.
- Network reply lifetime, immediate completion and timeout handling hardened; offline mocked transport tests verify cancellation and dispatch suppression.
- Existing host regression suite plus script, viewport and diagnostic-policy checks pass. Host compilation is not hardware proof.

## Move build evidence

A read-only inventory found a Chiappa running software **3.28.0.172**, Codex OS **5.8.203**, Qt **6.10.3**. Its matching official product SDK successfully cross-built the plugin and configuration helper as AArch64 ELF. Shared-library names and required native insertion/mapping names were inspected, not runtime behavior.

A separate [diagnostic-only extension](../tools/diagnostic/README.md) was built against that SDK. Its runtime gate requires that exact model, software release and Qt version. It can inspect visible method signatures and mapping finiteness through the existing broker protocol without screenshots, network calls, insertion or QMD patches.

The production plugin has **not** been loaded and still refuses the new device/firmware. The first diagnostic-only load failed during startup, before any probe, capture or insertion. Same-attempt logs show the broker initializing in the GUI process, while another child process aborts because the broker registry entry is absent there. The diagnostic initially lacked equivalent subprocess eligibility conditions. The corrected diagnostic now mirrors the upstream process predicates and conditionally unloads if the broker is unavailable; regression checks pass. Matching safeguards were added to the production XOVI specification.

Recovery removed the new broker and quarantined the diagnostic outside `extensions.d` (XOVI scans regular files regardless of suffix). Original mod binaries/QMD hashes match baseline, the restored UI process is stable, and root remains read-only. The package manager advanced the XOVI extension support package from revision r4 to r5 while resolving the broker; that revision remains, so this is not an exact package-version rollback. No notebook mutation, live API request, credential change or new persistent service was performed.

One bounded corrected load subsequently succeeded with broker 19.0.0-r5. The read-only diagnostic returned a **954 x 1696** window with **DPR 1**. The same notebook-app process remained active with zero restarts across follow-up checks, and root stayed read-only. The library was visible (`documentLoaded=false`), so notebook-controller signatures and native capture/insertion are still unverified. The diagnostic/broker are temporarily loaded for that remaining test, not a released install. No additional device reload is needed merely to repeat the metadata probe after opening a disposable notebook.

## Still required before support can be claimed

1. Bounded offline runtime diagnostic and native capture/insertion spike on Move, including scale/placement, undo and reopen persistence.
2. Equivalent inventory, matching SDK build and independent spike on Paper Pro. Move success does not certify Pro.
3. Validated device/firmware adapters and geometry contract: screenshot pixels, Qt logical/view coordinates and scene units must not be conflated.
4. Readable narrow-screen renderer/UI behavior, capture exclusions, glyph/diagram bounds, resource budgets and stale-context failure tests on hardware.
5. Compatible QMD selectors, coordinated runtime/installer/manifest gates, reversible packaging, recovery and rM2 hardware regression evidence.
6. Separately scoped live API smoke tests after offline gates pass; no claim that server-side processing/billing is cancelled when a local request aborts.

Portrait with validated normal-zoom geometry and grayscale answers is the initial scope. Other transforms must be rejected until tested. No new companion service, pen-event fallback, direct notebook-file modification or broad firmware allowlist is planned.
