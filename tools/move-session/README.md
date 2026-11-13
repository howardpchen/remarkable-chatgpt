# Chiappa per-request assistant

Experimental exact Chiappa OS 3.28.0.172 / Qt 6.10.3 runtime. Native-bounds placement
is independent of template artwork; device/viewport/native-pose calibration is not
optional. rM2 and the separate Ferrari runtime are not interchangeable.

Build with the exact Chiappa SDK:

```sh
SDK_ENV=/path/to/chiappa/environment-setup-cortexa55-remarkable-linux \
XOVI_REPO=/path/to/xovi SINGLE_CAPTURE=1 DOLLAR_BUDGET=1 \
bash tools/move-session/build.sh
```

With a matching host Qt environment, run `SINGLE_CAPTURE=1 DOLLAR_BUDGET=1 bash
 tools/move-session/test.sh` (omit SINGLE_CAPTURE for dual-capture regression).
Tests use mock HTTP, synthetic geometry/keys and temporary private state only.

The packaged variant uses a single capture and persistent conservative dollar
ledger, maximum 800 output tokens, explicit Start/Cancel and no retries. Key
changes never refill allowance. Do not write while a request is pending:
equal-envelope edits can evade snapshot comparison. Upgrade/removal preserve
private state; a package install does not activate code, configure a key or fund
requests. Retired signal-watcher commands are not part of the package.

See [design](../../docs/NATIVE-BOUNDS-PLACEMENT.md),
[checkpoint](../../docs/PROJECT-STATUS.md) and
[package setup](../../packaging/vellum/SETUP.md).
