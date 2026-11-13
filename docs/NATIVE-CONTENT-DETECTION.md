# Native content inspection

Both device runtimes expose read-only `content-bounds` and `content-detection`
commands through their existing device-specific broker entrypoint. Run only on
an authorized current session; never export notebook content or page IDs.

`content-bounds` reads native item, root-text and aggregate rectangles.
`content-detection` additionally returns their conservative scene envelope.
Missing/wrong-type/nonfinite/negative properties refuse full detection. Zero-height
or zero-width lines are retained; neither endpoint asserts verified emptiness,
complete content coverage or a monotonic revision.

The same product-neutral reader supports request placement, but diagnostics alone
never dispatch a capture, reserve funds, edit a notebook or authorize deployment.
The signal watcher and page-schema exploration are retired from the package.

See [placement design](NATIVE-BOUNDS-PLACEMENT.md), [evidence](PROJECT-STATUS.md)
and [remaining work](ROADMAP.md).
