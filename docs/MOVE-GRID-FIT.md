# Move Small Grid false crop-fit failure

## Current checkpoint (2026-10-27)

Owner reported `answer-does-not-fit-visible-crop` on Small Grid while Blank worked.
Read-only inspection of saved request captures reproduced a template-detector bug.
Initial diagnosis and validation made no provider call or device change.
The owner then authorized installation. The focused fix is now **installed and
verified loaded** on the Move after one clean stop/start. The narrow QMD and
loaded UI resource are unchanged; calibration, key, policy, usage and all 61
other protected file hashes are unchanged. The allowance is unchanged and no
Start/model request was sent for deployment. Real provider/answer insertion has
not been re-tested automatically.
Private captures, measurements, artifact hashes and exact verification logs are
outside this checkout under workspace `reports/remarkable-grid-fit-20261027/`.

## Root cause

`gridBands()` treated every row/column with >=25% dark pixels as a possible rule.
One thick band or one off-pitch band invalidated the *entire* axis mask. Wide
handwritten table borders meet that coverage threshold. Horizontal template
recognition consequently failed, leaving native dithered grid rules near the
bottom of the capture counted as handwriting. Blank bypasses this grid filter.
In one affected private capture the old detector returned row1655; the corrected
result is row746, matching the writing extent and the Blank comparison.

## Fix and invariants

- Skip thick candidate bands without discarding unrelated thin factory rules.
- Fit the dominant **known factory-pitch** lattice, requiring >=75% expected
  bands and full-dimension span >=75%, with bounded quantization tolerance.
- Mask only observed thin bands that match the certified lattice. Never
  extrapolate a mask across thick/table/extra unmatched handwriting.
- Keep immutable source/API pixels, exact template/zoom admission, placement gap,
  answer size, crop-fit guard, native-pose/freshness checks and spending unchanged.
- New fixtures cover wide tables, extra off-lattice handwritten rules, thick
  strokes touching a grid row, sparse patterns and incorrect near-factory pitch.
- Added `grid-table-before-start` full mocked request/capture/render/insertion
  regression, including unchanged outbound question pixels.

## Candidate and scope

Validation source is an isolated checkout of the current committed project plus
only detector/regression changes; unrelated uncommitted Paper Pro/lifecycle work
in the shared checkout is preserved. Both dollar-budget capture-mode suites now
have 38 scenarios. The candidate device module is independently rebuilt from the
**installed source baseline** `153d17a606a5e46781c231014d349493eeb3ab29` with only
this detector change (and its tests), not the newer CLI or lifecycle work.
Candidate module SHA256:
`e81754c6168d333591059173e8623b79f22e1ba7a8c8114c6ba965ab05307f28`.
The authorized installation updated only SO and matching device-profile file
hash. The deployed narrow QMD, resource fingerprint, calibration, keys, policy
and usage remain unchanged. Rollback SO/profile are retained in a unique private
inactive stage; the exact path and runtime PID are in the private TaskFlow.
The previous local Vellum APK predates this fix: rebuild/revalidate it before
package migration or distribution rather than overwriting the corrected Move.
Any live deployment requires scoped authorization and a rollback-safe monitored
stop/start; inspect current state first and never retry an uncertain stop.

## Interpretation

The error can still be correct if an answer is genuinely too tall for the space
**below the lowest ink**. Total blank area elsewhere on the page is not usable
under the current non-overlap placement rule. The fix does not silently shrink
answers, insert over handwriting, remove a fit guard or refund prior charges.
