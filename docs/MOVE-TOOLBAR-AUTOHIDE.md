# Move request-scoped toolbar auto-hide

## Current checkpoint (2026-10-27)

Implemented and verified **locally; not installed**. The owner reported
`native-ui-pose-changed-or-unavailable` with the native toolbar visible and
requested automatic hiding during inference, followed by restoration.
No device connection/change, restart, Start or provider request occurred here.

## Cause and behavior

The accepted profile's normal pose has `toolbar.expanded=false`,
`visible=true`, `shown=false`; its capture pose has `visible=false` as well.
An expanded toolbar fails exact normal-pose admission before the old
capture-only suppression lease runs. This is independent of Grid ink detection.

The per-request runtime now acquires a request-scoped toolbar lease before
admission. The actual QML uses reversible `Binding.RestoreBindingOrValue`
overrides for native `visible` and `expanded`. Native pose checks then compare
the **complete, separately reviewed hiddenNativePose**, including at API-wait
and insertion boundaries. No live pose is blessed, no fields are ignored and
no calibration is synthesized. Unsupported positions, overlays, changed pages,
zoom and resource/artifact mismatches still reject.

Only the native toolbar stays hidden through inference; our own UI is restored
after each screenshot, keeping Cancel reachable. Terminal results, early
failures, cancellation, timeout and runtime/widget teardown release the
overrides. A previously hidden/collapsed toolbar stays in that prior state;
native bindings remain intact, including changes to their source while hidden.
The screenshot lease remains independent and cannot restore the toolbar early.
Missing/incompatible bridges and already-held leases fail before allocating a
request identity; a failed acquirer cannot release somebody else's lease.
Normal pose admission failures retain existing conservative reservation rules.
Budget policy, ledger, capture-owner dimensions and no-retry behavior are unchanged.
One-shot live/offline builds retain their existing toolbar behavior.

## Verification and candidate

- Focused installed-baseline build: **40 request scenarios in each capture
  mode**, including Grid/table regression, budget persistence, cancellation,
  native-pose changes, timeouts and shutdown/teardown.
- Actual candidate QML: all four prior visible/expanded states; nested capture
  lease; binding restoration; exclusivity; missing bridge; notebook destruction.
- Real numeric native-pose collector with a synthetic complete bridge:
  expanded toolbar differs from reference, request-hiding exactly matches the
  reviewed hidden pose, and an additional overlay still differs/rejects.
- Exact Chiappa / Qt 6.10.3 AArch64 module builds. Firmware-specific QMD hashes
  and applies locally. An expected runtime resource preserves the entire saved
  native/other-mod prefix byte-for-byte; it is **not a live observation**.
- The inherited one-shot live/config/client/UI suite passes unchanged behavior.

Private artifacts, profile, focused source and logs:
main-workspace `reports/remarkable-toolbar-autohide-20261027/`.
The candidate starts at installed source `153d17a6`, plus Grid fix `ca9f07e`
and only the toolbar change. Newer CLI and unrelated Paper Pro/lifecycle changes
are excluded. A separate current-checkout SDK build also passed.
Candidate SO SHA256:
`fc37ceca586b5578aad1b96eae7089e7835e989dd20fdf63b6855f7329335d18`.

## Next action

Device deployment needs fresh scoped authorization; the earlier Grid-only
installation approval does not cover this new update. Before activation,
inspect current device/runtime and idle state and compare against the saved
baseline. Back up SO, QMD and profile; update only those three files, preserving
calibration, keys, policy and all run records. Use a rollback-safe monitored
stop/start, verify effective loaded SO/resource/profile and preserved state.
Do not make a paid request to verify installation. Real toolbar behavior and
answer insertion remain unverified on the tablet. Rebuild/revalidate the older
Vellum package before any package migration or distribution.
