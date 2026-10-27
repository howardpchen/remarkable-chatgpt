# Narrow Move offline demo (bounded hardware insertion passed)

Hardware checkpoint 2026-10-26: the separate demo and own QMD are now loaded on
Chiappa, stable with zero restarts. The single authorized fresh-page run completed:
two UI-suppressed captures were pixel-identical, and the rendered fixture was
visually verified readable and in bounds below the test handwriting. Both START
and INSERTION markers are consumed; canStart=false, no retry/reset. AOT compilation hid 25 lexical
IDs from the external expression context; `NativePoseBindings.qml.inc` now adds
a fixed read-only map in the original DocumentView lexical scope. The provider
still requires every reviewed object and refuses absent entries. All 27 live
objects and the exact normal Blank viewport passed. Fresh-run owner placement,
Undo/Redo and close/reopen confirmations all passed; a separate post-reopen PNG
also verified persistence. These are this fresh run, not older fixture results.

## Separate live-text work

The [local live-text target](../move-live/README.md) shares this controller behind
an explicit compile-time build distinction. The default demo target described
below still excludes all API/config access. Rebuilding locally does not change
the loaded device artifact, its reviewed reference or consumed markers.

## Current integration

`runtime.cpp` connects the exact tested 360×176 own-control footprint to the
existing fixed-response capture/render/native-insertion backend. `MoveDemo.qml`
provides explicit Start/Cancel and status; creating/loading it does not Start.
It is separate from the production runtime; the production gate stays CLOSED.
No credentials or application config are read. No API client code is included;
Qt Quick's SDK flags include QtNetwork transitively, but the demo imports no
QNetwork symbols. The response is clearly labeled an offline fixture, not
handwriting recognition.

`LivePose` checks actual native telemetry, Blank identity, exact viewport and
resource/module hashes against a separately hardware-reviewed reference in
`/home/root/xovi/move-demo-capture-v2/reviewed-reference.json`. Missing telemetry,
missing reference, an extra window, altered module set, a changed UI footprint,
template, page or zoom rejects. Native controls are never changed. Both capture
windows use the tested render-acknowledged suppression lease; decoding and the
freshness check happen while suppressed. Cancel/error/timeout/teardown restores
controls. START_CONSUMED and INSERTION_CONSUMED remain separate durable claims;
no reset/retry exists. Native dispatch still requires unchanged cropped and answer
pixels, identity and coordinate mapping. A successful dispatch is NOT visual proof.

The read-only `inspect` command reports telemetry; `start`, `cancel`, `status` are
the only other commands. `inspect` and `status` never begin the workflow. The
parent must provision the private directory/fonts and review real screenshots
before writing the reference. No reference acceptance boolean bypasses checks.

Build with `SDK_ENV=... XOVI_REPO=... bash tools/move-demo/build.sh`.
The generated extension and private exact-resource QMD are deployed only as the
bounded offline test; this is not production support.
Use `bash tools/move-demo/test.sh` for admission, API refusal, ten integrated
asynchronous runtime scenarios, real QML pointer interaction and native telemetry
tests. Host native-pose checks use an explicit test fake, not live evidence.

The live native IDs, pose/screenshot comparison, reviewed reference, explicit
operator Start and fresh-page insertion/readability checks passed for this exact
pose. Owner Undo/Redo/reopen checks passed, with independent post-reopen screenshot evidence. The first visible-vs-hidden review
pair had eight differing ink-edge pixels; two subsequent hidden frames and both
actual runtime captures were exactly identical. The strict pixel check was NOT
relaxed. Native insertion redrew handwriting edge pixels; post-insertion image
equality of existing ink is not claimed.
Preserve every older consumed marker. This run must not reuse an old workflow.

## Original admission preparation and evidence

Actual framebuffer evidence proves Chiappa/3.28.0.172/Qt6.10.3, 954×1696 DPR1,
`moveUiProbe` 360×176 at (578,1504), current left native-toolbar pose and crop
(120,112,834,1584). Hidden frames match each other; restored frames match baseline.
This is not evidence for the product 96px control or arbitrary native overlay poses.

`admission.h` is a separate interface, not a production runtime gate
replacement. It checks the exact live own-control ownership/geometry, unchanged
reviewed Blank viewport and original live document/page. Explicit Start and both
capture boundaries require a concrete `NativePoseSource`; the separate demo's
`LivePose` now implements this, pending hardware checks. Test fakes are not live evidence. No config boolean
can open this interface. Existing runtime hardgate remains unchanged.

Start exclusively and durably claims a NEW `move-demo-capture-v2/START_CONSUMED`
attempt, before any side effect. Never remove/reset it or use consumed older
workflows. Parent separately provisions the owner-only directory and approves the
run. Cancel/failure are terminal. Directory/file no-follow, owner/mode checks,
exclusive creation, file/directory fsync make interrupted claims non-repeatable.
`startInDirectory` is a local test seam, not exposed by QML, config or a broker;
a future device caller must use `start()` only. No auto capture/insertion occurs.

## Native-pose read-only query contract (hardware verification pending)

The original metadata supplied viewport/Blank identity but no native UI pose.
The added fixed telemetry must now be hardware-verified through a
fixed expression in the effective DocumentView QML context. Its exact local
resource hash is c4b4347578f972ee9ad182606edcb4de07ea8e3eee1f3ea6f9b5a6311f4ecf0c.
These targets are proven in source, NOT yet proven accessible through diagnostics:

- `toolbar.visible`, `expanded`, `shown`, `position`, `innerWidth`, `innerHeight`,
  mapped visible child rectangles. Root anchors.fill is not painted extent.
  Resolve the Left enum from exact resource; do not guess its integer value.
- Effective visibility and descendant bounds of `_uiContainer`, `pages`,
  `pageSlider`, `pageLabel`, `titleLabel`, `newPageButton`,
  `_toolbarTooltipOverlay`, `selectionHelper`, `formatSettings`,
  `formatMenuDelegate`, `addTagPopup`, `_searchResultsHeader`,
  `searchResultsProgress`, `notificationBar`, `hwcDialog`, `adjustViewPopup`,
  `_popupOverlay`, `zoomButton`, `scrollLockVisualCue`, `writingFormatsToolTip`,
  `connectivityDialogContainerItem`, `_tocDeleteLoader`, `_tocRenamePopup`,
  `_tocCrossDocErrorLoader`, `_tocCrossDocDownloadLoader`.
- Bounded traversal (20,000 items) of native UI and other DocumentView visual
  children except `sceneView`, recording only class/objectName, effective
  visibility, opacity and mapped rectangles, not content/text. Unknown visible
  overlays, missing IDs, truncation, extra windows, modals, changed toolbar side
  or a reviewed UI footprint entering the crop fail closed. Exclude only our
  exact known control, never caller-selected arbitrary items.

Pair the resulting pose with actual screenshot masks before reviewing acceptance.
Pin exact resource/mod hashes. Repeat live pose and document/page/viewport checks
at both pre-dispatch/post-PNG boundaries. The provider must independently check
actual machine/OS/Qt and Blank template identity as well; no config assertions.
If lexical context access fails, report exact unavailable objects and prepare a
bounded read-only telemetry helper. Fetch exact Toolbar.qml/SceneViewGestures.qml
using the supported resource reader if necessary, without guessing or reloading.
Preserve all Vellum mods and never assign native toolbar visibility/expanded.

## Integration requirements (implemented locally, not hardware-validated)

Implement/review the concrete live provider. Connect explicit Start/Cancel on the
SAME tested 360×176 specimen to a fixed-response offline worker. Existing
`DiaryCaptureUiSession` RAII must hold suppression across asynchronous decode at
BOTH capture windows, releasing on success/error/cancel/teardown without blocking
the GUI. Match cropped pixels and viewport before native insertion, verify fit,
and persistently consume any insertion permission BEFORE native dispatch. The
attempt marker is not evidence an insertion occurred. No automatic insertion/load.

The whole demo uses `INK_DIARY_EXPERIMENTAL_MOVE`: common API dispatch
rejects Responses and Images regardless of config. No config/credential reads
belong in this demo. The admission interface itself has no side-effect backend;
the separate runtime composes the offline workflow backend described above.

## Verification

`bash tools/move-demo/test.sh` exercises actual QQuickItems, exact/rejected panel,
absent/rejecting native provider, page change, marker persistence across controller
instances, both capture boundaries, terminal cancel/failure, private directory,
and existing shared no-network policy with zero request counters.
`SDK_ENV=... bash tools/move-demo/build-check.sh` compiles with exact AArch64
Qt6.10.3 SDK to an object only; deliberately no `.so` to activate accidentally.
