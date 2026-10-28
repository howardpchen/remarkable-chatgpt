# Move accounting and idle collapse

## Scope / local candidate (2026-10-27)

Owner requested the accounting correction and automatic widget collapse after
10 seconds idle, later shortened to 3 seconds at the owner's request. The focused tested build was subsequently installed with scoped
owner approval, and seven freshly verified unsent holds reconciled offline.
See PROJECT-STATUS.md for live evidence and limits. This was not provider
verification, a ledger reset or Vellum migration. Preserve the existing $5 policy
and all attempt history.

## Accounting

- New reservations create a durable `BUDGET_LOCAL` marker. The private-root
  exclusive lock serializes reservations, transitions and settlement.
- A `dispatchGuard` persists `BUDGET_DISPATCH` and syncs it **before** POST.
  Failure refuses dispatch. Any dispatch marker, including an incomplete one,
  prevents a local release: ambiguous transmitted requests retain their hold.
- Terminal local failures, cancellation and orderly destruction write
  `BUDGET_RELEASE` only for a provably undispatched new reservation. No attempt
  directory or START marker is removed. A release prevents later dispatch or
  settlement. Malformed release state fails the budget scan closed.
- Usage settlement occurs before answer/page-validity rejection when a complete
  successful response is available. Explicit abort/timeout with unknown usage
  still retains the conservative hold. No retries or extra requests are added.
- New settlements save allowlisted model/tier, integral token counts, optional
  bounded cached-input count and conservative pricing provenance in
  `BUDGET_USAGE.json`. No notebook/answer text, key or raw response is saved.
- The existing padded reservation and settlement rates remain the spending
  **guard**, not invoice pricing. The simplified widget shows only available
  local allowance as `AI: X.XX`; detailed settled guard charges and held
  reservations remain available through the CLI.
  CLI adds settled guard and released-attempt counts while retaining its old
  combined field for compatibility. Historical invoices cannot be reconstructed
  from the old derived-cost-only records; no actual billing estimate is invented.

## Legacy reconciliation

`remarkable-ai budget reconcile-unsent --state-dir PATH --request-id ID`
previews one explicit ID (repeat the option for several). It writes nothing.
Applying additionally requires `--apply --runtime-stopped`. This is an operator
assertion, **not automated proof** that the runtime is stopped: do not apply
while any old runtime can still claim START/dispatch. Review source/provenance,
back up and stop the runtime first. Do not infer absence of provider billing from
marker absence after storage corruption or out-of-band edits.

Legacy release requires no START_CONSUMED, dispatch, cost or existing release;
new local releases require the exact new local marker and no dispatch/cost.
All paths are canonical numeric IDs under an owned private root, opened without
following symlinks. Release keeps the policy, old cost records, capture files,
calibration and key unchanged; the release marker records its reason. Repeating
an applied release is refused rather than adding another credit. No automatic
startup migration of old reservations is performed.

The private audit identifies seven legacy no-START holds. Their release must be
revalidated against the then-current stopped device, not applied to a stale copy
or inferred from the owner's approximate OpenAI total. A lower available balance
than the provider dashboard does not authorize filling the ledger to that total.

## Widget idle behavior

The expanded idle panel collapses to its existing AI launcher after 3 seconds.
Opening, finishing a request and releasing input start a fresh interval; held
input pauses it. Status polling is not user activity. Busy requests stay expanded
with Cancel reachable, and capture suppression takes precedence. It never
starts/cancels a request, dismisses the widget entirely or changes native toolbar
restoration. The exact expanded capture-owner dimensions and admission guards
are unchanged. All controls and card-background input reset the timer.

## Validation / deployment boundary

Use both `DOLLAR_BUDGET=1 tools/move-session/test.sh` capture modes, CLI tests,
synthetic transport tests, actual QML timer/input/busy/layout/toolbar tests and
the exact Chiappa SDK. Private focused source/artifacts/logs are under workspace
`reports/remarkable-accounting-idle-20261027/`. The focused snapshot starts from
the previously installed toolbar candidate and excludes unrelated lifecycle,
Ferrari and runtime provider-observer integration work. Exact-release QMD must
apply locally; that is not proof of live resources or expanded-panel appearance.

Deploy only under scoped owner authority: recheck live firmware/state, preserve
rollback/baseline hashes, use the previously reviewed idle-gated stop, reconcile
only freshly verified explicit IDs while stopped, install SO/QMD and update only
the profile's artifact fingerprints. Verify effective module/resource, unchanged
calibration/key/policy/history, released-hold totals and stable process/root mode.
Historical shutdown crashes remain unresolved; no paid call is needed to install.
