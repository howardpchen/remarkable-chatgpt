# Paper Pro portrait — isolated local geometry candidate

This is **not a working Paper Pro port**, trusted calibration, runtime admission,
native insertion, firmware compatibility claim or installable package. It does
not load a module, patch QML, access a tablet, send requests, or touch private
configuration/usage. Existing rM2/Move source and installation gates are unchanged.

The 1620×2160 physical capture size is a candidate constraint, not proof of Qt
logical dimensions, DPR, scene coordinates or firmware compatibility. Every caller
must explicitly supply a physical-to-logical coordinate transform **separately**
from the observed logical viewport, DPR and logical-to-scene transform. The
synthetic fixture intentionally uses 810×1080 logical dimensions, DPR 1.25 and
0.5 physical-to-logical scale; none is asserted to describe a real tablet.

`geometry.h` freezes the supplied Blank/page/viewport reference, validates finite
positive uniform axis-aligned mappings, requires the physical-to-logical mapping
to match the entire logical viewport and bounds an explicitly supplied crop.
It reuses `src/layout.h` and `src/markdown.h` for physical screenshot → crop → ink
extent → Markdown render → scaled native-raster **candidate** and corresponding
physical/logical/scene rectangles. Physical fit is checked before raster rounding.
The source image is unchanged. This is Blank-only mathematical preparation;
no claim is made about color ink, factory templates or real native insertion.

No capture-owner logic is implemented. Any later integration must preserve the
exact 360×176 capture-owner boundary and independently validate its measured
position, native visible/hidden poses, suppression/restoration, page freshness and
capture admission at every boundary. A supplied reference here is test data,
never device evidence or authority. A logical/scene transform change rejects;
normal-zoom identity itself must later come from separately reviewed calibration.

Run host tests (rebuilds from current source):

```sh
source /tmp/remarkable-session-host-env.sh # this host's optional Qt environment
bash tools/paper-pro-portrait/test.sh
```

Tests cover the full synthetic pipeline, explicit non-DPR coordinate scaling,
outside-crop ink exclusion, frozen-reference changes, invalid transforms and
geometry, wrong/rotated capture dimensions, empty/full pages, answer overflow,
native raster budgets, and unchanged input images. Host Qt success does not
establish exact Ferrari SDK compatibility or device behavior.

Next gates: read-only exact product/firmware/Qt inventory; matching Ferrari SDK;
reviewed resource/ABI inspection; measured physical/logical/scene calibration;
separate offline widget/capture/insertion tests with explicit device authority.
Reuse existing dollar-ledger reservations, prompt caching, 800-output-token cap,
explicit Start/Cancel and no retries in a later port; setup must not fund usage.
