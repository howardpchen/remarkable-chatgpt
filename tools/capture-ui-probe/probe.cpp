// Temporary own-UI lease only: no screenshot, API, credentials or insertion code.
#include "../../src/capture-ui.h"
#include "../diagnostic/policy.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstring>
#include <dlfcn.h>
namespace {
QPointer<DiaryCaptureUiSession> lease; // GUI-thread-only; no static QObject owner
QString phase = QStringLiteral("idle"), failure;
unsigned generation = 0;
QByteArray read(const char *path) { QFile f(QString::fromLatin1(path)); return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray(); }
bool allowed() {
    return diagnostic::permitted(read("/sys/devices/soc0/machine"),read("/etc/os-release"),qVersion(),
        dlsym(RTLD_DEFAULT,"_Z21qRegisterResourceDataiPKhS0_S0_") != nullptr,
        dlsym(RTLD_DEFAULT,"_ZNK3csl10devicelock10DeviceLock3setENS1_5StateE") != nullptr);
}
void release() { ++generation; if (lease) delete lease.data(); lease.clear(); }
QJsonObject request(const QByteArray &command) {
    if (command=="release") { release(); phase="released"; }
    else if (command=="begin") {
        if (lease) return {{"ok",false},{"error","lease-already-held"}};
        QQuickWindow *window=nullptr;
        for (auto *w:QGuiApplication::allWindows()) {
            auto *quick=qobject_cast<QQuickWindow *>(w);
            if (!quick || !quick->isVisible()) continue;
            QList<QQuickItem *> pending{quick->contentItem()};
            bool found=false;
            while (!pending.isEmpty()) {
                auto *item=pending.takeLast();
                if (item->objectName()=="moveUiProbe" && item->isVisible()) found=true;
                pending.append(item->childItems());
            }
            if (found) { if (window) return {{"ok",false},{"error","ambiguous-window"}}; window=quick; }
        }
        if (!window) return {{"ok",false},{"error","visible-probe-required"}};
        phase="waiting"; failure.clear();
        lease=new DiaryCaptureUiSession(window,[](auto event,const QString &why) {
            phase=event==DiaryCaptureUiSession::Event::RenderReady ? "ready" : "failed"; failure=why;
        },qGuiApp);
        const auto id=++generation;
        // Recovery independent of SSH connection, including client interruption.
        QTimer::singleShot(15000,qGuiApp,[id] {
            if (generation==id) { release(); phase="expired"; }
        });
        lease->begin();
    } else if(command!="status") return {{"ok",false},{"error","unsupported-command"}};
    return {{"ok",true},{"phase",phase},{"ready",lease && lease->readyForCapture()},
        {"error",failure},{"captureOnly",true},{"maximumHoldMs",15000}};
}
}
extern "C" char _xovi_shouldLoad() { return allowed(); }
extern "C" char *ink_diary_capture_ui(const char *arg) {
    QJsonObject result{{"ok",false},{"error","unsupported-runtime"}};
    auto *app=QCoreApplication::instance();
    if (allowed() && app) {
        auto job=[&] { result=request(arg ? QByteArray(arg) : QByteArray()); };
        if (QThread::currentThread()==app->thread()) job();
        else if (!QMetaObject::invokeMethod(app,job,Qt::BlockingQueuedConnection))
            result={{"ok",false},{"error","gui-unavailable"}};
    }
    return strdup(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());
}
