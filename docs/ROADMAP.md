# Roadmap

## Current release preparation

- Implement matching native-envelope placement semantics in separate Chiappa and
  Ferrari packages; preserve each device's exact geometry/native-pose admission.
- Rebuild host tests and exact SDK targets; audit signed APK payloads and synthetic
  install/upgrade/removal including private-state preservation and wrong-model/OS
  refusals. Publish only inspected generic source/docs and synthetic fixtures.

## Hardware validation still needed

- Deploy the new Ferrari candidate only under separate scoped authority; validate
  native-bounds placement and native Undo/Redo/reopen on the actual device.
- Validate the new packaged lifecycle on Chiappa; earlier focused module testing
  does not prove Vellum migration/upgrade/removal for this package.
- Expand evidence across imported/image/vector content, hidden layers, templates,
  zoom/pan and both reading orientations; never turn sample evidence into a
  universal-coverage claim.

## Reliability and distribution

- Establish a complete content-revision source for equal-envelope edits and
  switch-away/back; do not revive the failed watcher without a new justified plan.
- Resolve historical xochitl shutdown instability before broader support claims.
- Improve supported setup/calibration UX without bundling private calibration,
  licensed fonts, credentials or funding a budget automatically.
- Vellum catalog submission is separate from source push/local signed packages.

See [current checkpoint](PROJECT-STATUS.md) and [parity matrix](VELLUM-PARITY.md).
