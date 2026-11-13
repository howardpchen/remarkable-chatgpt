# reMarkable AI / Vellum

## Scope

A native handwritten-question assistant with separate experimental Chiappa
(Paper Pro Move) and Ferrari (Paper Pro) runtimes and Vellum packages. Native rM2
remains separate. Exact supported firmware is OS 3.28.0.172 / Qt 6.10.3.

Both package candidates use template-independent native content bounds to place
text responses below existing content. This is not arbitrary-device support or
proof that every imported/image/vector item contributes complete bounds.

## Decisions

- Use Vellum's maintained vbuild/abuild pipeline; do not create a package manager.
- Read native item, root-text and aggregate scene rectangles; freeze geometry and
  viewport for each request. No template-pixel filtering or artwork allowlist in
  the per-request native-bounds path. Preserve independently reviewed device poses.
- Explicit Start/Cancel, text only, maximum 800 output tokens, conservative dollar
  reservations/ledger, prompt caching and no automatic retries.
- Key/status setup is offline. Key replacement does not refill allowance or prove
  provider billing/authorization. No package-triggered provider requests.
- Preserve private configuration, calibration, usage and prior artifacts on
  upgrade/removal. No package lifecycle hooks or automatic xochitl restart.
- Separate exact SDKs, binaries, model dependencies, private state and firmware
  resources. Never exchange Ferrari and Chiappa profiles or infer Ferrari turns.
- Only generic code/docs and synthetic tests belong in Git/packages. Device
  resources, credentials, notebook data, reports and signing private keys stay out.

## Current references

- [Checkpoint and evidence](PROJECT-STATUS.md)
- [Remaining work](ROADMAP.md)
- [Native placement design and limitations](NATIVE-BOUNDS-PLACEMENT.md)
- [Per-device parity matrix](VELLUM-PARITY.md)
- [Packaging procedure](../packaging/vellum/README.md)

Repository: https://github.com/howardpchen/remarkable-chatgpt
Branch: `port/paper-pro-and-move`. Source publication is not Vellum catalog
acceptance or authorization to deploy to a tablet or call a paid provider.
