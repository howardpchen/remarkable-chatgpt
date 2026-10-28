# Local broker lifecycle overlay (experimental)

This is an isolated build overlay for rm-xovi-extensions commit
`6c709dd8416ddf89034831073d76fb4ee3372905`, broker ABI 0.2.0. It is not a Vellum
package update or a change to an installed upstream checkout. Upstream license
and provenance are retained in the generated build copy. Only the explicitly
reviewed Ferrari 3.28.0.172 SDK build is admitted here.

The existing FIFO names, protocol, GUI-process conditions, native handler
metadata and ABI are unchanged. A joinable worker uses nonblocking descriptors
and a wake pipe. App `aboutToQuit` closes command intake and wakes polling; app
context destruction or final ELF teardown joins the worker and removes only the
FIFO inodes it created. It refuses pre-existing endpoints rather than replacing
another active broker. Replies handle partial writes with a three-second deadline;
missing/disconnected readers cannot indefinitely block shutdown. SIGPIPE is
blocked only on the FIFO worker, not process-wide. Input commands are bounded
and oversized/embedded-NUL commands reject. Signal values/replies are not logged.

QML listener dispatch uses the shared owned-job GUI dispatcher instead of a
blocking queued call. Native handlers retain their ABI and must themselves use
bounded/cancellable dispatch: the diagnostic and Move endpoints now do. A
third-party native callback that never returns cannot safely be preempted by
this broker; no claim of arbitrary callback cancellation or live `dlclose`
safety is made. `aboutToQuit` does not join on the GUI thread while other quit
callbacks still need to cancel their queued work.

## Build and tests

```sh
bash tools/broker-lifecycle/test.sh
SDK_PRODUCT=ferrari SDK_RELEASE=3.28.0.172 \
SDK_ENV=/path/to/exact/ferrari/environment-setup-cortexa53-crypto-remarkable-linux \
BROKER_REPO=/path/to/pinned/rm-xovi-extensions \
XOVI_REPO=/path/to/xovi \
BROKER_BUILD_DIR=/fresh/local/build-directory \
bash tools/broker-lifecycle/build.sh
```

A fresh build directory is required; an unexpected upstream commit or tracked
source modification rejects. The build records source/binary hashes and checks
the generated dependency table and ELF target. Builds/tests never deploy.
Host worker tests exercise the actual replacement implementation with synthetic
paths/handlers: partial large replies, oversized input, absent reader at shutdown,
join/cleanup and refusing pre-existing endpoints. Shared GUI tests additionally
exercise queued/direct work, canceled pending work, an already-running timeout,
and real `aboutToQuit` intake closure. Target loading and tablet shutdown remain
separate, explicitly authorized checks.
