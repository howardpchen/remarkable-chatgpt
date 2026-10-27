# Move steps 1–3 completion — 2026-10-26

> Historical scope report. Subsequent authorized deployment, one GPT-6 Luna
> request/insertion, and full-screen capture-only validation supersede the
> not-deployed/not-live-tested statements below. See [current readiness](MOVE-LIVE-READINESS.md).


Historical local-preparation report. The later authorized unarmed widget
deployment is recorded in [Move widget deployment](MOVE-WIDGET-DEPLOYMENT.md);
its hardware observations do not imply live API or insertion testing.

## Scope and reconciliation

Authorized scope: reconcile existing work, implement a narrow live-OpenAI test
path and verify locally. No deployment, tablet SSH operations, key provisioning,
paid request, new insertion, production gate change, commit or publication.

| Code/artifact | Actual capability | Evidence boundary |
| --- | --- | --- |
| `src/diary.cpp` / original product | Full API routing, rendering and original configuration loader | Not the loaded Move demo; production Move gate stays closed |
| `tools/move-demo` | Fixed-response, single-use offline capture/render/insertion | Prior bounded hardware result; consumed device markers remain untouched |
| `tools/move-live` | New direct OpenAI, single-use text-only asynchronous experiment | Local mock checks and exact-SDK build only; not deployed or live-provider-tested |

The initial tree had 71 changed/untracked files. A recoverable local archive,
per-file SHA256 manifest and tracked binary diff were saved **before edits** in
`build/local-preparation-20261026/`. No files were staged or committed. Sixty-one
prior files remain byte-identical; ten intentionally updated files cover shared
controller/transport seams, regression tests and status docs. A task-only patch
relative to that snapshot and `change-scope.json` distinguish the new work from
the prior uncommitted port work. New source lives in `tools/move-live` and
`src/api-request.h`; this completion report is also new.

## Implementation

The controller and renderer are shared behind an explicit compile-time split;
there is no configuration flag that turns the old demo into a live app. The
separate live build has its own broker signal, exported function, QML control,
artifact and run directory. Production generic API refusal is retained. Shared
OpenAI request construction keeps the existing endpoint/header/redirect policy;
the live transport is asynchronous to avoid running the synchronous helper's
nested event loop on the GUI thread.

The new path freezes the first UI-free crop, restores controls, performs one
text-only request, validates the strict response, renders, then obtains the
second suppressed capture and checks exact page/answer pixels before native
insertion. Existing page/viewport/native pose, fit, resource, durable attempt
and insertion claims remain in force. New reloads cannot arm a consumed Start.
No retry/reset/fallback, search, image-generation or specialized visual route.

Model remains `gpt-4.1-mini`, 800 output-token limit and `store:false`.
Current official documentation marks the model deprecated; supported feature
list is not proof of live availability or exact application behavior. No model
was silently changed. See [contract, build and next milestone](../tools/move-live/README.md).

## Local verification results

| Check | Observed result |
| --- | --- |
| Integrated live runtime | **21/21 scenarios passed**, synthetic captures/fake native pose/mock transport |
| Transport | **12/12 scenarios passed**, including immediate completion, dispatch/context guards, timeout, HTTP redirect rejection, oversize/refusal/incomplete/schema errors, cancel and teardown |
| Config | Missing/valid private file, symlink, public permissions and incompatible model checks passed with synthetic credentials |
| Live QML | Actual pointer Start/Cancel, no on-load Start, exact footprint and suppression binding passed with fake broker |
| Existing demo suite | Admission, generic Responses/Images refusal, ten integrated runtime scenarios, QML and native-pose checks passed |
| Existing offline workflow | Capture/render/verify/one mock insertion and stale/cancel/content/duplicate guards passed |
| Full `test.sh` | Passed, including API cancellation, policy, mapping/template, capture/layout, append/viewport, five Python port tests, response/font, plot, web, Markdown safety, config, templates, visuals and calendar checks |
| Exact Move SDK | Both offline and live extension builds passed, AArch64/Qt 6.10.3, unique generated XOVI link entries |
| Artifact inspection | Live artifact exports `ink_diary_move_live` but not the shared raw workflow; rebuilt offline artifact exports demo control and imports **no QNetwork symbols** |
| Rendering | Inspected white-background composition of successful synthetic answer; readable Caveat text, not handwriting-recognition evidence |
| Hygiene | Shell syntax and `git diff --check` passed; earlier unrelated files retained |

Host Qt was the existing extracted Qt 6.4.2 tree under
`/tmp/remarkable-port-qt/root`; no packages were installed. Initial default-env
attempts lacked Qt/plugin paths, then the existing documented host environment
was used. Target build used the existing exact Chiappa 3.28.0.172 SDK. Logs,
synthetic preview, archive, task-only patch, manifest and scope inventory:
`build/local-preparation-20261026/`. Build metadata and SHA256 are in
`build/move-live/ink-diary-move-live.json`.

## Remaining milestone

Prepare exact-resource live QMD and controlled deployment/recovery; obtain scoped
hardware/reload/insertion and live-spend approval with a spending ceiling. Stage
and verify the hidden key-entry setup, fresh private run directory/fonts, new
reviewed actual native pose/resource/artifact hashes and a fresh Blank page.
Neither the intended config nor helper was created or verified on hardware by
this local task. Never reuse old references or reset consumed offline markers.

Then verify one real handwritten timeless question, useful answer, fit/placement,
Undo/Redo/reopen and device health/unrelated hashes. No such outcome is claimed
here. One request/800 output tokens is not a dollar billing cap. Earlier hardware
shutdown SIGSEGV remains unexplained; host lifecycle tests are not a hardware fix.

## Follow-up: compact idle launcher and toolbar-aware dismissal

Per hc's UI refinement, local live QML now defaults to a 72×72 launcher. A tap
opens the 360×176 controls (not an API request); − collapses them. Busy requests
stay fully expanded with Cancel, and completion/cancel returns to the launcher.
The expanded idle panel offers × only while the native toolbar is hidden; ×
dismisses everything until toolbar display restores the launcher. The collapsed
launcher never shows ×. Unknown toolbar state disables dismissal. Capture
suppression takes precedence and admission still rejects the collapsed size;
no capture or native-pose guard was relaxed.

Focused real-pointer host tests cover default idle bounds, expand/collapse,
non-starting launcher, exact expanded admission, busy Cancel and completion,
dismissal/recovery/repeated toolbar cycles, suppression and missing bridge.
Only live QML/shared host test/test runner/docs changed. No tablet changes, paid
requests or loaded-demo QML changes. Actual hardware UI review remains pending.
Logs: `build/widget-dismissal-20261026/`.

## Follow-up: owner-selected GPT-6 Luna

The owner requested replacing the deprecated GPT-4.1-mini pin with
`gpt-6-luna`. The local live client, private-config validation, synthetic live
fixtures and build metadata now use the new model. Requests explicitly set
`reasoning.effort` to `none`, preserving the prior non-reasoning behavior,
800-token output ceiling and strict single-message text parser. The Responses
endpoint, image input, JSON schema and one-shot guards are unchanged. Official
model documentation confirms these capabilities:
https://developers.openai.com/api/docs/models/gpt-6-luna.

This supersedes the earlier model choice in this historical report, not the
previously observed hardware state. No tablet deployment, credential provisioning
or paid API request occurred as part of this change. The installed artifact still
has its prior model pin; it needs rebuilding and reviewed redeployment before the
live test. Account access and real handwriting quality remain unverified.

Local migration verification passed: 21 integrated live-runtime scenarios, 12
synthetic transport scenarios, private-config checks (including rejection of
the old model), and live QML interaction/binding checks. The request assertion
checks explicit `reasoning.effort:none`. Shell syntax and `git diff --check`
passed. Log: `build/move-live-host/luna-test.log`. No new exact-SDK cross-build
is claimed for this model change.
