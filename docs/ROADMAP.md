# Roadmap

## Active: full-size Paper Pro portrait

1. Read-only Ferrari inventory and exact SDK setup — completed locally/observed.
2. Isolated portrait geometry pipeline, synthetic rejection tests and separate
   exact-SDK diagnostic target — completed locally, not device-validated.
3. Separately authorized temporary diagnostic loading; inspect actual Qt window,
   resource/ABI and notebook mapping before constructing calibration.
4. Ferrari-specific native-pose/capture/owner-boundary integration and offline
   insertion/Undo/Redo/reopen validation.
5. Reuse existing guarded request/budget flow; separately authorize provider
   testing and any package deployment/publication. Landscape is out of scope.

See [Paper Pro portrait checkpoint](PAPER-PRO-PORTRAIT.md).

## Current milestone: local experimental Vellum package

1. Implement key status/set/remove and budget reporting without network calls.
2. Persist only sanitized provider outcome metadata, tied to the credential
   fingerprint; replacing a key must not label an old error as its verification.
3. Test hidden entry, state preservation, unsafe paths, malformed ledgers,
   exhausted budget, authentication/quota/rate-limit guidance and stale outcomes.
4. Build AArch64 CLI and module with the exact Move SDK.
5. Package with VELBUILD/APK tooling; test install, upgrade and removal in an
   isolated root with synthetic state. No automatic service restart or funding.
6. Document first-time setup/calibration and migration from a manual installation.
7. Review local artifacts and limitations. Tablet package installation and any
   public distribution are separate next actions, requiring scoped authorization.

## Later

- Resolve historical shutdown instability before broader support claims.
- Add supported calibration UX, more firmware/device builds and compatibility CI.
- Validate package installation/removal on the real Move with rollback.
- Consider catalog submission (check current upstream contribution rules).
- Improve UI and answer capabilities without weakening capture or spending guards.
