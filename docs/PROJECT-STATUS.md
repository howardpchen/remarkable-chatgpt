# Project checkpoint — native-bounds package parity

## Current objective

Separate Ferrari and Chiappa Vellum beta2 packages are prepared with native-bounds
placement parity, consolidated documentation/comments, exact-SDK builds and
synthetic signed-package audits. The intended generic source commit is ready for
publication after final tree and outgoing-history verification. No new device deployment, credential change or API call is part
of this package-preparation task.

## Evidence by target

| Evidence | Chiappa / Move | Ferrari / Paper Pro |
|---|---|---|
| Native-envelope placement implemented | yes | yes, separate local candidate |
| Template-only per-request admission removed | yes | yes, local candidate |
| Exact firmware / SDK |3.28.0.172 / Qt 6.10.3 |3.28.0.172 / Qt 6.10.3 |
| Geometry / native poses | existing calibrated Move profiles | independent Ferrari profiles and CW/CCW anchors |
| Current native-bounds module loaded on tablet | verified earlier focused build | not deployed |
| Real response below writing without overlap | owner confirmed on one ruled-template page | not validated for new method |
| Undo / Redo / close-reopen | owner confirmed for that Move sample | not validated for new method |
| New beta2 Vellum install on tablet | not performed | not performed |

Local suites, exact-SDK builds and signed lifecycle/payload audits passed for both
devices. Chiappa single/dual modes ran 78 request scenarios each; Ferrari ran 96.
Historical runtime/core and offline CLI checks also passed. Results are recorded in [the parity matrix](VELLUM-PARITY.md).
The new cleaned Move package omits the abandoned signal watcher; its build is not
identical to the earlier live-tested module. Ferrari's earlier pixel-based runtime
and historical hardware checks are not validation of its new native-bounds code.

## Architecture and limits

Shared product-neutral geometry lives in `src/native-content-bounds.h` and
`src/native-placement.h`. Each runtime retains its own capture geometry, SDK,
profile, native resource/pose bridge, broker identity and private-state directory.
Native rM2 and historical one-shot admission remain separate.

Bounds and page/object/viewport snapshots are not monotonic content revisions.
Equal-envelope edits or a switch away/back between observations can escape.
Move single-capture retains this limitation; Ferrari keeps its second-capture
pixel check. Hidden/offscreen/text/erasure samples support the method, not complete
imported/image/vector coverage. Historical shutdown instability is unresolved.

## Resume

1. Use exact current package payload and lifecycle audits before any rollout.
2. Obtain separately scoped device deployment approval; inspect live state first.
3. Preserve private state and rollback; activate once from owner-confirmed library.
4. Independently verify mapped code/resources and unchanged usage/state.
5. Validate Ferrari native-bounds response placement, Undo/Redo and persistence;
   broader templates/orientations/content require their own evidence.

Abandoned watcher experiments and chronological private deployment notes are
retained outside the checkout. No automatic retry, restart or paid trial is queued.
