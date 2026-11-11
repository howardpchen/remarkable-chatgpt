# Experimental Ferrari package setup

Only reMarkable Paper Pro **Ferrari**, OS 3.28.0.172 / Qt 6.10.3.
Not Chiappa (Paper Pro Move), rM2 or arbitrary future firmware. This package
has local synthetic lifecycle validation only, not live Vellum validation.

## Identity and prerequisites

Package: `remarkable-ai-ferrari`; offline CLI: `remarkable-ai-ferrari`.
Module: `/home/root/xovi/extensions.d/ink-diary-paper-pro-live.so`.
QMD: `/home/root/xovi/exthome/qt-resource-rebuilder/paper-pro-live.qmd`.
Private configuration: `/home/root/.config/remarkable-ai-ferrari/config.json`.
Private state: `/home/root/xovi/paper-pro-session-portrait-v1`.

This is not turnkey provisioning. Independently review the complete device profile,
module/QMD/resource fingerprints and required dependency file manifest at the
final installed paths. Existing manual runtime paths are not package paths.
Calibrate each template/direction independently; never import a Move profile.
The app requires `Caveat.ttf` and `DejaVuSansMono.ttf` in its private state
folder, provisioned separately from the source repository's licensed `fonts/`
assets. The installer never owns or replaces anything in private state.
Missing fonts/profile/policy must fail closed; installing or setting a key does
not fund the app. Current runtime supports portrait Blank/Dots small/Grid small,
and separately calibrated Blank/Dots small/Grid small CW/CCW landscape, with
bounded zoom/pan. Local test coverage is not real placement validation.

## Migration, upgrade and removal

Owner approval and library readiness are required for device work. Preserve a
paired SO/QMD/profile rollback and all current private state. Use the reviewed
single idle-gated stop; inspect live state if shutdown fails. Do not automatically
retry. Historical shutdown instability is unresolved.

Do not force-overwrite manual files. Retain only app-owned conflicting artifacts
in inactive rollback storage, preserve unrelated XOVI mods, install through the
supported trusted Vellum workflow and review fingerprints for the new paths.
Do not load the old isolated Ferrari runtime together with the packaged module;
use the supported XOVI launcher only after the complete pair/profile is ready.
Explicit public signing-key trust is separate authority. Never copy the private
signing key onto a tablet or bypass APK signature verification.

No package hook restarts xochitl, calls an API, sets/removes a key, provisions a
profile, funds a budget or changes usage. Upgrade/removal preserves all app-owned
private state. Uninstall `remarkable-ai-ferrari` removes package files only.
Restoring rollback must never restore an older spending ledger.

## Offline key CLI

```
remarkable-ai-ferrari key status --json
remarkable-ai-ferrari key set
remarkable-ai-ferrari key remove
```

Hidden terminal entry only; no secret command-line argument. Status is local
metadata, not provider verification/account credit. Key changes do not reset
usage or replenish allowance. Preserve conservative reservations, 800 output
tokens, explicit Start/Cancel and no automatic retries. Real API calls and live
installation require separate scoped authority.
