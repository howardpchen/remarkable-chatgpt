# Bounded Move offline spike (not production support)

Separate temporary extension, exact Chiappa/3.28.0.172/Qt6.10.3 and broker GUI
process gates inherited from the verified metadata probe. It also exports the
read-only metadata signal; do not load alongside the original diagnostic.
No QMD patch, network client, credentials, pen injection, notebook-file editing,
package transaction or persistent service is included.

`ink-diary-offline$run` accepts only three literal operations:

- `prepare`: require one visible window, loaded document and matching view;
  require a nonempty internal page identity, 954x1696/DPR1 and positive
  axis-aligned finite scene mapping. Generate a fixed 240x160 black-and-white
  calibration PNG; retain page/controller/view identities and geometry in memory.
- `capture`: require unchanged prepared context; attempt Qt render-window
  capture to `before.png` or `after.png` under the private device directory
  `/home/root/xovi/offline-spike`. This is **not rm-shot** and may omit hardware
  composited ink. Failure is a separate capture blocker, not insertion proof.
- `insert`: recheck the exact page/controller/view and viewport; accept only the
  expected native method signature and fixed generated PNG. Consume a one-shot
  guard **before** directly invoking native insertion at the scene point mapped
  from window (477,700). A returned invocation success does not prove placement.

Only prepare/capture after the owner confirms the disposable blank notebook is
visible. After one insertion, require visual placement, scale, undo/redo and
reopen checks. Stop after an uncertain insertion; do not repeat automatically.
No API calls are part of this spike. Production support stays disabled.

## Build

Use the same SDK_ENV and XOVI_REPO as `../diagnostic/build.sh`:

```sh
bash tools/offline-spike/build.sh
```

Actual ELF64/AArch64 verified; source/artifact hashes recorded in ignored
`bin/chiappa/offline-build.json`.

## Host integration test

With native Qt6 Core/Gui/Qml/Quick development tools and offscreen platform:

```sh
mkdir -p build-host
"$(pkg-config --variable=libexecdir Qt6Core)/moc" tools/offline-spike/test-spike.cpp -o build-host/test-spike.moc
c++ -std=c++17 -fPIC tools/offline-spike/test-spike.cpp -Ibuild-host -o build-host/offline-spike-test $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core) -ldl
INK_SPIKE_TEST_DIR="$PWD/build-host/offline-test-artifacts" QT_QPA_PLATFORM=offscreen ./build-host/offline-spike-test
```

The test uses a real window/view/engine with a fake native controller. It checks
valid mapping, degenerate/reflected mapping refusal, unprepared/page-changed/
viewport-changed refusal, one invocation and duplicate refusal. Host-only path
override is compiled exclusively by test-spike.cpp, never by build.sh.

## Temporary device load/recovery

Record current hashes, PID/restarts, root mount and narrow logs. Preserve the
verified original probe outside extensions.d; stage and checksum the new binary
before an atomic rename into extensions.d. Keep only one metadata signal handler.
Use supported `/home/root/xovi/start` for one app reload; verify loaded mappings,
FIFOs, zero restarts, stable PID, root read-only and no fatal/helper errors.
Ask the owner to reopen the disposable notebook after reload.
Use an exclusive host-timeout-bounded broker FIFO exchange with quoted heredoc
for each operation, following ../diagnostic/README.md. Record commands attempted,
uncertain writes and evidence in the parent report checkpoint before dispatch.
If startup fails, move only the spike outside extensions.d, restore the verified
probe, then use the supported recovery path and verify health. Preserve unrelated
extensions, QMDs, services and packages. No repeated load on the same failing build.
