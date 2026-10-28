# Experimental Move capabilities

This development branch keeps native reMarkable 2 support separate from the
experimental Chiappa / OS 3.28.0.172 / Qt 6.10.3 implementation. Full-size
Paper Pro work is paused; its isolated preparation is not a supported port.

## Orientation and zoom

- Portrait supports Blank, Dots small and Grid small; landscape is Blank-only.
- Physical framebuffer and reading coordinates are distinct. Explicit reviewed
  quarter-turn corners and independent orientation calibration are required.
- Schema 3 remains portrait-only. Schema 4 selects an independently reviewed
  orientation record; missing calibration fails closed.
- Uniform 50%–400% zoom and finite pan are admitted at Start. The complete
  transform is frozen for each request; changes cancel at guarded boundaries.
- The capture owner remains exactly 360×176 in reading space. Toolbar and zoom
  controls are hidden reversibly, without weakening native-pose admission.
- Dollar reservations, prompt caching, the 800-token output limit, explicit
  Start/Cancel and no automatic retries remain unchanged.

Loaded-resource verification and synthetic tests are not end-to-end hardware
validation. Real landscape capture/insertion and zoomed notebook interactions
remain unvalidated. Do not infer broader device or firmware compatibility.

## Lifecycle and packaging

Shared owned-job GUI dispatch closes intake at shutdown, cancels pending work,
and distinguishes an in-flight timeout that must not be retried. The separate
experimental broker overlay preserves its protocol and bounds FIFO shutdown.
These changes are not a demonstrated fix for historical stock shutdown crashes.

Container package tests require explicit non-root host UID/GID and hand back
their dedicated synthetic output mount on exit, preserving restrictive modes
and avoiding symlink dereferencing. See [packaging instructions](../packaging/vellum/README.md).
Build artifacts, signing keys, credentials, device profiles, notebook data and
operational reports are not publication assets.

## Reproducible host checks

```sh
bash test.sh
DOLLAR_BUDGET=1 bash tools/move-session/test.sh
DOLLAR_BUDGET=1 SINGLE_CAPTURE=1 bash tools/move-session/test.sh
bash tools/move-live/test.sh
bash tools/move-demo/test.sh
```

Use the exact product SDK for target builds. Host checks alone do not prove
target compatibility. Publication does not deploy a package, change device
state, trust a signing key, fund an allowance or authorize paid API requests.
