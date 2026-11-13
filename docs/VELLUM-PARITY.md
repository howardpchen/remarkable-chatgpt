# Ferrari / Chiappa native-bounds parity

## Beta2 feature matrix

| Capability | Chiappa / Move | Ferrari / Paper Pro |
|---|---|---|
| Product/firmware | Chiappa3.28.0.172 / Qt 6.10.3 | Ferrari3.28.0.172 / Qt 6.10.3 |
| Physical framebuffer |954×1696 |1620×2160 |
| Landscape reading view |1696×954 |2160×1620 |
| Package | remarkable-ai-move | remarkable-ai-ferrari |
| Native item/root-text/aggregate envelope | yes | yes |
| Template-only per-request admission | removed | removed |
| Native-envelope projection / 45px gap /fit | yes | yes |
| Frozen bounds/page/object/viewport checks | yes | yes |
| Bounded50–400% zoom/pan | calibrated orientation | calibrated orientation |
| Landscape anchor | reviewed half-turn derivation | independent CW/CCW; never infer opposite |
| Request capture | single (known edit-detection limitation) | dual, retaining pixel comparison |
| Start/Cancel,800 tokens,no retries | yes | yes |
| Persistent conservative dollar ledger | yes | yes |
| Existing reservation per request |$0.25421 |$0.10 |
| Offline key/status CLI | remarkable-ai | remarkable-ai-ferrari |
| Package hooks/funding/private data | none | none |

Feature parity means the same template-independent placement semantics and
package guarantees, not identical hardware, calibration, capture strategy or
universal content coverage. Ferrari retains its independent native bridge and
private state. Existing Ferrari explicit template-pose records are honored;
no native-pose guard or orientation calibration is removed.

## Validation boundary

The owner confirmed Chiappa ruled-template placement without overlap and native
Undo/Redo/reopen in an earlier focused module. The cleaned beta2 module/package
is a fresh build; that prior sample is not validation of every package/view.
Ferrari native-bounds code remains local-only; prior pixel-detector hardware
checks do not validate this method. Native rM2 remains unchanged.

## Completed local release checks

- Exact isolated publication-tree builds passed for both device AArch64 SDKs.
- Chiappa: 78 request scenarios in each capture mode, actual QML/widget, native
  projection, bounds/profile/budget checks passed. Ferrari: 96 request scenarios,
  native geometry/profile/pose and budget-policy checks passed.
- Historical offline runtime/widget and core capture/mapping checks passed.
- Both signed beta2 r0/r1 pairs passed payload audits and real APK solver
  install/upgrade/removal with synthetic private state byte-identical and wrong
  model/firmware refused. Six generic files per payload; no hooks or private data.
- CLI tests use synthetic keys/temporary state only; no provider or tablet calls.

Live Vellum migration/expanded-view placement and
Ferrari native-bounds response tests require separate scoped authority.
See [checkpoint](PROJECT-STATUS.md), [design](NATIVE-BOUNDS-PLACEMENT.md) and
[packaging](../packaging/vellum/README.md).
