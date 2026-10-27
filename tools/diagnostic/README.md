# Runtime metadata probe (diagnostic only)

## Paper Pro portrait preparation — 2026-10-27

An explicit **separate Ferrari diagnostic build** is available for the observed
`reMarkable Ferrari` / `IMG_VERSION=3.28.0.172` / Qt `6.10.3` tuple. The default
build and all historical Move evidence below remain Chiappa-only. Each binary
rejects the other model; neither enables the AI runtime or any installer.

```sh
SDK_PRODUCT=ferrari SDK_RELEASE=3.28.0.172 \
SDK_ENV=/path/to/exact-ferrari-sdk/environment-setup-cortexa53-crypto-remarkable-linux \
XOVI_REPO=/path/to/xovi bash tools/diagnostic/build.sh ferrari
```

Outputs are isolated in `build/diagnostic-ferrari/` and `bin/ferrari/`.
Product/release are explicit operator declarations, not proof derived from
AArch64 or Qt alone; use the official Ferrari SDK matching the observed tablet.
The manifest records SDK environment/source/artifact hashes and explicitly marks
the module not deployed and not runtime-validated. Compilation does not prove
private notebook APIs or geometry work.

Ferrari supports window/method/mapping metadata and the bounded fixed
DocumentView resource queries described below. Template identity is returned as
`unvalidated-ferrari-resource`; `native-pose` refuses until Ferrari's actual
resource bindings have been independently inspected. Do not reuse the Move
lexical map, normal-zoom calibration, native-pose profiles or capture dimensions.
No screenshot, image insertion, provider request or spending operation exists in
this diagnostic. Loading it or adding its broker dependency requires separate
device authority and a reviewed rollback. Nothing here auto-installs/restarts.

The root host suite builds/tests both mutually exclusive policy variants.

## Historical Move diagnostic and evidence

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
paths, IDs, controller bounds, or arbitrary property/method dumps. A fixed synchronous JavaScript function in the existing engine uses only
read-only coordinate mapping, without QML component imports. It preserves the
existing UI object ownership. `mappingStatus` distinguishes initialization or
invocation errors (`mappingFinite=null`) from tested valid/non-finite/degenerate
mappings. Discovery runs on the UI thread;
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

## Runtime evidence and mapping-helper correction

The process-gated diagnostic loaded successfully; a library-only probe returned
954 x 1696 at DPR 1. On 2026-10-26 the owner opened a disposable notebook and a
single bounded probe confirmed `insertImageFileAsSceneItem(QUrl,QPointF)` and
`viewToScene(QVariant)` with `documentLoaded=true`. The same application process
remained stable with zero restarts. No capture, insertion or API call ran.

The original mapping check returned false while the journal reported
`QQmlComponent: Component is not ready`. That result is inconclusive: `setData`
can leave imports loading, and immediate `create` does not establish readiness.
The revised helper avoids component creation/imports entirely, evaluates only a
fixed read-only function, and reports fixed status categories. Host integration
checks use a real QQuickItem wrapper with a native mapping method and cover valid
transformed coordinates, degenerate/non-finite mappings, missing methods,
missing engines and unchanged object ownership. Run through `./test.sh`.
The revised helper was loaded with owner approval on 2026-10-26. After the
owner reopened the disposable notebook, one bounded probe returned
`mappingStatus=valid`, `mappingFinite=true` and `documentLoaded=true`. The
same app PID3096 remained active with zero restarts and root read-only.
This read-only check alone does not establish capture calibration or insertion
behavior. A subsequent **separate offline spike** passed one synthetic insertion
with owner-observed placement/scale, undo/redo and close/reopen persistence;
Qt window capture failed. See the [current evidence matrix](../../docs/PORT-STATUS.md).
The diagnostic described here still never captures or inserts anything.

## Bounded embedded-resource and template preparation (2026-10-26)

Local source now supports `document-view-resource` (metadata only) and fixed
`document-view-resource-chunk-0` through `-15` requests. Only the embedded
`:/qml/device/view/documentview/DocumentView.qml` resource is readable; no
caller-supplied path. Total cap1MiB, chunk64KiB, whole-file SHA256 in each reply.
Unknown requests fail closed. Proprietary output stays in private reports,
never in this repository or publication. Missing/oversize resources reject.

The resource-only diagnostic was hardware-verified:163922 bytes, three chunks,
SHA256 c4b4347578f972ee9ad182606edcb4de07ea8e3eee1f3ea6f9b5a6311f4ecf0c.
Initial128KiB limit rejected this file; the bounded chunk variant succeeded.
Host synthetic resources verified missing, valid and over-budget handling.

`readonly-template.h` follows this verified resource's read-only
`document.templateForPage(currentPage)` usage. Installed template catalog maps
Blank to filename `Blank`. Only blank/nonblank/unavailable is returned; no
page index, ID or arbitrary template name escapes. Host tests passed with a
real QQmlEngine and mock document, including missing/invalid context and retained
object ownership. Live template-status behavior is NOT yet verified.

The combined UI/template load was interrupted by the *previous* xochitl process
segfaulting during shutdown, followed by reboot. There is no evidence that the
new artifact initialized. It was quarantined; prior probe34222109...724a and
all baseline mods/QMD hashes were restored and mapped in stable PID865, zero
restarts, read-only root. Do not reload/retry until shutdown incident is reviewed.
Source capability is therefore newer than the currently loaded diagnostic.

### Local-only normal-reference metadata preparation

The diagnostic now includes a fixed numeric `viewport` on supported visible-view
candidates: view/window dimensions, DPR, and scene mappings of screen points
`(0,0)`, `(100,0)`, `(0,100)`. Missing window, hidden/zero-size view,
non-finite/degenerate/reflected/rotated/sheared mapping returns no viewport.
This is measured metadata, **not proof of normal zoom** or runtime eligibility;
the production experimental policy additionally requires uniform mapping and an
independently verified normal reference. Runtime snapshot template identity is
still unwired and fails closed. Nothing here promotes the default runtime.

`controllerOrdinal` associates document-controller and visible-view candidates
within **one response only**, without exposing pointers or document/page IDs.
Zero means no controller. Ordinals are traversal-local and must never be compared
between calls. A baseline is usable only with exactly one loaded active document
and one supported visible view associated with its nonzero ordinal; ambiguous or
truncated scans reject. Window geometry in the viewport must match the expected
Move window. Do not combine template status from one controller with another view.

Live metadata remains **UNVALIDATED**. The next device phase must activate only
the diagnostic artifact after a separately verified clean stop, keeping the
already validated UI patch unchanged. With a disposable notebook, collect Blank
at owner-confirmed portrait/normal zoom; change only to dots and require the
matching template status to change from blank to nonblank; restore Blank and
require blank. Change only zoom and require three-point geometry to change;
restore the exact normal viewport and verify all numeric fields return to the
baseline. Stop on unavailable/ambiguous metadata or unchanged geometry after a
confirmed zoom change. No capture, insertion, API, production promotion, or new
consumption-guard reset is part of this test.

## Local lifecycle regression coverage

Run `bash tools/diagnostic/test-lifecycle.sh` with a working host Qt6 development
and offscreen-platform environment. This exercises the actual read-only mapping
and template helpers (not copied implementations): 8 combinations of C++/JS
ownership, QObject parenting and engine-first/view-first destruction; 512 helper
calls interleaved with garbage collection; root/document ownership and exactly
once destruction checks. It also checks context-bound queued callback removal.

The harness verifies that `aboutToQuit` fires while the application still exists
and `QCoreApplication::closingDown()` is false. Therefore neither app existence
nor that flag alone is a complete late-intake guard for the current blocking
broker dispatch. No new dispatch guard is introduced by this test. Operators
must quiesce broker exchanges before a separately reviewed reload; do not submit
requests during application teardown. A context-bound queued callback being
removed after destruction does not prove an already-blocked caller can safely
wait through shutdown.

These host checks do not emulate XOVI/broker/other-extension unload, target
firmware objects, rendering threads, or the recorded tablet SIGSEGV. Passing
checks establish local ownership/lifetime behavior, not a shutdown-crash fix.
