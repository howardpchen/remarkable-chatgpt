# Template-independent native content placement

## Request path

1. Admit the exact supported device/firmware/Qt tuple, calibrated orientation,
   uniform 50–400% zoom, native poses/resources and exact capture-owner boundary.
2. On the GUI thread read `itemsBoundingRect`, `rootDocumentBoundingRect` and
   `boundingRect` from the native controller. Require typed finite nonnegative
   rectangles. Freeze all three alongside page/object and viewport identity.
3. Union the rectangles conservatively in scene coordinates, retaining horizontal
   and vertical lines. A zero-height root-text rectangle is not verified emptiness.
4. Project every envelope corner through the inverse frozen reading-to-scene map.
   The largest reading Y is the lowest obstacle, including lower offscreen content.
5. Place below that row with the existing 45-pixel gap. Preserve toolbar clearance,
   full-question/API pixels, rendering limits and answer-fit checks. Obvious lack
   of even the minimum gap refuses before API dispatch; answer-dependent fit can
   still fail after incurring provider cost. Never shrink/retry silently.
6. At existing capture/API/render/insertion boundaries compare all native bounds
   in addition to page/object/geometry/native-pose/resource and answer integrity.
   Ferrari retains second-capture pixel comparison; Move retains single capture.

Shared numeric code is product-neutral. Ferrari and Chiappa retain their own
axes, physical/readout mappings, exact SDKs, resource bridges and private profiles.
No template-pixel subtraction or template allowlist participates in per-request
native-bounds admission. Optional sanitized identity metadata is frozen for
observed template changes; absent metadata cannot reject an otherwise valid view.

## Calibration compatibility

Chiappa keeps its reviewed orientation anchor and exact supported opposite-turn
pose derivation. Ferrari requires independently reviewed CW and CCW anchors.
Ferrari honors existing explicit per-template pose records where present; a
broken explicit record refuses rather than silently falling back. Without such a
record, the SAME Ferrari orientation anchor is used. Template pixel pitch is no
longer needed. This never substitutes Move data or approves a live native pose
against itself. Exact live pose/resource checks still run before any capture.

Historical one-shot workflows and native rM2 retain their original contracts.
The optional read-only `content-detection` endpoint exports numeric geometry only;
its coverage/revision flags remain unverified. No notebook text/IDs are exported.

## Evidence and limitations

The owner confirmed real Chiappa ruled-template placement below handwriting
without overlap and successful Undo, Redo and close/reopen. Earlier numeric
samples cover erasure/Undo, hidden/scrolled writing and native text. Ferrari's
new native-bounds runtime is locally prepared, not hardware-validated.

A rectangle snapshot is not a monotonic page revision. Equal-envelope edits,
object changes restored between observations and switch-away/back can evade
snapshot comparison. The single-capture Move path cannot catch every same-page
edit. Do not claim complete mutation safety or universal imported/image/vector
coverage. Conservative union can refuse usable internal whitespace; this method
only supports append-below-all-content, not finding gaps between writing.

The abandoned signal-watcher experiment is not part of the packaged runtime.
Its failed live-event trial is historical evidence, not a solved revision source.
Explicit Start/Cancel, the conservative persistent dollar ledger, maximum 800
output tokens, prompt caching and no automatic retries remain unchanged.

## Prior misleading template error

The old viewport collector returned an empty object for non-allowlisted template
artwork. Numeric validation then emitted a DPR/window error despite a valid view.
Removing template-only collection/admission gates fixes that cause without
removing dimension, DPR, calibration or native-pose guards.
