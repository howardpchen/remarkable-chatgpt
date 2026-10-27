# Move UI-only notebook probe — NOT deployable yet

This is a **local specimen**, not production support or a live recognition test.
No `.qmd` file is supplied: `probe.qmd.in` deliberately contains unresolved
selectors. Do not copy it to the tablet or rename it as a loadable patch.
Production `inkDiary.qmd` is unchanged. Its old hash values do not prove the
DocumentView resource/container on Move software 3.28.0.172.

`MoveUiProbe.qml` uses only QtQuick, local properties and one click handler.
There is no broker, Timer, startup handler, Loader, URL, image source, network,
file access, native helper or insertion. Start/Cancel toggles **simulated** busy
status only. Nothing starts on load, including status polling. Backend
cancellation remains a separate untested gate.

## Exact-target gate before creating any loadable QMD

Parent owns authenticated device access, target identity, supported load and
rollback. This preparation never contacts the tablet. Require all of:

1. Fresh evidence of Chiappa / 3.28.0.172 / Qt 6.10.3; read exact `IMG_VERSION`
   from `/etc/os-release`. QRR's `VERSION` gate compares `IMG_VERSION`, not an
   assumed semantic-version string. It does not gate hardware or Qt: enforce
   those in the external preflight too. Do not guess this token.
2. Identify the installed qt-resource-rebuilder version and existing hashtab
   size/SHA256 plus its exact-target provenance. Enumerate only its format and
   DocumentView resource/container mappings locally. A table entry alone proves
   a name mapping, **not** that the target resource contains the selected node.
3. Query only the live resource registry for DocumentView candidate paths, then
   read just that QML resource (plus a minimal parent snippet if ambiguous).
   Record resource SHA256, exact matching container count **one**, parent visual
   width/height, ownership and visibility during notebook open/close. Do not
   read private notebook data, unrelated resources or process memory. If no
   supported read-only query exists, report the gap before any reload/extraction.
4. Resolve resource and traversal from that exact file and compare the installed
   QRR/QMLDiff syntax. An older hash can be reused **only if independently
   resolved/verified here**; never because the patch happened to load elsewhere.
5. Apply a generated candidate to a private copy of the resource on the host
   with matching QMLDiff, inspect the diff for exactly one inserted child, and
   lint emitted QML. Resolve the template by inlining the Rectangle from
   `MoveUiProbe.qml` without its import. Keep proprietary resource content local.
   Do not wholesale-replace the production DocumentView resource using QRR.

Minimal read-only parent metadata: device model, `IMG_VERSION`, Qt version,
QRR package version, `stat`/SHA256 of existing QRR hashtab, and bounded matching
DocumentView resource path/hash/container evidence. `rebuild_hashtable` is **not**
a read-only query: upstream stops xochitl and deletes/regenerates the table.
Do not invoke it merely to inspect selectors. Resource query/export mechanisms
must be established first; none is claimed to exist in the current probe.

## Narrow-screen and startup validation

At verified portrait 954×1696 the overlay is 360×176, right/bottom margins 16:
x578..938, y1504..1680. Button is 336×76; 22px status wraps inside 336×60.
At minimum visible parent width320 it shrinks to288 with264px content width.
Smaller-than320×352 parents hide it. These are parent-coordinate calculations,
not proof of actual target parenting or touchscreen legibility. No full-screen
MouseArea: only the visible button intercepts clicks. The overlay temporarily
occludes the lower-right page; avoid destructive notebook tools underneath.

Host: SDK Qt6.10.3 `qmllint`/`qmlformat` can validate the standalone specimen;
QMLDiff syntax/selector matching additionally requires actual target evidence.
Device gate: preserve unrelated QMD/module hashes and consumption markers;
stage only one uniquely named temporary patch, use supported reload once, verify
stable PID/zero restart growth/root read-only and no new parse/import errors;
reopen designated disposable notebook after reload. Check one button only in
notebook, status legibility/no overflow, local Start then Cancel, notebook
close/reopen and app health. No API/insertion endpoint may be connected.

Rollback: remove/quarantine **only** this new uniquely named patch outside QRR's
scan directory, supported reload, confirm overlay absent and unrelated hashes
unchanged. Preserve pre-existing configuration and all consumed markers. Do not
promote production support after this test: live recognition/backend cancel,
template-aware placement, offscreen output and regression gates remain.

## Provenance

* Existing local `inkDiary.qmd`: old 3.27.3.0 candidate only.
* `asivery/rm-xovi-extensions` commit
  `6c709dd8416ddf89034831073d76fb4ee3372905`, QRR README,
  `src/systemversion.c`, `rules.qrl`, and `xovi-setup/rebuild_hashtable`.
* QMLDiff pinned submodule `a38cccc4e0de99c0fc7f8cb8bf963a1c234267b1`,
  [README.MD](https://github.com/asivery/qmldiff/blob/a38cccc4e0de99c0fc7f8cb8bf963a1c234267b1/README.MD):
  VERSION must precede all other directives; apply-diffs supports local trees.
  README fetched from the immutable public raw URL; no device access.

The new specimen is original QtQuick code; no proprietary resource is embedded.

## Exact-target parent follow-up (2026-10-26)

Parent independently obtained only the embedded DocumentView resource via a
fixed-path Qt diagnostic, verified three chunks against its whole-file SHA256,
and applied the patch privately with pinned QMLDiff a38cccc4e0de99c0fc7f8cb8bf963a1c234267b1.
Exact target is FocusScope#root in /qml/device/view/documentview/DocumentView.qml.
The current device hashtab resolves these to the previously listed hashes;
this is now verified rather than assumed. Exact IMG_VERSION is3.28.0.172.
Normalized before/after diff contains only one new overlay; source has no deleted
or modified lines. SDK qmlformat accepts resulting QML. Standalone qmllint warns
that generic QQuickItem parent has no documentLoaded property; actual verified
FocusScope declares it. The overlay now explicitly requires documentLoaded=true.

Temporary patch was staged, but the old app crashed during shutdown and the
device rebooted before new-load evidence. Patch and template diagnostic were
quarantined and baseline mods restored. NO successful UI load or interaction
is claimed. Source template stays unresolved/not deployable by default; verified
candidate and proprietary resource remain only in private task reports. First
review shutdown failure before another activation. Parent owns that gate.

## Parent host interaction check

`bash tools/move-ui-probe/test.sh` builds a real QQmlEngine/QQuickWindow test
against host Qt6. It creates the unchanged overlay under a declared
`documentLoaded` parent, delivers actual pointer Start/Cancel clicks, checks
no on-load action, document hide/show, geometry954×1696 and320-wide, rejection
below minimum width/height, and destruction. All passed on offscreen Qt6.4.2.
Host QML runtime modules were extracted into the existing task-local Qt directory,
not installed globally. This is host evidence only: target rendering/touch and
shutdown safety remain device gates. It does not diagnose the earlier SIGSEGV.
