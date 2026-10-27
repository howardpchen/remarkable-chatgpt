# Bounded Move live-text experiment — unarmed widget deployment

This is a **separate, unarmed test artifact**, not production Move support.
The extension/widget is now installed on the exact Move release; no API key or
reviewed live-run reference was provisioned by this deployment. See
[deployment verification](../../docs/MOVE-WIDGET-DEPLOYMENT.md) for observed UI
evidence and owner UI acceptance. Installation is not live-provider validation.
It reuses the tested 360×176 controller, exact native-pose admission, rm-shot
capture suppression, Markdown renderer, scene scaling and insertion guards.
The original `tools/move-demo` target remains offline-only; the shared generic
Responses/Images helper continues to reject experimental Move calls. The new
client deliberately uses a separate compile-time target, not a config bypass.

Local completion/evidence: [Move steps 1–3](../../docs/MOVE-LIVE-PREPARATION.md).

## Contract

- One explicit Start, one cropped 834×1584 image, one direct OpenAI Responses POST.
- Model fixed to `gpt-4.1-mini`; `store:false`, 800 maximum output tokens.
- Strict JSON `{kind: "text" | "unsupported", text: string}`, checked locally.
  Incomplete output, refusal, extra fields, wrong types, tools, non-text routes,
  empty/oversized text and malformed envelopes are rejected, not inserted.
- No search, image generation, plots, specialized diagram/template routing,
  fallback model, redirect following, automatic retries or on-load Start.
  Text may use the existing local Markdown renderer; no external resources load.
- Restores our control before asynchronous network wait. Cancel is active;
  page/controller/view/mapping/template/native UI changes abort the reply.
  Cancellation cannot retract data already sent or guarantee no provider charge.
- 120-second HTTP timeout, 150-second overall timeout, 1 MiB response ceiling.
- Frozen question pixels are retained. A second UI-suppressed capture must match
  exactly immediately before insertion; answer-file pixels must also match.
  Fit and native image budgets are unchanged. A dispatch is not visual proof.
- QObject-owned asynchronous client, no detached worker/nested event loop;
  teardown disconnects/aborts pending reply and restores the capture lease.
  Host teardown tests do not resolve the earlier hardware shutdown crash.

## Idle launcher and dismissal (local QML)

Idle defaults to a 72×72 AI launcher at the lower right. Tapping it opens the
360×176 controls without starting an API request. The − returns to the launcher.
Starting a request keeps the full panel/Cancel accessible; completion or cancel
returns to the launcher. Neither − nor × is offered while busy.

The upper-right × is offered only in the expanded, idle panel while the native
`toolbar.shown` is false. It dismisses the entire widget (including launcher)
until the native toolbar is shown again, which restores the idle launcher.
The recovery binding works while the widget/status polling is hidden. Showing
the toolbar clears dismissal; hiding the toolbar again does not auto-dismiss.
There is no × on the collapsed launcher and no native-toolbar write.
Missing/unknown toolbar state keeps × unavailable while expansion/collapse remain
usable. The existing read-only `diaryNativePoseObjects.toolbar` bridge supplies
the state; recovered native source defines `shown` as `visible && expanded`.

The **expanded** capture footprint remains exactly 360×176. The existing admission
checks reject a collapsed launcher; explicit Start is only in the expanded panel,
and busy keeps it expanded for both captures. Screenshot suppression wins over
all display/recovery states. No capture gate was relaxed for the new idle size.
The live QML is now deployed in our own replacement widget patch; the old
offline demo binary/consumed markers and original-product source remain untouched. Run `bash tools/move-live/test-ui.sh` for focused pointer/binding tests.
These use a fake native toolbar, not hidden-toolbar hardware calibration.
Deployment hashes and actual UI evidence are recorded separately; host checks
alone do not establish tablet interaction or a live answer.

## Intended on-device state (not provisioned by these scripts)

- New, separately reviewed private run directory:
  `/home/root/xovi/move-live-text-v1` (0700).
- New `reviewed-reference.json` for the actual page, viewport, native pose,
  effective resource and **new installed artifact hashes**, plus fonts. Never
  copy an old reference unchanged or reset/reuse the consumed offline run.
- New START_CONSUMED and INSERTION_CONSUMED claims. Old experiment markers stay
  untouched. Cancellation/failure/reload cannot replay a consumed Start.
- Intended credential file `/home/root/.config/ink-diary/config.json` (0600),
  owned by the running user, regular, not symlinked or hard-linked, max 16 KiB.
  Required fields: `api_key` and `model` (`gpt-4.1-mini`); no demo mode.
  No credential is read at load/status. The loader reads it only on Start and
  does not return it through the broker. Config endpoint overrides are ignored.
- New `MoveLive.qml` uses the distinct `ink-diary-move-live$control` broker
  signal and the exact tested footprint. `inspect`/`status` are read-only.
  The live extension exports only its control entry point plus XOVI metadata;
  the shared workflow export is local, not an externally available bypass.

The directory, config file, helper and live QMD are **not installed** by building.
Do not interpret the intended paths as evidence they exist on the tablet.
No standard installer or production firmware gate is changed.

## Local checks and build

Use a host Qt6 Core/Gui/Quick/Qml/Network/Test installation:

```sh
bash tools/move-live/test.sh
bash tools/move-demo/test.sh
bash tools/offline-workflow/test.sh
bash test.sh
```

Tests inject `LiveMockManager` (no sockets), synthetic notebook pixels and a fake
native pose. They never use real credentials. The client tests also cover schema,
private-file checks, redirect failure, immediate completion and reply lifetime.
The integrated runtime tests cover both capture boundaries, fit, cancellation,
network errors, in-flight context changes, destruction and persistent reload
refusal. QML tests exercise pointer Start/Cancel and suppression bindings.

```sh
SDK_ENV=/path/to/exact-chiappa-sdk/environment-setup-cortexa55-remarkable-linux \
XOVI_REPO=/path/to/xovi bash tools/move-live/build.sh
```

Output: `build/move-live/ink-diary-move-live.so`; the build never deploys it.
Build requires exact AArch64/Qt 6.10.3 SDK. The reviewed native lexical binding map
remains in `tools/move-demo/NativePoseBindings.qml.inc`. An exact-resource QMD
must be prepared and reviewed against current hardware before deployment;
`MoveLive.qml` alone is not an installable firmware patch.

Prepare the exact-release patch locally with the firmware's actual hashtable and
private QML tree (the output directory must be new):

```sh
python3 tools/move-live/build-qmd.py --qmldiff /path/to/qmldiff \
  --hashtab /private/device-hashtab --qml-root /private/source \
  --output /private/new-patch-review
```

The QMLDiff parser used on this firmware treats `typeof` as an ordinary identifier;
parenthesize the complete multiline `typeof` bindings. A component accepted by Qt
can otherwise fail patch parsing. The helper hashes and reapplies the candidate,
requires one widget/map, and never activates it. Compare normalized original and
patched QML with the matching SDK before any later deployment.

## Next milestone — live API still unarmed

Keep the earlier unexplained shutdown crash in mind for any future reload.
Widget deployment does not authorize insertion or API spending: obtain an explicit
spending ceiling, and a disposable fresh Blank page. Harden/stage the private
key-entry helper without overwriting other config; then verify actual paths and
permissions. Do not ask the owner to enter a key into a missing helper.

After controlled deployment, remeasure native UI and artifact hashes, arm the
fresh reference, and perform one ordinary timeless handwritten question. Verify
actual useful recognition, answer placement/readability, Undo/Redo, reopen
persistence, health and unrelated mod hashes. Never automatically repeat an
uncertain call or insertion. This is not approval to perform that milestone.
One-request/token ceilings bound traffic but are **not a dollar billing cap**.

## Official contract checked 2026-10-26

- https://developers.openai.com/api/docs/guides/structured-outputs
- https://developers.openai.com/api/docs/models/gpt-4.1-mini

The current model page lists image input, Responses and structured outputs but
marks GPT-4.1 mini **deprecated**. Preserve the selected model here; availability
and exact application-shaped behavior still need the separately authorized live
probe. There is deliberately no silent substitution.
