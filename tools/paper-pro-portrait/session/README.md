# Ferrari AI session

Separate experimental full-size Paper Pro integration for Ferrari OS 3.28.0.172 /
Qt 6.10.3. Do not substitute Chiappa SDKs, resources, profiles or private state.
Native rM2 and Move integrations remain separate. PORT-SOURCES.json records the
original fork basis, not current deployment status.

## Supported candidate scope

Portrait 1620×2160 and landscape 2160×1620 reading coordinates, with independently
reviewed Blank/Dots small/Grid small profiles for each admitted direction.
Bounded zoom/pan is frozen throughout each request. Missing calibration, changed
page/pose/resource or unsupported firmware fails closed. The capture-owner
boundary remains exactly 360×176 even when the visible widget is narrower.

The guarded workflow retains explicit Start/Cancel, conservative persistent dollar
reservations, prompt caching, 800 output tokens and no automatic retries. Offline
key operations never refill allowance or verify credentials with a paid call.
No policy or funding is created by constructors, package hooks or key replacement.
Fonts, private profile and owner-funded policy are separate setup prerequisites.

## Build and verification

`build.sh` requires SDK_PRODUCT=ferrari, SDK_RELEASE=3.28.0.172, SDK_ENV pointing
to the exact Ferrari SDK, and XOVI_REPO. Use a fresh PAPER_PRO_BUILD_DIR and rebuild
capture dependencies with this SDK. Never use host moc or Chiappa binaries.
The optional `benchmark-png.c` harness also needs `stb_image_write.h` from the
external rm-shot checkout (supply its source include directory); it is not a
standalone build or a package payload.
`build-qmd.py` validates the separately reviewed original DocumentView resource;
firmware resources must remain private and outside Git.

`bash tools/paper-pro-portrait/session/test.sh` rebuilds synthetic profile,
template-ink, mock HTTP/runtime, native-pose and ledger tests. No real provider
sockets are opened. `bash tools/paper-pro-portrait/test-layout-ui.sh` exercises
both widgets with host pointer events, including both Ferrari landscape turns.
Host success is not target ABI, capture, native insertion or live placement proof.

See [Vellum setup](../../../packaging/vellum/SETUP-FERRARI.md) and
[release preparation](../../../docs/RELEASE-PREPARATION.md) for packaging and
verification limits. Local build/test does not authorize installation, restart,
trust changes, key transfer, paid calls or publication. Preserve app-owned private
state through migration and removal; do not load manual and packaged copies together.
