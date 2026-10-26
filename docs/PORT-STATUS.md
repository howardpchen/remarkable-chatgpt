# Paper Pro / Move implementation status

Updated: **2026-10-26**, after owner confirmation of the disposable-notebook
lifecycle checks. This is a record of bounded experiments, **not a validated
Paper Pro/Move release**. Baseline: `7b883aaf7eaf3e0c2f06c101becab6baec17d789`.
The existing notebook-button → screenshot → AI response → rendered image → native
insertion design remains the intended workflow.

## Current evidence at a glance

| Gate | Move (`chiappa`) | Paper Pro (`ferrari`) |
| --- | --- | --- |
| Exact device / firmware / Qt inventory | 3.28.0.172 / Codex OS 5.8.203 / Qt 6.10.3 | Pending independent inventory |
| Matching official SDK build | Plugin/helper and separate probes cross-built as AArch64 ELF | Matching-device build evidence pending |
| Temporary extension and broker | Loaded; post-test app stable, zero restarts, root read-only | Untested |
| Read-only notebook mapping | Valid finite mapping; native method signatures present | Untested |
| Screenshot capture | **Blocked:** Qt render-window capture unavailable; `rm-shot` absent and untested | Untested |
| One synthetic native image insertion | Invocation succeeded; owner confirmed visibility and sensible placement/scale | Untested |
| Undo / redo / close-reopen persistence | **All passed, owner-observed** | Untested |
| Full offline question-to-answer workflow | Not yet validated | Not yet validated |
| Live API workflow | Not tested in this spike | Not tested |
| Production installation support | **Disabled** | **Disabled** |

These observations are limited to the tested portrait, normal-zoom disposable
Move notebook. They do not certify other transforms, append placement below
arbitrary/offscreen content, device-reboot persistence, or production UI behavior.
No fresh device inspection was performed merely to revise this documentation.

## Local preparation

- Separate rM2, Paper Pro and Move build targets verify actual ELF architecture
  and artifact checksums before emitting build metadata.
- The new-device compatibility inventory remains fail-closed; existing rM2
  installer acceptance stays restricted to its original exact release.
- Read-only inventory supports the observed `IMG_VERSION` firmware source.
- Capture/API lifecycle guards reject stale page and viewport contexts; viewport
  snapshots include affine calibration, window size and DPR.
- Mock transport checks cover reply lifetime, immediate completion, timeout,
  cancellation and dispatch suppression.
- Host script, geometry, mapping and bounded insertion checks passed during
  preparation. They are local evidence, not hardware certification. Some probe
  and offline-spike implementation work remains a separate uncommitted changeset.

## Move experiment record

### Diagnostic load and mapping

The first diagnostic load failed before any probe because it could initialize in
subprocesses where the broker was unavailable. The corrected loader mirrors the
broker's process predicates and requires its conditional broadcast dependency.
Recovery preserved unrelated mod/QMD hashes. Dependency resolution advanced the
XOVI extension support package from r4 to r5; recovery was therefore not an exact
package-version rollback. Subsequent corrected loads were stable.

The [read-only diagnostic](../tools/diagnostic/README.md) observed a
**954 × 1696, DPR 1** window and confirmed
`insertImageFileAsSceneItem(QUrl,QPointF)` and `viewToScene(QVariant)` with a
loaded notebook. An initial `mappingFinite=false` was inconclusive because its
QML helper was not ready. A revised synchronous helper returned
`mappingStatus=valid` and `mappingFinite=true`, without capture or insertion.

### Bounded offline insertion

A separate temporary offline extension replaced the read-only diagnostic. It
contains no API client or credential access and performs no insertion on load.
It requires the exact device/firmware/Qt tuple, unchanged page/controller/view,
954 × 1696/DPR 1 and positive finite axis-aligned mapping.

After the owner confirmed the disposable blank notebook was open:

1. Preparation generated a fixed **240 × 160** calibration PNG. Mapping measured
   approximately **0.85984848 scene units per screen pixel** for this viewport.
2. Qt window capture returned `qt-capture-unavailable`. **No screenshot was
   obtained.** This does not establish whether a supported `rm-shot` path works.
3. Exactly one native insertion was dispatched at the scene point mapped from
   window **(477, 700)**. The one-shot guard was consumed before invocation.
4. The owner confirmed the rectangle/cross/square image was visible and sensibly
   placed, then confirmed undo removed it, redo restored it, and closing/reopening
   the notebook preserved it.

Invocation success and the subsequent owner observations are distinct evidence.
The app remained stable with zero restarts after insertion, root stayed read-only,
and unrelated extension/QMD hashes matched. No live API calls occurred.
**Do not repeat this consumed insertion attempt**, including after a restart or an
uncertain tool result. Local guards do not replace the persistent operator record.

### Temporary state and recovery

At the last recorded check, the offline extension and broker remained loaded;
the verified read-only diagnostic was preserved outside `extensions.d` for
rollback. No production plugin/QMD was enabled. No credential changes or new
persistent service were part of the test. XOVI scans regular files in
`extensions.d` regardless of suffix: renaming a module there is not disabling it.
Do not load both diagnostic handlers together. Any authorized recovery should
restore the preserved module through the supported XOVI workflow, then recheck
app health, loaded artifacts, unrelated mods and root mount state.

Private raw logs, notebook artifacts and the operational recovery checkpoint
remain outside this repository. This summary contains only synthetic test
parameters and non-secret compatibility findings.

## Next gates, in order

1. **Investigate supported Move capture locally.** Check maintained upstream
   capture tooling and exact device/firmware compatibility before changing the
   tablet. Qt window capture and `rm-shot` are separate mechanisms.
2. Run a separately scoped disposable-notebook capture test. Verify actual image
   dimensions, ink visibility, crop/UI exclusions and screenshot-to-view-to-scene
   alignment. Do not substitute a synthetic image for successful capture.
3. Integrate the offline question → rendered answer → insertion path. Validate
   append placement, narrow-screen readability, glyph/diagram bounds, resource
   budgets and hardware page/viewport/cancellation rejection. Host rejection
   tests do not establish these hardware behaviors.
4. Inventory and validate Paper Pro independently with its matching SDK and
   bounded spike. Move results do not certify Paper Pro.
5. Validate QMD selectors, reversible packaging, supported recovery and rM2
   hardware regression; coordinate runtime, installer and manifest gates only
   after exact-release evidence supports promotion.
6. After offline gates pass, obtain separately scoped live API smoke-test
   authorization. Local cancellation cannot guarantee server-side cancellation
   or prevent billing.

Initial scope remains portrait, validated normal zoom and grayscale output.
Unsupported transforms must be rejected until tested. No pen-event fallback,
direct notebook-file editing, new companion service or broad firmware allowlist
is planned. This documentation commit does not authorize deployment, another
reload/insertion, a live API request, or production support promotion.
