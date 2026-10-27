# Full-screen Move live capture (local revision)

> Historical scope report. Subsequent authorized deployment, one GPT-6 Luna
> request/insertion, and full-screen capture-only validation supersede the
> not-deployed/not-live-tested statements below. See [current readiness](MOVE-LIVE-READINESS.md).


The separate live experiment now captures (0,0,954,1696), not the earlier
(120,112,834,1584). This is a local revision, not evidence of device installation.
The offline demo and production compatibility gates remain unchanged.

Existing widget launcher, minimize, × dismissal and toolbar-show recovery behavior
are unchanged; capture hiding does not redesign those interactions.

## Temporary native UI hiding

The live widget uses a capture-only QML `Binding` on the exact lexical native
toolbar's `visible` property. `Binding.RestoreBindingOrValue` preserves the
original binding/value. It does not change `expanded`, toolbar positioning or
persistent settings. The same lease release restores native visibility on each
capture, cancellation, error and capture-session teardown while the widget exists.
It restores UI during the API wait. Per owner direction, destroying the widget
itself is an intentional exception: it may leave the native toolbar hidden; no
extra widget-destruction restoration hook is installed. A toolbar already hidden remains hidden; changes in the original bound source
while hidden take effect when restored.

Capture requires the widget's hidden-toolbar acknowledgement and a schema-2
reference with `captureArea:[0,0,954,1696]`, exact `nativePose` for the restored
state and `hiddenNativePose` measured while the capture Binding is active.
Visible native elements smaller than the full frame reject capture; a close
button that remains visible is not silently ignored. Existing exact native
resource/mod manifests, page/viewport checks, two identical pixel captures,
render fit and durable consumed markers stay enforced. Old cropped references
are rejected before any new request. No fallback to a partly obstructed frame.

Capture expands to all edges, but answer rendering retains width=834 and x=120
to avoid the restored left toolbar. Device geometry/mapping must remain stable
across hiding/restoration; otherwise the context checks reject the run.

## Validation and deployment boundary

`tools/move-live/test.sh` checks full-frame payload dimensions and first-row/left
edge pixels, 22 integrated scenarios including old-reference refusal, client
transport guards, and real QML toolbar Binding restoration. Native visibility,
close-button inheritance and preserved bindings are modeled in the host tests;
these are not firmware visual proof. Historical demo and capture-lease tests
remain applicable. Build both the live SO and firmware-specific QMD together.

Before another live run, deploy only with authorization for the notebook-app
reload, keep all consumed markers, verify current mod hashes/runtime health,
then obtain actual hidden/restored pose telemetry and before/hidden/after
screenshots. Confirm native bar, collapse handle and close button are absent in
the full captured frame and the prior toolbar state returns afterward. Do not
infer device behavior from successful QMD application alone. No paid request or
native insertion is required for this capture-only verification.
