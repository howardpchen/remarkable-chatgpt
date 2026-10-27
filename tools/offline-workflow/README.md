# Separate Move offline fixture workflow — preparation only

**Not deployed or hardware-validated. Not production Move support.** This is a
new endpoint and artifact, not a reset or replacement of the consumed historical
`offline-spike` insertion. Never reactivate that old spike. Loading this new
insertion-capable tool or issuing its `insert` operation is a separate bounded
hardware gate owned by the parent/operator, not performed by these build/tests.

## What it exercises

Exact Chiappa / release 3.28.0.172 / Qt 6.10.3 / broker GUI-process gate from the
existing diagnostic. Supported rm-shot 1.2.0 performs two asynchronous captures;
no Qt window grab. The caller supplies an **explicit measured** portrait content
crop. Actual `diaryCaptureLayout`, `lastInkRow` and `renderDiaryMarkdown` production
helpers run unchanged, with bundled Caveat and DejaVu Sans Mono fonts. The answer
is a short fixed fixture clearly labeled as such. **The screenshot is NOT
transcribed or recognized, and the answer does NOT respond to handwriting.** This
tests capture → ink extent → local rendering → fresh capture → native insertion,
not the live question-recognition/API flow. No credentials, API/network clients,
QMD, runtime/installer/support gates, pen injection or notebook-file editing.

The tool has no auto-run behavior or metadata endpoint. Only
`ink-diary-workflow$run` is registered. Its inputs are JSON, at most 1024 bytes;
there are no caller-supplied output paths, scripts, response text or URLs.

## Bounded operation sequence (not authorization to execute)

Each operation is a separate broker request, dispatched on the GUI thread:

1. `{"op":"prepare","measuredCrop":[X,Y,W,H]}`: caller-provided integer crop
   must fit 954×1696/DPR1 and exclude the observed left toolbar and top close
   control (`X >= 112`, `Y >= 112`, width ≥300). These are minimum exclusions,
   **not proof that a crop is correct**. Parent must independently measure and
   review the actual current screenshot. No default crop is accepted.
2. `{"op":"capture"}`: request unique `question.png` through rm-shot. Returns
   immediately; no GUI-thread sleeping/event-loop pumping. Capture dispatch
   failure/uncertainty is terminal, not retried.
3. `{"op":"render"}`: poll for decodable full-size capture within 10 seconds
   of dispatch. `capture-pending` is the sole nonterminal polling result. Render
   fixed Markdown at the crop width, below the final detected ink row +45px.
   Refuse a blank crop or answer extending outside its visible rectangle. Review
   output dimensions/placement before proceeding. There is **no offscreen append**.
4. `{"op":"verify"}`: start a second unique `check.png` capture.
5. `{"op":"insert"}`: while that capture is pending, this operation returns
   `capture-pending` and does not insert. Once decoded, require identical cropped
   pixels, identical canonical answer pixels, exact page/controller/window/view
   identity and finite positive axis-aligned mapping; then consume a durable
   marker **before** one direct native `insertImageFileAsSceneItem(QUrl,QPointF)`
   invocation. `insertionAttempted:true` is NOT proof of placement. An uncertain
   reply must be inspected and never retried automatically.

`{"op":"cancel"}` terminally invalidates this process's workflow.
`{"op":"status"}` reports phase (Idle0, Prepared1, Capturing2, Rendered3,
Verifying4, Consumed5, Cancelled6) and durable consumed-marker presence, no page ID.
There is no reset operation. Reloading cannot bypass a consumed marker or reuse
prior captures. Preparation refuses pre-existing run artifacts. Do not delete
markers or rename the run directory merely to repeat an insertion.

Private fixed device directory: `/home/root/xovi/offline-workflow-v1` (0700).
Parent must stage `Caveat.ttf` and `DejaVuSansMono.ttf` there before the gate.
Images reside within this private directory; answer/marker are explicitly 0600.
`INSERTION_CONSUMED` is atomically created with `NewOnly`, flushed and fsynced
along with the directory before invoking native insertion. An inability to persist
consumption prevents insertion. Native placement/scale, narrow readability,
undo/redo, persistence, resource health, post-capture evidence, long/offscreen
output, cancellation during real asynchronous capture and exact-release QMD UI
remain hardware validation work. Insertion is center-anchored using the same
screen-to-scene convention as production; physical image scale remains a gate. The fixture now rescales its rendered PNG
using the measured scene-units/window-pixel map and checks its actual visible
footprint; host tests cover both 2× and observed approximately0.85985× mappings.
This correction is local preparation until confirmed on hardware.

## Build and local tests

```sh
SDK_ENV=/path/to/exact/chiappa-3.28.0.172/environment-setup-cortexa55-remarkable-linux \
XOVI_REPO=/path/to/xovi bash tools/offline-workflow/build.sh
bash tools/offline-workflow/test.sh
```

Host tests require Qt6 Core/Gui/Qml/Quick development tools/offscreen plugin.
The extracted local Qt environment additionally needs `PKG_CONFIG_SYSROOT_DIR`,
`PKG_CONFIG_LIBDIR`, `LD_LIBRARY_PATH`, and `QT_PLUGIN_PATH` set to its root.
Tests use synthetic captures/crops, a real Qt view/engine and mock controller.
Each test process gets a new build-local temporary directory. Test-only resets
exist solely inside the host test program, not the extension or broker API.

Cross-build emits ignored `bin/chiappa/ink-diary-workflow.so` and
`offline-workflow-build.json`; this does not stage/load anything on the tablet.
Only this tool's source/test/build files are changed. Production and historical
spike remain unchanged. Parent owns deployment/health/recovery, measured crop,
and any later device insertion authority/checkpoint. If approved for deployment,
record current state/artifact hashes before staging, use supported XOVI reload
once, verify stable process/root read-only/module health, then require the owner
to reopen the designated disposable notebook; do not infer continuity after load.

## Startup failure and correction (2026-10-26)

The first hardware load failed in XOVI pass1 before workflow initialization:
explicit `condition rm-shot$screenshotHandler` duplicated the condition generated
by `import? rm-shot$screenshotHandler`. Stock UI recovery did not establish mod
recovery. Parent quarantined that exact artifact outside extensions.d. The
corrected manifest retains `import?` (both condition and import) and removes only
the redundant explicit condition; no eligibility check is weakened. The build
now checks generated LINKTABLENAMES for duplicate pass1 entries before linking.
The checker rejected the actual failing generated table and accepts the corrected
one. Local compile/host tests alone do not prove device startup compatibility.
