# Ferrari / Chiappa packaging checkpoint

Prepared locally on 2026-11-11. No tablet operation or provider call.

| Capability | Ferrari / full-size Paper Pro | Chiappa / Paper Pro Move |
|---|---|---|
| Package | remarkable-ai-ferrari | remarkable-ai-move |
| Model dependency | rmpp | rmppm |
| Exact SDK | Ferrari3.28.0.172 Qt6.10.3 | Chiappa3.28.0.172 Qt6.10.3 |
| Offline key/status CLI | remarkable-ai-ferrari | remarkable-ai |
| Start/Cancel,800 tokens,no retries | yes | yes |
| Preserved conservative dollar ledger | yes | yes |
| Portrait Blank/Dots small/Grid small | yes, separate profiles | yes, separate profiles |
| Bounded zoom/pan | yes | yes |
| Landscape templates | Blank/Grid/Dots, independent CW/CCW profiles | experimental calibrated Blank only |
| Zoom-aware bounded dot filtering | current shared detector | current shared detector |
| Signed local APK / upgrade / removal | synthetic verified | synthetic verified |
| Real tablet Vellum install/upgrade/remove | not tested | not tested |

Parity here means equivalent packaging and guarded workflow, not identical
hardware coverage. Do not enable Move patterned landscape or another turn by
copying Ferrari records; new native geometry, exact resources, per-direction
calibration and regression tests are required. Real zoomed-dot placement after
the last Ferrari fix remains owner-unverified. Grid rules thicker than4 pixels
remain a conservative false-ink risk on both; no masking limit was widened.
Move single-capture mode retains its known same-page-new-ink limitation.

## Local checks

Fresh exact-SDK builds, host runtime/profile/template-ink/native-pose/budget
suites, offline CLI tests and production UI suites passed. Signed APK solver
checks reject wrong firmware/model and preserve synthetic private state through
install/upgrade/remove. Payload audit checks six generic files and no hooks,
links, unexpected private paths or resource dumps. r1 is solely an upgrade-test
release of the r0 payload. No full hardware or external dependency-ABI guarantee.

## Remaining before real use

Follow the device-specific setup document. Both packages intentionally need
separately provisioned licensed fonts, reviewed profile/file/resource fingerprints
and owner-funded policy. No credentials, calibration or data are bundled.
Migration from manual isolated runtime must review the new installed paths and
avoid double-loading mods. Historical shutdown instability remains unresolved.
Next live phase needs explicit scoped approval, library readiness, rollback,
trusted signing setup, clean migration, loaded-resource and dependency checks,
then real placement/Undo/Redo/reopen. Publication is separately authorized.
