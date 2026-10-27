# reMarkable AI / Vellum project

Start with `docs/PROJECT.md` (scope and decisions), `docs/PROJECT-STATUS.md`
(current checkpoint), and `docs/ROADMAP.md` (work remaining). Update the status
checkpoint after meaningful work so a fresh session can resume without chat history.

## Boundaries

- This checkout is the source of truth. Keep native rM2 support separate from the
  experimental Move implementation in `tools/move-session/`.
- Current Move support is narrow: Chiappa, portrait 954×1696, normal zoom,
  OS 3.28.0.172 / Qt 6.10.3, Blank/Dots small/Grid small, text answers only.
- Paper Pro portrait work is separate local preparation in
  `tools/paper-pro-portrait/` and the explicit Ferrari diagnostic build. Its
  physical/logical mapping, native resources and runtime remain unvalidated;
  never substitute Move profiles, SDKs, packaging or private state.
- Preserve the exact 360×176 capture-owner boundary even when the visible widget
  is narrower. Do not weaken native-pose, page-change or capture admission guards.
- Preserve the dollar ledger, conservative reservations, prompt caching, 800
  output-token limit, explicit Start/Cancel and no automatic retries.
- A credential change never resets usage, refills a budget, or grants paid calls.
- Never commit keys/configuration, notebook data, captures, private device profiles,
  private reports, package signing keys, or raw provider error bodies. Use synthetic
  fixtures. Only generic code/docs may be published; inspect outgoing changes.
- Local implementation/testing is authorized. Device deployment, publication,
  key replacement/removal on a live tablet, and real API calls require scoped
  authority. Inspect live state before retrying an uncertain operation.
- Vellum installs must not auto-restart xochitl or silently fund a budget. Preserve
  app-owned private state across upgrade/removal. Refuse unvalidated firmware.

## Verification

Use the installed exact Move SDK for AArch64 binaries; host Qt tests alone do not
prove target compatibility. Rebuild tests from current source. Use synthetic
credentials, temporary private state directories and mock HTTP responses.
Distinguish local rendering, package lifecycle tests, loaded device resources,
provider verification and live expanded-panel rendering; none implies the others.

Current source and host setup pointers are documented in `docs/PROJECT-STATUS.md`.
Project-specific procedures belong here or in docs, not in generic learned skills.
