# Paper Pro / Move implementation status

## Current personal-beta checkpoint — 2026-10-27

The owner accepted the installed per-request v4 single-capture beta for **Blank,
Dots small and Grid small**, portrait at normal zoom. Dots small and Grid small
both passed real handwritten-question recognition, native answer placement without
overlap, and owner-confirmed Undo/Redo/close-reopen. Two requests used the bounded
three-attempt root; one slot remains. No automatic retry or quota refill exists.
An ongoing-use spending allowance has not been specified; this handoff does not
increase the existing quota or budget.

The single-capture variant cannot detect new ink on the same page while thinking:
keep the page still until completion. Exact OS 3.28.0.172 / Qt 6.10.3 and reusable
device-profile checks remain. The historical shutdown crash is unresolved despite
later clean restarts. Production installation and full-size Paper Pro remain
unsupported. See [per-request beta notes](../tools/move-session/README.md).

The following sections are historical evidence, not instructions to replay runs.

Updated: **2026-10-26**, after the first supported Move screenshot validation. This is a record of bounded experiments, **not a validated
Paper Pro/Move release**. Baseline: `7b883aaf7eaf3e0c2f06c101becab6baec17d789`.
The existing notebook-button → screenshot → AI response → rendered image → native
insertion design remains the intended workflow.

## Earlier Move checkpoint (reconciled 2026-10-26)

One scoped GPT-6 Luna request and native table insertion completed; an actual
screenshot independently showed a visible 2×2 table below the handwritten request.
The owner retrospectively confirmed Undo, Redo and close/reopen worked for that live output. The consumed
run must not be reset or replayed. Later full-screen capture-only verification
showed all UI absent during capture and pixel-identical before/after restoration.
The unexplained historical shutdown crash remains unresolved; later clean stops
are not a demonstrated fix. No new API request or hardware changes were made
while reconciling this checkpoint.

See [current readiness and remaining gates](MOVE-LIVE-READINESS.md). Earlier
local-preparation and deployment reports below describe their original scopes;
claims that the live target has never completed a provider request are superseded.
The original product and offline demo guards remain intact.

## Earlier evidence at a glance (2026-10-26)

Latest bounded demo: the separate 360×176 offline Start/Cancel control now drives
two own-UI-suppressed captures, rendering, strict recheck and native insertion on
a fresh Blank page. All 27 live native UI references were verified through a
fixed read-only lexical map. Both runtime captures were pixel-identical and the
post-image showed readable, in-bounds placement below the handwriting. Ten host
failure/success scenarios passed; this is not device fault injection coverage.
Fresh-run owner placement/Undo/Redo/close-reopen checks also passed, and an
independent post-reopen PNG verified persistence. The table's older lifecycle
results below still belong to the historical fixture. API remains
excluded and the production gate remains disabled. Both new demo guards consumed;
do not reset or replay the attempt.

| Gate | Move (`chiappa`) | Paper Pro (`ferrari`) |
| --- | --- | --- |
| Exact device / firmware / Qt inventory | 3.28.0.172 / Codex OS 5.8.203 / Qt 6.10.3 | Pending independent inventory |
| Matching official SDK build | Plugin/helper and separate probes cross-built as AArch64 ELF | Matching-device build evidence pending |
| Temporary extension and broker | Loaded; post-test app stable, zero restarts, root read-only | Untested |
| Read-only notebook mapping | Valid finite mapping; native method signatures present | Untested |
| Screenshot capture | Qt capture unavailable; supported `rm-shot` PNG decoded at 954×1696 with calibration image visible; controlled live toolbar change passed; crop/ink extent checks pending | Untested |
| One synthetic native image insertion | Invocation succeeded; owner confirmed visibility and sensible placement/scale | Untested |
| Undo / redo / close-reopen persistence | **All passed, owner-observed** | Untested |
| Integrated offline fixture capture/render/insertion | **Passed bounded Blank-template fixture**; Undo/Redo/close-reopen also owner-confirmed | Untested |
| Handwritten question recognition / live answer | One narrow live handwritten table request completed; broad quality untested | Not tested |
| Live API workflow | One bounded GPT-6 Luna request/insertion; live Undo/Redo/reopen owner-confirmed | Not tested |
| Production installation support | **Disabled** | **Disabled** |

These observations are limited to the tested portrait, normal-zoom disposable
Move notebook. They do not certify other transforms, append placement below
arbitrary/offscreen content, device-reboot persistence, or production UI behavior.
The rm-shot capture is separate evidence from the earlier failed Qt capture; production workflow validation remains pending.

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

### Capture dependency activation and recovery

After the owner authorized resuming the Move port, supported Vellum installation
added only `rm-shot 1.2.0-r0` and `framebuffer-spy 19.0.0-r5`. The rm-shot module
matches the verified upstream release asset. Stock dependency conditions unload
it when broker/framebuffer exports are unavailable, before dependency constructors.

The completed insertion spike was quarantined outside `extensions.d` and the
verified read-only diagnostic restored. One supported app reload loaded capture,
broker, probe and prior mods. Follow-up showed a stable app, zero restarts,
read-only root and unchanged unrelated mod/QMD hashes. The framebuffer constructor
hook reported storage **960 × 1696, RGBA, stride 3840**. rm-shot should trim the
six padded columns to **954 × 1696**; that still requires actual PNG validation.

After fresh owner confirmation that the disposable notebook was open, one
bounded rm-shot request completed. The PNG independently decoded as **954 × 1696
RGB**, upright, with the expected calibration rectangle/cross/square visible.
The outline measured x340–612/y609–788 and cross center near (477,700), matching
the original insertion center. Six padded framebuffer columns are absent.
Pre/post read-only probes retained valid mapping at DPR1. The app stayed on the
same PID with zero restarts, root read-only and unchanged extension/QMD hashes.

The left toolbar occupies x0–111 and a close control is near (898,56). These
observations inform a candidate crop, not a validated production UI profile.
A controlled toolbar-hidden second frame passed: UI changed while the calibration
fixture region stayed pixel-identical. Both frames decoded at954×1696. Actual
production ink scanning counts the black dotted template near the bottom and
leaves no append space; Blank-template testing is next. Handwritten question
handling and template-dot exclusion are not established.
The original insertion remains consumed and no insertion-capable endpoint is
loaded. No production plugin/QMD, API call, credential change or new persistent
service was activated.

If recovery is needed, inspect current state, quarantine only the new capture
modules outside `extensions.d`, and use the supported XOVI start workflow. Preserve
the verified read-only probe, broker, existing mods and access path; verify health,
hashes and read-only root. Renaming a module inside `extensions.d` does not disable
it. Private raw logs and operational checkpoints remain outside this repository.

## Separate integrated offline fixture preparation

Blank-template capture retained the calibration image and made actual ink extent
scan terminate at the image edge; the fixed local response fits below it. This
isolates a template-sensitive append scanner issue, not a native placement failure:
earlier image placement, undo/redo and persistence passed on the dotted template.

A separate temporary `offline-workflow` tool uses rm-shot and the production
renderer with explicit crop and measured scene scaling. Its first load failed
XOVI pass1 because an explicit condition duplicated the condition generated by
`import?`. The failing artifact was quarantined; the corrected manifest retains
the eligibility condition and build now rejects duplicate generated link entries.
Corrected exact-SDK artifact loaded successfully; parent verified module mappings,
idle status, stable app/zero restarts, read-only root and unchanged unrelated hashes.
After fresh owner reopen confirmation, the actual capture → production renderer
→ second-capture freshness check → one native insertion passed. Parent independently
inspected the post-insertion screenshot: fixed response readable below calibration
and within the visible page. Measured scene scaling converted834×397 screen pixels
to717×341 native image pixels. App remained stable with zero restarts/read-only root
and unchanged unrelated mod/QMD hashes. The new durable insertion marker is consumed;
never repeat it. New-response Undo/Redo/close-reopen checks ALL PASSED, owner-observed.
This fixed response does not recognize handwriting or answer a real question; no
API request ran.
Production support remains disabled.

## Next gates, in order

1. Local-only bounded live-text implementation and host/SDK checks are recorded
   in the separate target README. This does not certify hardware recognition.
2. Plan controlled deployment/recovery, separately scoped reload/insertion and
   live API spend, private key provisioning and a fresh disposable Blank page.
   Preserve all older consumed markers; new artifact hashes need a new reference.
3. Verify one real ordinary handwritten question → useful OpenAI text answer →
   readable in-bounds insertion; owner Undo/Redo/reopen and independent health.
   Cancellation cannot retract a dispatched request or guarantee no charge.
4. Broaden hardware page/viewport/cancel/resource coverage, long/offscreen append,
   template support, reversible packaging and rM2 hardware regression. The prior
   hardware shutdown SIGSEGV remains unresolved; host lifecycle passes are not a fix.
5. Promote runtime/installer/manifest gates only with exact-release evidence.
   Only after Move works, independently inventory and test full-size Paper Pro.

Initial scope remains portrait, validated normal zoom and grayscale output.
Unsupported transforms must be rejected until tested. No pen-event fallback,
direct notebook-file editing, new companion service or broad firmware allowlist
is planned. This documentation commit does not authorize deployment, another
reload/insertion, a live API request, or production support promotion.

## Isolated UI-only gate (2026-10-26)

After local ownership/destruction and real QML pointer-interaction tests passed,
an isolated UI-only patch loaded following a verified clean old-process stop.
Owner reports requested Start/Cancel and notebook close/reopen behavior correct.
Parent postinteraction check: same appPID,zero restarts,read-only root,unchanged
unrelated module/QMD hashes and consumed insertion marker preserved. This UI
simulates busy/cancel locally: no backend cancellation, recognition or API claim.
The earlier shutdown SIGSEGV remains unexplained; one clean stop is not a proven
fix. Template/normal-viewport live metadata is the next separate validation gate.

## Actual own-control screenshot exclusion (2026-10-26)

A separate capture-only diagnostic verified two capture windows on Move. Our
360×176 notebook control was absent from both actual framebuffer PNGs; both
hidden images matched pixel-for-pixel, and both restored-control images matched
the before image pixel-for-pixel. Only our control rectangle changed. The native
left toolbar/Vellum configuration and full measured viewport stayed unchanged.
This passes the own-control overlap check for the tested pose, not arbitrary
native toolbar positions/modals or the distinct production96px control. No new
insertion/API ran. The next stage is narrowly scoped offline demo integration;
production installation/eligibility remains disabled.
