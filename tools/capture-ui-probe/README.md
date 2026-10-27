# Capture UI lease diagnostic (temporary, Move only)

No screenshots or notebook writes are performed by this extension. It exposes
`ink-diary-capture-ui$control` with exact commands `begin`, `status`, `release`.
Nothing happens on load. Only the owned compatible Move UI specimen is suppressed;
native/Vellum toolbar state is untouched. `ready` means a synchronized Qt frame,
NOT framebuffer-spy or physical display freshness. All begin outcomes must end in
release; a 15-second GUI timer automatically restores after operator disconnect.
Missing/old/incompatible panels fail closed. No API/config/credentials/insertion.

Build with exact Chiappa 3.28.0.172 Qt6.10.3 SDK using build.sh. Outputs are only
under build/capture-ui-probe; do not promote bin/release or production support.
The generated link table must have unique typed entries. Helper process load
rejection uses the existing diagnostic policy plus broker's GUI predicate.

Host verification: run tools/move-ui-probe/test.sh in a working host Qt environment.
Compile test.cpp with Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Test and -ldl, then run with
QT_QPA_PLATFORM=offscreen QSG_RHI_BACKEND=software. Its expiry test takes 15 seconds.

Device procedure: after reviewed isolated load and fresh disposable-page owner
confirmation, exclusively query template/viewport, save a baseline screenshot;
begin and poll bounded status until ready, call existing rm-shot into a unique
private path, decode image and recheck ready, release. Capture restored control,
compare panel region and outside region, viewport and native toolbar state. Repeat
for the second capture window only within that confirmed page's scope. Reject
page/zoom changes, lease expiry or UI contamination. Do not infer success from
render acknowledgement. Preserve old consumed insertion guards. Runtime Move
capture readiness must remain closed until framebuffer evidence is verified.

Recovery: restore old temporary UI QMD and quarantine only this diagnostic outside
extensions.d, using supported clean-stop/restart flow. Never delete third-party
mods or overwrite native toolbar bindings. A prior shutdown crash remains
unexplained; one clean stop does not resolve it.
