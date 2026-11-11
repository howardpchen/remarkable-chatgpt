# Ferrari / Chiappa release preparation — 2026-11-11

## Commit scope

This source milestone adds a separate Ferrari runtime, independent notebook
mapping review and synthetic layout tests, bounded zoom/pan and per-template
landscape admission, zoom-aware certified dot filtering, reviewed Move Blank
half-turn support, and device-specific Vellum packaging. Native rM2 is unchanged.

Packages are `remarkable-ai-ferrari` (rmpp) and `remarkable-ai-move` (rmppm),
version 0.2.0_beta1-r0; r1 exists only to test upgrades of the same payload.
Both pin OS 3.28.0.172 and require their own exact Qt 6.10.3 AArch64 SDK.
See [packaging](../packaging/vellum/README.md), [Ferrari setup](../packaging/vellum/SETUP-FERRARI.md)
and [parity matrix](VELLUM-PARITY.md).

## Verification record

The preceding local packaging run recorded successful exact-SDK builds for both
devices, host runtime/profile/ink/native-pose/ledger and production UI suites,
offline CLI checks, and signed APK install → upgrade → removal in disposable
network-disabled containers. Wrong firmware/model rejection and byte-identical
synthetic private-state preservation passed. Four payload audits (r0/r1 for both
devices) verified six generic files and no lifecycle hooks or private payload.
These are prior recorded checks, not a claim that every build was repeated during
commit preparation. Commit preparation independently rechecked retained artifact
hashes and both r0 payload audits against the final prepared inputs. Logs and manifests remain in ignored build storage.

Reproducible local commands (configure host Qt pkg-config/library environment):

```sh
bash tools/paper-pro-portrait/session/test.sh
bash tools/paper-pro-portrait/test.sh
bash tools/move-session/test.sh
bash tools/remarkable-ai/test.sh
bash tools/move-live/test.sh
python3 packaging/vellum/audit-package.py --device ferrari --input PREPARED APK
bash packaging/vellum/run-lifecycle.sh ferrari FRESH_DISPOSABLE_DIRECTORY
# Repeat audit/lifecycle for chiappa; exact SDK builds are documented in packaging.
```

## Fresh commit-preparation checks

Rebuilt and passed Ferrari session tests (including mock runtime, profile, ink,
native-pose and policy checks), Ferrari mapping/geometry and all four layout UI
invocations, the default Move session suite and production UI, and offline CLI
status/reconciliation/configuration/provider-metadata tests. Shell syntax,
Python compilation, staged whitespace and sensitive-pattern checks passed.
Both r0 APK payload audits and retained artifact hashes passed. No exact-SDK
rebuild or container lifecycle rerun was performed in this commit-preparation
pass; those results above come from the retained packaging run. Host compilers
still report existing unchecked-write return-value warnings in key-entry code.

## Limits and next gates

Workflow parity is not view parity: Move retains Blank-only experimental
landscape; Ferrari requires independently reviewed CW/CCW template profiles.
Do not copy calibration between devices. Grid rules thicker than four pixels can
remain false ink; dot certification cannot distinguish handwriting exactly on a
certified factory dot. Source/API pixels remain immutable.

No real-tablet Vellum install/upgrade/remove, dependency ABI, new loaded-resource
or post-fix zoomed-Dots placement validation is established by these local tests.
Real placement, Undo/Redo and reopen need scoped hardware validation. Historical
shutdown instability remains unresolved. Package installation must not restart
xochitl automatically or fund a budget. Preserve keys, policy, profiles and usage.

This commit preparation makes no tablet changes, API calls or publication.
Private device history, reports, resources, profiles, captures, configuration,
build artifacts and signing keys are excluded. Deployment, trust changes and
publication require separate scoped authority.
