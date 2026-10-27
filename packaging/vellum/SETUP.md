# Experimental Move package setup

This is not general reMarkable support. Only Chiappa / OS 3.28.0.172 / Qt 6.10.3,
portrait 954×1696, normal zoom and Blank/Dots small/Grid small are validated.
The known historical shutdown issue is not claimed resolved by packaging.

## Install and manual migration

A locally signed APK is built using Vellum's maintained vbuild/abuild tools.
Install through Vellum with the normal trusted local-package workflow. Review and
trust the local development public signing key explicitly; this package must not
change the tablet trust store itself. Never use force-overwrite to take ownership
of untracked manual files. Before migrating the manually installed beta, stop
xochitl cleanly, retain the existing SO/QMD/profile as a rollback set, and move
only the two untracked app-owned SO/QMD files into an inactive private staging
directory. Preserve every other mod and all app state. Do not restart into a
partially migrated pair. Install the package before the supported XOVI start.
Device mutation and signing-key trust changes require owner authorization.

## Key CLI

```
remarkable-ai key status
remarkable-ai key status --json
remarkable-ai key set
remarkable-ai key remove
```

Set uses a hidden terminal prompt (use a private SSH terminal). No key is accepted
as a positional argument. Status is offline: it shows a non-secret SHA-256 key
identifier, local conservative allowance, unsettled charges and last observed
provider outcome, not the account balance or fresh verification. Provider history
is recorded only by this new module; older runs have no such history. Quota,
rate-limiting and authentication guidance are distinct. Changing/removing a key
does not revoke it at OpenAI and never resets spending records.

Private config: `/home/root/.config/ink-diary/config.json` (0600 in 0700 directory).
Private runtime: `/home/root/xovi/move-session-small-templates-v4` (0700). No config,
profile, policy, captures or notebook material are bundled or owned by the package.

## Device setup and allowance

The module fails closed without a separately reviewed schema-3 device profile and
valid budget policy. This first package retains the existing reviewed calibration
workflow documented in `tools/move-session/README.md`; it does NOT automatically
calibrate an arbitrary tablet. Review the exact firmware, viewport, shown/hidden
native poses, current effective QML resource hash and file hashes on that tablet.
Never copy another user's profile, reset numbered request directories, or grant a
budget from an installer. The profile and one-time policy must be explicitly
provisioned by the owner using the reviewed deployment workflow. Existing owner
calibration and budget records must be retained. See `docs/PROJECT.md` in the source
repository for support scope and `tools/move-session/README.md` for policy schema.

## Upgrade and remove

No lifecycle hook restarts xochitl or touches private data. Before an upgrade or
removal, the operator must cleanly stop xochitl using the reviewed procedure.
Upgrade replaces only package-owned binaries/docs/QMD. The old profile's hashes
will no longer match a changed build: review the new installed file/resource
hashes and update only the profile fingerprint fields, retaining all calibration,
policy, usage records and historical requests. Then use `/home/root/xovi/start`.
No automatic hash re-approval or budget refill is performed. On failure restore
the paired rollback artifacts and matching profile; never restore/reset old usage.

`vellum del remarkable-ai-move` removes package-owned files only. Credentials,
provider metadata, policy, calibration and spending records remain. Key removal
is a separate explicit CLI action before uninstall, if wanted. The app's local
$5 guard is not an OpenAI account-wide or provider-enforced billing cap.
