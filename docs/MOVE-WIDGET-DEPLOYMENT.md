# Move widget deployment — 2026-10-26

## Scope and current evidence

hc authorized deploying the compact/dismissible widget, then documenting success
and preparing a commit. Only our own widget QMD was replaced and the separate
live-test extension added. No credentials/config/helper, reviewed run reference,
paid API request, native insertion, package upgrade or production promotion.

| Check | Observed outcome |
| --- | --- |
| Target | Move/Chiappa, app release 3.28.0.172, Qt 6.10.3 |
| Preparation | Exact SDK AArch64 build; hashed QMD applied to one verified root; SDK parsing and additions-only normalized diff passed |
| Local UI | Pointer tests passed for launcher, expand/collapse, busy Cancel, toolbar-aware ×/restore, missing-state fallback and capture suppression |
| Clean stop | Prior app stopped cleanly; inactive/MainPID 0/Result success before activation |
| Supported activation | New extension mapped and QMLDiff loaded the exact widget patch |
| Health | Stable new app process, zero restarts, same boot, root read-only |
| Integrity | All unrelated extension/QMD/hashtable hashes and old consumed markers matched baseline; only intended QMD changed and one extension added |
| Runtime | Read-only status: idle, Start disabled, no new Start/insertion claim |
| Actual notebook | Owner reopened disposable page; read-only inspect confirmed loaded notebook/native map |
| Actual launcher | Private screenshot independently decoded 954×1696 with matching device/local SHA256; 72×72 AI launcher at lower right, no ×, prior notebook fixture retained |
| Touch/recovery checks | After the requested expand/minimize, hidden-toolbar × and toolbar-show recovery check, hc confirmed: “Yes the widget behavior is acceptable.” This is owner acceptance, not automated per-action hardware measurement |
| Historical interaction-window frame | Widget absent while preceding telemetry had toolbar shown; these were not an atomic observation and were not proof of a restore failure. Subsequent owner acceptance closes the UI acceptance check; no frame-by-frame restore timing claim |

Final read-only verification after owner acceptance retained the same app process,
zero restarts, read-only root, unchanged unrelated hashes/consumed markers, and
idle/unarmed status. No new Start or insertion attempt was consumed.

The startup `Values`/`size` warnings match prior documented healthy loads; no
new startup fatal was found. This clean stop/start does not explain or fix the
historical shutdown SIGSEGV.

## Installed artifacts and recovery

- Extension SHA256:
  `6d6d6588258caeff52a2e139a3260c7aa28ea865ad347e495a1283a294dbfb2c`.
- Widget QMD SHA256:
  `aa68aaa58836a2014501ad355b005892b7a2f5dc2125c82d302a61755c538f6a`.
- The existing offline demo binary and its consumed run remain intact. Its old
  reviewed reference must not be reused for the new target.
- The new live-run reference is absent, so Start remains unarmed independently
  of any credentials that may exist elsewhere. Deployment is not authority for
  a paid question or an insertion.
- Focused rollback: quarantine only the added live extension **outside** the
  extension directory, restore our saved previous QMD, then use the supported
  XOVI startup route within authorized recovery scope. Do not reset markers,
  alter unrelated mods or weaken production gates.

Raw logs, firmware inputs, private screenshots, exact rollback locations,
TaskFlow, isolated commit index/checkouts and manifests are kept outside the
repository under the task report directory. They are not commit/publication
assets. The reusable [QMD preparation helper](../tools/move-live/build-qmd.py)
accepts private inputs and produces an uninstalled local candidate.

## Commit preparation

The live target depends on previously uncommitted capture/metadata/offline
foundation files. A standalone widget-only commit would be incomplete. Prepare
one coherent source/test/doc candidate including those dependencies, excluding
unrelated product-runtime edits, private task evidence, firmware resources,
generated binaries and credentials. The isolated candidate checkout passed live-text, offline-demo and offline-workflow
suites, including the pointer/toolbar checks. The resulting 51-file change set is
staged after owner acceptance, with a proposed commit message.
Private evidence/generated outputs and unrelated product-runtime work remain
excluded. No commit or push was performed during preparation; publication was
subsequently authorized separately by hc after review.
