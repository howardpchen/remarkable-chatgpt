# Move per-request personal beta (v4)

This separate experimental build was installed and accepted for narrow personal
beta use on 2026-10-27; it is **not production Move support**. The installed variant
uses a single capture. Blank, Dots small and Grid small are in scope, portrait at
normal zoom, on the exact reviewed OS 3.28.0.172 / Qt 6.10.3 setup. Both small
templates passed real recognition and placement without overlap, plus owner
Undo/Redo/close-reopen. Two requests consumed two slots, with one slot unused;
no ongoing-use budget or quota increase has been provisioned. Keep the page still
while thinking because single capture cannot detect new ink on that same page.
The historical shutdown crash remains unexplained despite later clean restarts.

The following development sections retain their original local/historical scope;
statements that hardware validation is pending are superseded only for the narrow
accepted cases above, not for arbitrary templates, zoom or general answer quality.

 The owner accepted v2's answer, Undo, Redo and reopen behavior. Its binary,
QMD and consumed markers remain the validated baseline; never reset its runs.

## Deliberate requests and bounded usage

- Only an explicit Start begins an attempt. There is no on-load Start, queue,
  automatic retry, redirect, model fallback or automatic quota replenishment.
- One active request at a time; duplicate broker Starts during busy do nothing.
  The visible busy button remains Cancel. After a terminal result, a new Start
  explicitly requests a **new** attempt; it never replays the old identity.
- A separately provisioned private root `/home/root/xovi/move-session-small-templates-v4` holds
  a reusable schema-3 `device-profile.json` and fonts. Every Start reserves a new private
  numbered directory (1, 2, 3) using atomic mkdirat, then fsyncs the parent before
  capture/networking. Occupied entries, including symlinks, count as consumed.
  Allocation/sync failure fails closed. Cancellation/failure/reload never refunds
  or removes a directory. Existing v1/v2 markers and evidence stay untouched.
- Each directory retains independent START_CONSUMED/INSERTION_CONSUMED guards,
  frozen question, comparison capture and rendered answer. All original native
  pose, installed-artifact, supported-template/portrait/zoom, pixel equality and fit checks
  remain. The device profile must match the installed artifact hashes; it is
  provisioned once for a reviewed device/build, not uploaded for each question.
- Three attempt slots persist across process reloads; each request retains the
  fixed Luna model, one full-screen image, 8 MiB encoded-image limit, 800 output
  tokens and strict text schema. Thus at most 3 POSTs / 2400 requested output
  tokens per provisioned root. Attempts failing before POST also spend a slot.
  **This is a request/token quota, not a provider-enforced dollar cap.** Do not
  treat the old consumed $0.50 authorization as a budget for this candidate.
  Approve the proposed test's total spending separately; verify then-current
  model/image pricing before dispatch. No billing-API integration is claimed.
- Completion/error text remains expanded until explicitly collapsed, with Cancel
  available while busy and remaining slots displayed on Start. No automatic
  clearing of an error and no hidden retry. Capture suppression still wins.
- The quota is deliberately small for the next test. No refill command or
  production installer exists; increasing it or provisioning another root is a
  separate reviewed change, not an error recovery operation.

## Shutdown evidence, not a crash fix

This build emits content-free `ink-move lifecycle` journal milestones at runtime
creation, reservation, dispatch boundary, POST return, reply completion/abort,
terminal stage, aboutToQuit/cancel completion and destructor body boundaries.
No question text, page ID, provider response, key, URL or HTTP headers are logged.
A destructor-body-end marker does not prove QObject child teardown completed.
Missing/buffered journal messages do not establish that a stage was not reached.

`collect-shutdown.sh` is read-only. Capture it before and after **one separately
authorized** app restart; it reports service state, boot/core-routing metadata,
and bounded filtered current/previous-boot lifecycle records. It does not enable
core dumps, replace Memfault, change OnFailure, add services or alter logging
configuration. Do not save unrestricted core dumps: process memory can contain
credentials and private notebook content. The earlier crash root cause remains
unidentified; host tests and a clean subsequent restart do not establish a fix.

## Local build and verification

```sh
bash tools/move-session/test.sh
SDK_ENV=/path/to/exact/chiappa/environment-setup-cortexa55-remarkable-linux \
XOVI_REPO=/path/to/xovi bash tools/move-session/build.sh
```

Output is isolated under `build/move-session`, never `build/move-live`. The SO
retains the existing live broker ABI and must replace—not coexist with—the live
extension during a later approved activation. QMD preparation uses the existing
helper with `--widget tools/move-session/MoveLive.qml`, plus the firmware's actual
hashtable and original private QML tree. Build/QMD preparation never deploys.

Checks: all 22 inherited live mocked runtime scenarios plus aboutToQuit during an
in-flight API request; consecutive successes,
explicit recovery after cancel/HTTP failure, busy duplicate rejection, no extra
calls after persistent quota exhaustion/reload; concurrent quota claims and
permission/symlink refusal; actual candidate-QML pointer/capture/restore checks.
Keep original one-shot, offline demo, workflow and repository regression suites.
These use synthetic pixels/credentials and a mock transport, not live-provider
or hardware proof. Private runtime reports and notebook captures are not publication assets.

## Capture frequency and transport diagnostics

A normal successful Start takes two full-screen screenshots: `question.png`
before inference (the only image sent to the model), then `check.png` after the
answer returns and before insertion. The second is local-only and verifies that
ink did not change while inference was running. API failure ends after the first
capture. Status/pose polling does not take screenshots. Manual before/hidden/after
capture-review and result screenshots are test instrumentation, not a continuous
runtime capture loop; do not repeat that review solely to investigate a few
settling pixels. A future single-capture path needs a verified ink/content revision
signal, not merely stable page identity.

The local diagnostics revision distinguishes HTTP status from numeric Qt network
error and displays only an allowlisted provider error code, if present. The
per-request build journals the same safe summary. It never logs response messages,
headers, URLs, credentials or raw bodies. It adds no retry, quota refill or change
to the model/payload. The previously deployed generic error cannot be reconstructed
from this new code; another live request requires fresh spending authority.

### Single-capture variant (installed beta; original trial design)

Build/test with `SINGLE_CAPTURE=1 bash tools/move-session/test.sh` and
`SINGLE_CAPTURE=1 SDK_ENV=... XOVI_REPO=... bash tools/move-session/build.sh`.
This selects a compile-time trial, writes isolated `move-session-single-capture`
outputs, and leaves the ordinary two-capture build available. It does not deploy,
create a new run root, reset markers or extend the exhausted three-request quota.
After answer rendering, the trial rechecks the visible native UI, page identity,
template and viewport, then inserts directly without a second screenshot or a
second control-hide/restore cycle. It retains frozen question/answer integrity,
fit checks, native insertion signature and durable insertion claims. New ink on
the same page is deliberately not detected; do not write while it is thinking.
The original two-capture build still refuses same-page ink changes. Both paths
are covered by synthetic integration tests; hardware validation is still pending.

## Start without manual page arming

The new per-request build reads one private 0600 `device-profile.json` from its
session root. Schema 3 retains the accepted device `captureArea`, normal-zoom
`viewport` calibration, visible/hidden native UI poses, effective resource SHA256,
and installed binary/QMD manifest. An old schema-2 per-page reference is not
silently promoted, and a missing profile is not synthesized from arbitrary live
state. Provisioning/review belongs to a future approved installation; never copy
old artifact hashes onto a changed build or reset the exhausted attempt root.

At each explicit Start the runtime reads the current supported-template viewport,
checks portrait/DPR and affine geometry against the retained normal scale, then
freezes that current viewport and page/controller identity in memory. Different
page scene origins are accepted; zoom, rotation/shear and subsequent viewport or
page changes reject. Device/UI/resource/installed-mod checks remain unchanged.
No question pixels, notebook ID or page ID are stored in the reusable profile.
The test suite switches pages and scene origins across three Starts using one
byte-identical profile and confirms no `reviewed-reference.json` is needed.
No screenshot/calibration upload or JSON file edit is required between questions.

The ordinary one-shot live and offline artifacts retain their prior reference
contracts. Only the per-request QML replaces “not armed” with setup/test-limit
status; it still never Starts automatically. The single-capture trial remains
optional and retains its acknowledged same-page-ink limitation.

## Readable non-answer outcomes (local revision, 2026-10-27)

The same image-reading POST now routes four explicit kinds. `text` keeps the
existing Markdown/render/insert path. `needs_web` displays **Needs web access;
search is off.** `clarification` displays **Cannot read question. Please rewrite.**
`unsupported` displays **Text answers only; try another question.** These are fixed
local status messages, not provider transcriptions; provider text for non-answer
kinds is discarded. They remain expanded in the per-request UI until dismissed.
All three stop after the first capture without rendering/inserting notebook
content, taking a second capture, searching, retrying or refunding the attempt.
A rewritten question needs another explicit Start and consumes another slot.

The prompt requests clarification for missing/unreadable/ambiguous latest questions
and a web outcome for current/latest facts or source lookup. This is model routing,
not deterministic detection of every web-dependent or unreadable question; no
live-provider recognition claim follows from synthetic tests. Unknown kinds,
malformed output and refusals remain failures. Model/token/capture/quota settings
and production gates are unchanged. The shared bounded one-shot client also uses
these routes; original production diary.cpp behavior is not changed here.

Both capture-mode runtime suites now cover 32 scenarios, including fixed status
text, no insertion marker/second capture, one POST, unknown-kind rejection and
unchanged remaining quota. Actual candidate QML checks confirm the three notices
fit the status area both with and without the dismiss control. Host transport
checks ensure non-answer provider text does not reach status. No hardware or paid
call was made for these changes.

## Small factory templates (local revision, 2026-10-27)

The per-request candidate admits exactly `Blank`, `P Dots S` (**Dots small**) and
`P Grid small` (**Grid small**), verified read-only from the current page. This
allowlist does not extend legacy one-shot or offline artifacts. A pending switch
between two supported templates still cancels: the exact template identity is
part of the frozen viewport. Existing normal-zoom/portrait/geometry/UI gates stay.

The model receives the original screenshot, including its template; the frozen
question and two-capture pixel comparison are unmodified. Placement alone ignores
verified repeating small-grid bands or isolated small-dot components on a broadly
repeated lattice, with spacing derived from the factory asset and current scale.
Dark ink on pale grid lines and handwriting joined to dots are retained. A missing
or unrecognized pattern is retained as ink, potentially causing a no-room refusal
rather than assuming space is empty. No extra model call or retry is added.

This is not perfect semantic ink separation: a tiny isolated mark exactly on a
factory dot, or a stroke exactly matching a dark grid rule, is visually ambiguous.
Use ordinary handwriting crossing the pattern, not template-aligned single-pixel
marks as the only question content. Dense content can defeat pattern detection.
Real template rasterization, recognition and answer placement still need device
acceptance at normal zoom; synthetic fixtures are not hardware proof.

Native firmware exposes `sceneController.setBackgroundVisible(...)` via its layer
menu. The current rm-shot capture reads the composite framebuffer, with no
layer-exclusion option. Automatic background hiding is not implemented: a
capture-only rendering override/restoration contract has not been verified, and
we do not toggle the notebook's layer settings merely to take screenshots.

### Grid small native dither (2026-10-27 follow-up)

A real Move framebuffer capture renders gray factory grid rules as binary
black/white dither with roughly one-third black coverage along a line. Continuous
gray-rule fixtures alone are insufficient. Grid-band candidates therefore accept
>=25% dark coverage only together with <=4-pixel band thickness, at least four
bands spanning >=75% of the dimension, and every interval matching the factory
52-scene-unit pitch within two pixels. Inconsistent/extra handwriting bands
reject the lattice rather than being removed speculatively. Tests include binary
1-in-3 dither, original-pixel immutability and handwriting/punctuation retention.
The real capture moved from a false bottom row1693 to the actual ink bottom384.
This is placement-only filtering, never alteration of the image sent to the LLM.
