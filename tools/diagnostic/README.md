# Move runtime metadata probe (diagnostic only)

This separate XOVI extension is **not** the ChatGPT plugin and does not enable
Paper Pro/Move support. It never captures screenshots, reads page IDs or text,
calls image insertion, reads credentials, or connects to a network. It does not
install a QMD patch or create a service. Loading alone performs no probe.

Load and every probe are gated on all of:

- `/sys/devices/soc0/machine`: `reMarkable Chiappa`
- `/etc/os-release`: exactly one `IMG_VERSION=3.28.0.172` (quoted accepted)
- runtime Qt: `6.10.3`
- process symbols: Qt resource registration plus the broker's DeviceLock symbol
  `_ZNK3csl10devicelock10DeviceLock3setENS1_5StateE`
- XOVI conditional module availability: `xovi-message-broker$broadcast`

The probe reads visible window dimensions/DPR, the `documentLoaded` boolean,
object references required to inspect a small whitelist of method signatures
(`viewToScene`, `insertImageFileAsSceneItem`), and returns a finite/nondegenerate
mapping boolean. It does not return scene coordinates, object names, content,
paths, IDs, controller bounds, or arbitrary property/method dumps. A temporary
QtObject uses only read-only coordinate mapping. Discovery runs on the UI thread;
large object trees are bounded at 100,000 objects. It can briefly delay the UI.
Metadata signatures alone do not prove that native insertion works.

## Local build (does not deploy)

```sh
SDK_ENV=/absolute/path/to/chiappa-sdk/environment-setup-cortexa55-remarkable-linux \
XOVI_REPO=/absolute/path/to/xovi \
tools/diagnostic/build.sh
```

Outputs: ignored `build/diagnostic/`, `bin/chiappa/ink-diary-probe.so` and
`bin/chiappa/diagnostic-build.json` with SHA-256 source/artifact manifest.
The build checks Qt version/compiler target and actual AArch64 ELF64 headers.
It uses Qt Core/Gui/Qml/Quick; the QtQml dependency brings Qt Network into
the ELF dependencies, but this code does not construct any network client.

Host-only policy regression (Qt6Core development files required):

```sh
c++ -std=c++17 -fPIC tools/diagnostic/test-policy.cpp -o build-host/diagnostic-policy $(pkg-config --cflags --libs Qt6Core)
./build-host/diagnostic-policy
```

## Process-scope correction after failed startup attempt

The initial diagnostic artifact's model/firmware/Qt gate was insufficient:
XOVI preload reached xochitl subprocesses with the same device tuple while the
broker correctly excluded them. Parent observed broker threads in PIDs
847/915/969, but fatal missing-broker dependency initialization in different
PIDs 888/942/996. No probe FIFO command ran. The initial artifact is **not safe
to redeploy**. Parent restored the prior mod state before this local correction.

The corrected artifact mirrors the broker's DeviceLock process predicate in
both `_xovi_shouldLoad` and its XOVI conditions, additionally requiring the
broker's `broadcast` export as a conditional dependency. XOVI Pass2a cascades
conditional unloads before dependency constructors run. Host policy tests cover
all combinations of the two required process-symbol predicates. These offline
checks do not constitute a successful runtime retry.

## Trigger after separately authorized temporary deployment/loading

No install/restart command is provided or executed by this build. The parent
operator must preserve prior extension state, deploy only this diagnostic,
and use the supported XOVI workflow. This extension requires broker ABI 0.2.0.
Vellum package revision (for example 19) is not the broker ABI version.

Once already loaded, use the broker's native FIFO endpoint; no QMD is needed:

```sh
# Confirm the existing broker FIFOs before invoking. Do not create replacements.
test -p /run/xovi-mb && test -p /run/xovi-mb-out
printf '%s\n' '>eink-diary-probe$probe:' > /run/xovi-mb
cat /run/xovi-mb-out
```

Use an operator-side timeout (e.g. 10 seconds) around the complete exchange.
This is an exclusive broker exchange: do not run another broker consumer in
parallel. A missing handler, multiple handlers or stalled UI can leave the FIFO
read waiting; timeout is not proof that the handler did or did not run. The
probe is read-only, but inspect state before retrying or claiming an outcome.

Verified upstream protocol:

- <https://github.com/asivery/rm-xovi-extensions/blob/6c709dd8416ddf89034831073d76fb4ee3372905/xovi-message-broker/README.MD>
- `xovi-message-broker/src/pipes.cpp`: `>` requests output, `e` dispatches native,
  colon separates signal/value, newline completes input; returns only for one hit.
- `xovi-message-broker/src/messagebroker.cpp`: version-1 simpleSignal handlers
  return malloc-compatible strings, which the broker frees.

No actual tablet load, FIFO call, runtime metadata or image placement has been
verified by these local build/policy checks.
