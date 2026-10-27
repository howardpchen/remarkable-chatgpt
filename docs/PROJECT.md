# reMarkable AI — Vellum Mod

## Objective

Make the existing native handwritten-question assistant installable through
Vellum, with a safe companion `remarkable-ai` CLI for local key and allowance
management. Keep the existing plugin architecture; use Vellum's APK/VELBUILD
conventions rather than inventing a package manager or companion service.

## Agreed design

- Package native XOVI module, QML widget, helper CLI, dependencies and exact
  device/firmware requirements. Start with a local experimental Move package.
- `remarkable-ai key status`: configured/missing/invalid metadata with masked
  identity, local allowance, conservative charged/reserved amounts, whether a
  next request fits, and last observed provider status with actionable guidance.
- `key set`: hidden interactive entry; add/replace without changing unrelated
  settings, calibration or spending. `key remove`: remove only the local key.
- No API request for status or setup. Do not represent the app's local allowance
  as an OpenAI account balance. Distinguish invalid credentials, provider billing
  quota, temporary rate limits and exhausted local allowance.
- Key replacement never refills the allowance. No automatic renewal, retries,
  paid verification or package-triggered request.
- Upgrades/removal preserve private state. Never bundle a tablet calibration or
  credential. Setup must use separately reviewed device-specific calibration;
  packaging is not a claim of general firmware/device compatibility.

## Existing implementation

`tools/move-session/`: persistent conservative $5 ledger and reusable-profile
controller. `tools/move-live/`: configuration helper and asynchronous OpenAI
client. `tools/move-demo/test-ui.cpp`: synthetic host UI/capture tests.
`tools/remarkable-ai/` and `packaging/vellum/`: CLI and package work.

## Source and distribution

Repository: https://github.com/howardpchen/remarkable-chatgpt
Working branch: `port/paper-pro-and-move`. Local commits may be ahead of remote;
check before publication. No new publication is authorized by project registration.
Vellum catalog acceptance is separate from building a local package.

## References

- https://github.com/vellum-dev/vellum (VELBUILD and virtual device/OS packages)
- https://github.com/vellum-dev/vellum-cli (supported CLI)
- https://github.com/vellum-dev/vellum/blob/main/packages/rm-shot/VELBUILD
