# Move live readiness — updated 2026-10-27

## Current accepted personal beta

The installed per-request v4 single-capture build is accepted for personal use on
Blank, Dots small and Grid small at normal portrait zoom. Both small templates
passed live recognition/placement and owner Undo/Redo/close-reopen checks. Two of
three attempt slots are consumed; one remains, with no quota or spending increase
implied. Keep the page still while thinking: this variant cannot detect same-page
ink edits after its capture. The historical shutdown crash remains unresolved.
See [beta boundaries](../tools/move-session/README.md). No production promotion or
full-size Paper Pro certification follows from this acceptance.

## Earlier observed state (2026-10-26), not a new live-test authorization

- Prior `remarkable-live-test-20261026` evidence records one authorized GPT-6 Luna
  request and one native insertion. A SHA-matched actual screenshot independently
  showed a visible 2×2 table below the handwritten request. No automatic retries.
  This establishes one narrow example, not general recognition or output quality.
- The owner retrospectively confirmed Undo, Redo and close/reopen worked at the
  time for the earlier live insertion. Record this as owner-observed acceptance,
  not automated replay or new device verification. The bounded live milestone
  is complete; no further owner repetition is required.
- Both live consumed markers were recorded present. Keep them and all prior
  offline markers; never reset a run to repeat a consumed request or insertion.
- Later `remarkable-full-capture-20261026` evidence records installed full-frame
  954×1696 capture, toolbar/close/widget hiding, exact restored native pose and
  pixel-identical before/after screenshots. No new API request or insertion.
- The earlier notebook-app shutdown SIGSEGV remains unexplained. The old app
  reproduced the failure before the full-capture pair was installed; a fresh-boot
  stop then succeeded. This is not evidence that the newer pair fixed the crash.
- Reconciliation and helper development performed no tablet access, actual-key
  access, API request, credential provisioning, restart, insertion or publication.
  Saved reports are evidence of their timestamps, not fresh runtime observations.

## Local-only configuration helper

The existing root-level `configure.sh` remains unchanged for the rM2 prototype.
Its full setup writes only key/model fields, so it is not used to preserve an
existing experimental configuration. The Move target now builds a separate
`build/move-live/configure-move-live`, sharing validation with the live client.

- No key through command arguments, environment, stdout or stderr; reads only a
  controlling terminal with echo disabled and restores the exact terminal state
  on success, error and handled INT/TERM/HUP/QUIT. No shell tracing or terminal
  recording should be active during actual key entry. SIGKILL cannot be handled.
- Requires an existing owned 0700 parent, without symlink ancestors; accepts only
  owned, singly-linked 0600 regular config files of at most 16KiB. Rejects malformed
  existing JSON, demo configuration and detected changes during key entry.
- Preserves unrelated JSON fields, sets only `api_key` and the fixed `gpt-6-luna`
  model, bounds the resulting file size, and uses atomic `QSaveFile` replacement
  without direct-write fallback. Failure before commit leaves the old file intact.
- `--check` validates the private config using the live client's loader and prints
  only a generic success/failure. It reads the key internally but does not print it
  or prove account access. Do not run it just to inventory file metadata.
- The helper links QtCore only. It has no HTTP, broker, marker-reset, arming,
  service-management or insertion code. Builds do not install or execute it.
- Key buffer wiping and disabled core dumps reduce incidental exposure; they do
  not promise elimination of all Qt/allocator copies from process memory.

After a separately authorized, inspected installation, use a private human-owned
terminal, not an agent-recorded PTY, for `configure-move-live --configure`.
The default path is `/home/root/.config/ink-diary/config.json`.
`--config ABSOLUTE_PATH` exists for isolated synthetic tests or an explicitly
reviewed alternate file; it does not change the live extension's configured path.
Do not provision another key if an already valid on-device configuration suffices.
There is no standing authorization to overwrite or rotate an existing credential.

Build/test entry points:

```sh
bash tools/move-live/test.sh
SDK_ENV=/path/to/exact-chiappa-sdk/environment-setup-cortexa55-remarkable-linux \
XOVI_REPO=/path/to/xovi bash tools/move-live/build.sh
```

Host checks use synthetic credentials and a controlling pseudo-terminal to test
echo, exact terminal restoration, interruptions, preservation, atomic boundaries,
oversize files/results, missing TTY, symlink/hardlink/FIFO and permission refusal.
They are not on-device proof. Build metadata includes helper and extension hashes.

## Historical candidate preparation — local only at the time (2026-10-26)

The completed full-frame v2 run is owner accepted: response completeness is not
an open defect, and Undo/Redo/reopen passed. The earlier shutdown root cause is
still unidentified. The previous lifecycle-first sequence below is superseded
by hc's explicit instruction to prepare per-request controls and diagnostics
before the next test; this is not a declaration that lifecycle is resolved.

[Move per-request candidate](../tools/move-session/README.md) preserves v2 and
adds an isolated, finite three-attempt build with persistent per-request guards,
visible terminal messages and content-free shutdown milestones. It retains the
exact capture/insertion checks and all production/offline gates. Three attempts
at 800 output tokens each is a usage quota, not a provider dollar billing cap.
No candidate deployment, restart, real-key access, paid API call or insertion
is authorized by local preparation alone. Next: review the local checks and
artifacts, then obtain the exact device-test/restart/spending scope.

## Historical next development sequence (superseded)


1. **Live run closed.** Owner-confirmed Undo, Redo and close/reopen complete the
   bounded milestone. Keep consumed markers and do not replay the request.
2. **Investigate lifecycle reliability locally.** Correlate saved previous-boot
   journal, shutdown stack/error evidence, extension load order and capture/client
   teardown ownership. Keep widget destruction's owner-directed visibility
   exception. Add a targeted regression only when a concrete failure mechanism
   is found; host teardown passing alone is not a hardware fix.
3. **Make any hardware experiment bounded and reviewable.** Name exact artifacts,
   rollback hashes, current notebook state, one stop/start attempt and the terminal
   condition. Inspect fresh runtime before any retry. Obtain restart authority for
   that experiment; the older deployment approval is not reusable standing access.
4. **Broaden behavior only after lifecycle closure.** A later ordinary text answer
   can test recognition/content/fit beyond the single table example. Use a new
   reviewed run directory and schema-2 visible/hidden reference, fresh disposable
   Blank page, unchanged old markers, explicit one-request/insertion authority and
   a spending ceiling. The earlier $0.50 single-run allowance is consumed, not a
   budget for a new test. One request/800 tokens is not a hard dollar billing cap.

No model fallback, API gate relaxation, production promotion, installer expansion,
Paper Pro hardware certification or broader template/zoom/orientation support is
implied. Current local helper artifacts remain undeployed.
