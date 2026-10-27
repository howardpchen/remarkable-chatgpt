// Shared bounded Move controller, independent of production readiness.
// Default demo build has no API/config access. Only the separate compile-time
// INK_MOVE_LIVE_TEST target includes the single-use asynchronous text client.
#define INK_MOVE_DEMO
#include "../offline-workflow/workflow.cpp"
#include "admission.h"
#include "../../src/capture-ui.h"
#include <memory>
#ifdef INK_MOVE_PER_REQUEST
#include "../move-session/quota.h"
#include "../move-session/page-profile.h"
#include <QDebug>
#endif
#ifdef INK_MOVE_LIVE_TEST
#include "../move-live/client.h"
#endif
namespace moveDemo {
inline QJsonObject viewport(const workflow::Page &page) {
    auto *doc=qobject_cast<QQuickItem *>(page.document);
    const auto name=diaryMetadata::captureTemplateName(qmlEngine(doc),doc);
    if (name.isEmpty()) return {};
    auto out=diagnostic::viewportSnapshot(qmlEngine(page.view),page.view);
    if (!out.isEmpty()) out.insert("templateName",name);
    return out;
}
class LivePose final : public NativePoseSource {
public:
    QJsonObject reference;
#ifdef INK_MOVE_DEMO_HOST_TEST
    std::function<QString()> testVerify;
#endif
    QString verifyLivePose(QQuickWindow *window,const QRect &crop, bool suppressed) override {
#ifdef INK_MOVE_DEMO_HOST_TEST
        if (testVerify) return testVerify();
#endif
        if (!diagnostic::allowed()) return "unsupported-runtime";
        const auto p=workflow::current();
        if (p.window!=window || crop!=diaryMoveBoundedCaptureArea()) return "native-window-or-crop-changed";
        auto *doc=qobject_cast<QQuickItem *>(p.document);
        if (diaryMetadata::captureTemplateName(qmlEngine(doc),doc).isEmpty()) return "supported-template-required";
        const auto pose=diagnostic::nativePose(doc);
        if (!pose.value("ok").toBool() || pose!=reference.value(
#ifdef INK_MOVE_LIVE_TEST
            suppressed?"hiddenNativePose":"nativePose"
#else
            "nativePose"
#endif
        ).toObject()) return "native-ui-pose-changed-or-unavailable";
#ifdef INK_MOVE_LIVE_TEST
        if (suppressed && (!diaryMoveCaptureUiClear(pose) ||
            pose.value("states").toObject().value("toolbar").toObject().value("visible")!=QJsonValue(false) ||
            pose.value("states").toObject().value("toolbar").toObject().value("shown")!=QJsonValue(false)))
            return "capture-ui-visible-or-unavailable";
#endif
        const auto resource=diagnostic::documentViewResource();
        if (!resource.value("ok").toBool() || resource.value("sha256")!=reference.value("resourceSha256")) return "native-resource-changed";
        const auto hashes=reference.value("files").toObject();
        if (hashes.isEmpty() || hashes.size()>64) return "reviewed-mod-manifest-required";
        QSet<QString> actual,reviewed;
        for (const auto &directory:{QStringLiteral("/home/root/xovi/extensions.d"),QStringLiteral("/home/root/xovi/exthome/qt-resource-rebuilder")}) {
            const QDir dir(directory);
            const auto filter=directory.endsWith("extensions.d")?QStringList{"*"}:QStringList{"*.qmd"};
            for (const auto &name:dir.entryList(filter,QDir::Files)) actual.insert(dir.filePath(name));
        }
        for (auto it=hashes.begin();it!=hashes.end();++it) reviewed.insert(it.key());
        if (actual!=reviewed) return "installed-mod-set-changed";
        for (auto it=hashes.begin();it!=hashes.end();++it) {
            // A reference cannot read arbitrary files; only installed extension
            // binary/QMD hashes, no notebook/credential paths.
            const auto path=it.key();
            if ((!path.startsWith("/home/root/xovi/extensions.d/") || !path.endsWith(".so")) &&
                (!path.startsWith("/home/root/xovi/exthome/qt-resource-rebuilder/") || !path.endsWith(".qmd"))) return "invalid-mod-manifest-path";
            if (path.contains("..")) return "invalid-mod-manifest-path";
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly) || f.size()>32*1024*1024) return "mod-unavailable";
            if (QString::fromLatin1(QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex())!=it.value().toString()) return "mod-hash-changed";
        }
        return {};
    }
};
class Runtime final : public QObject {
public:
    explicit Runtime(QObject *parent):QObject(parent) {
        #ifdef INK_MOVE_PER_REQUEST
        qInfo("ink-move lifecycle runtime-created");
#endif
        timer_.setInterval(75);
        connect(&timer_,&QTimer::timeout,this,[this]{ tick(); });
        connect(qGuiApp,&QCoreApplication::aboutToQuit,this,[this]{
#ifdef INK_MOVE_PER_REQUEST
            qInfo("ink-move lifecycle about-to-quit");
#endif
            cancel();
#ifdef INK_MOVE_PER_REQUEST
            qInfo("ink-move lifecycle quit-cancel-complete");
#endif
        });
    }
    ~Runtime() override {
#ifdef INK_MOVE_PER_REQUEST
        qInfo("ink-move lifecycle destructor-begin");
#endif
#ifdef INK_MOVE_LIVE_TEST
        if(client_) client_->cancel();
#endif
        if (lease_) lease_->release();
#ifdef INK_MOVE_PER_REQUEST
        qInfo("ink-move lifecycle destructor-body-end");
#endif
    }
    QJsonObject status() const {
        return {{"ok",true},{"busy",busy_},{"message",message_},{"offlineFixture",
#ifdef INK_MOVE_LIVE_TEST
            false
#else
            true
#endif
        },
            #ifdef INK_MOVE_PER_REQUEST
            {"requestsRemaining",moveSession::remaining(workflow::sessionRoot)},
            {"maxOutputTokens",800},{"automaticRetries",false},{"manualPageArming",false},
            {"canStart",!busy_ && moveSession::remaining(workflow::sessionRoot)>0 && QFile::exists(workflow::sessionRoot+"/device-profile.json") && QFile::exists(configPath_)},
#else
            {"canStart",!started_ && !QFile::exists(workflow::base+"/START_CONSUMED") && !QFile::exists(workflow::base+"/INSERTION_CONSUMED") && QFile::exists(workflow::base+"/reviewed-reference.json")
#ifdef INK_MOVE_LIVE_TEST
                && QFile::exists(configPath_)
#endif
            },
            #endif
            {"stage",int(stage_)},{"attemptConsumed",QFile::exists(workflow::base+"/START_CONSUMED")},
            {"insertionAttempted",workflow::phase==workflow::Phase::Consumed}};
    }
    void start() {
        #ifdef INK_MOVE_PER_REQUEST
        if (busy_) return; // A duplicate must not cancel or mutate the running request.
        // Dispose the previous transport only after its completion stack unwound.
        if(client_) {client_->cancel();client_->deleteLater();client_.clear();}
        admission_.reset(); ready_=false;
        workflow::phase=workflow::Phase::Idle;
        workflow::question={};workflow::answer={};workflow::liveResponse.clear();
        workflow::base=workflow::sessionRoot;
#else
        if (started_) { message_="Single-use test; no retry"; return; }
#endif
        started_=true;
#ifdef INK_MOVE_LIVE_TEST
        try {key_=moveLive::loadKey(configPath_);}
        catch(const std::exception &e) {stop(QString::fromUtf8(e.what()));return;}
#endif
        const auto p=workflow::current();
#ifdef INK_MOVE_PER_REQUEST
        try {
            native_.reference=moveSession::referenceForStart(
                moveSession::loadProfile(workflow::sessionRoot+"/device-profile.json"),viewport(p));
        } catch(const std::exception &e) {stop(QString::fromUtf8(e.what()));return;}
#else
        QFile file(workflow::base+"/reviewed-reference.json");
        if (!file.open(QIODevice::ReadOnly) || file.size()>1024*1024) { stop("Reviewed UI reference required"); return; }
        native_.reference=QJsonDocument::fromJson(file.readAll()).object();
#ifdef INK_MOVE_LIVE_TEST
        if (native_.reference.value("schema").toInt()!=2 ||
            native_.reference.value("nativePose").toObject().isEmpty() ||
            native_.reference.value("hiddenNativePose").toObject().isEmpty() ||
            native_.reference.value("captureArea").toArray()!=QJsonArray{0,0,954,1696}) {
            stop("Reviewed visible and toolbar-hidden full-screen reference required");return;
        }
#else
        if (native_.reference.value("schema").toInt()!=1) {stop("Invalid reviewed reference");return;}
#endif
#endif // The one-shot build retains its old per-page reference contract.
#ifdef INK_MOVE_PER_REQUEST
        const auto requestDir=moveSession::reserve(workflow::sessionRoot);
        if(requestDir.isEmpty()) {stop("Request quota exhausted or unavailable");return;}
        workflow::base=requestDir;
        qInfo("ink-move lifecycle request-reserved");
#endif
        admission_=std::make_unique<Admission>(p.window,qobject_cast<QQuickItem *>(p.document),
            native_.reference.value("viewport").toObject(),&native_);
#if defined(INK_MOVE_DEMO_HOST_TEST) || defined(INK_MOVE_PER_REQUEST)
        auto error=admission_->startInDirectory(viewport(p),workflow::base);
#else
        auto error=admission_->start(viewport(p));
#endif
        if (!error.isEmpty()) {stop(error);return;}
        const auto prep=workflow::run({{"op","prepare"},{"measuredCrop",QJsonArray{diaryMoveBoundedCaptureArea().x(),diaryMoveBoundedCaptureArea().y(),diaryMoveBoundedCaptureArea().width(),diaryMoveBoundedCaptureArea().height()}}});
        if (!prep.value("ok").toBool()) {stop(prep.value("error").toString());return;}
        busy_=true; stage_=Stage::BeforeFirst; clock_.start(); pause_.start();
        message_=
#ifdef INK_MOVE_LIVE_TEST
            "Live text test: tap Cancel to stop";
#else
            "Offline fixture: tap Cancel to stop";
#endif
        timer_.start();
    }
    void cancel() {
        started_=true;
        if (admission_) admission_->cancel();
        if (busy_) workflow::run({{"op","cancel"}});
        stop("Cancelled; no automatic retry");
    }
private:
#ifdef INK_MOVE_DEMO_HOST_TEST
    friend struct RuntimeTest;
#endif
    enum class Stage { Idle, BeforeFirst, SuppressFirst, CaptureFirst, AwaitAnswer, BeforeSecond, SuppressSecond, CaptureSecond, Terminal };
    void stop(const QString &reason) {
#ifdef INK_MOVE_LIVE_TEST
        if(client_) client_->cancel();
        key_.clear();
#endif
        if (busy_ && workflow::phase!=workflow::Phase::Consumed) workflow::run({{"op","cancel"}});
        #ifdef INK_MOVE_PER_REQUEST
        qInfo("ink-move lifecycle request-terminal stage=%d",int(stage_));
#endif
        timer_.stop(); busy_=false; stage_=Stage::Terminal; message_=reason;
        if (lease_) {lease_->release();lease_->deleteLater();lease_.clear();}
        if (admission_ && admission_->phase()==Admission::Phase::Started) admission_->cancel();
    }
    bool boundary() {
        if (!lease_ || !lease_->readyForCapture()) {stop("Capture UI lease unavailable");return false;}
        const auto error=admission_->captureBoundary(viewport(workflow::current()));
        if (!error.isEmpty()) {stop(error);return false;}
        return true;
    }
    void suppress(bool second) {
        stage_=second?Stage::SuppressSecond:Stage::SuppressFirst;
        const auto p=workflow::current();
        lease_=new DiaryCaptureUiSession(p.window,[this,second](auto event,const QString &error) {
            // Callback can originate synchronously in begin(). Never delete
            // the notifying session from its own stack; stop uses deleteLater.
            if (!busy_) return;
            if (event!=DiaryCaptureUiSession::Event::RenderReady) {stop(error);return;}
            // Give the supported framebuffer backend time to see the rendered
            // hidden control, as independently hardware-validated in the lease test.
            readyDelay_.start(); captureSecond_=second; ready_=true;
        },this);
        ready_=false; lease_->begin();
    }
    void tick() {
        if (!busy_) return;
        if (clock_.elapsed()>overallTimeoutMs_) {stop("Demo timed out; controls restored");return;}
#ifdef INK_MOVE_LIVE_TEST
        if(stage_==Stage::AwaitAnswer) {
            if(!liveValid()) stop("Page, viewport or native UI changed during API request");
            return;
        }
#endif
        if (stage_==Stage::BeforeFirst || stage_==Stage::BeforeSecond) {
            // Stay cancellable with visible UI before each screenshot.
            if (pause_.elapsed()<pauseMs_) return;
            suppress(stage_==Stage::BeforeSecond); return;
        }
        if (stage_==Stage::SuppressFirst || stage_==Stage::SuppressSecond) {
            if (!ready_ || readyDelay_.elapsed()<settleMs_) return;
            if (!boundary()) return;
            const auto result=workflow::run({{"op",captureSecond_?"verify":"capture"}});
            if (!result.value("ok").toBool()) {stop(result.value("error").toString());return;}
            stage_=captureSecond_?Stage::CaptureSecond:Stage::CaptureFirst;
            return;
        }
        if (stage_!=Stage::CaptureFirst && stage_!=Stage::CaptureSecond) return;
        if (!boundary()) return;
        const bool second=stage_==Stage::CaptureSecond;
        // Decode while suppressed; call the backend only after PNG availability
        // and the final native-pose/template/page/viewport check on this GUI turn.
        QImage decoded(workflow::base+(second?"/check.png":"/question.png"));
        if (decoded.isNull()) {
            if (workflow::captureClock.elapsed()>captureTimeoutMs_) stop("Capture timed out; controls restored");
            return;
        }
        if (!boundary()) return;
#ifdef INK_MOVE_LIVE_TEST
        const auto result=workflow::run({{"op",second?"insert":"read-question"}});
#else
        const auto result=workflow::run({{"op",second?"insert":"render"}});
#endif
        if (!result.value("ok").toBool()) {stop(result.value("error").toString());return;}
        lease_->release(); lease_->deleteLater(); lease_.clear();
        if (second) {
            admission_->finish(); stop(
#ifdef INK_MOVE_LIVE_TEST
                "Live text inserted; verify placement and Undo/Redo"
#else
                "Offline fixture inserted; verify placement and Undo/Redo"
#endif
            );
        } else {
#ifdef INK_MOVE_LIVE_TEST
            #ifdef INK_MOVE_PER_REQUEST
            qInfo("ink-move lifecycle api-dispatch-boundary");
#endif
            stage_=Stage::AwaitAnswer; message_="Thinking; Cancel available";
            client_=new moveLive::Client(this);
            client_->start(workflow::question,key_,[this]{return liveValid();},
                [this](QString text,QString error) {
                    if(!busy_ || stage_!=Stage::AwaitAnswer)return;
                    if(!error.isEmpty()) {stop(error);return;}
                    if(!liveValid()) {stop("Page changed before rendering answer");return;}
                    workflow::liveResponse=std::move(text);
                    const auto result=workflow::run({{"op","render"}});
                    if(!result.value("ok").toBool()) {stop(result.value("error").toString());return;}
#ifdef INK_MOVE_SINGLE_CAPTURE
                    // Explicit owner-authorized trial: no second suppression/capture.
                    // Still validate visible native UI/page/template/viewport immediately.
                    if(!liveValid()) {stop("Page changed before single-capture insertion");return;}
                    const auto inserted=workflow::run({{"op","insert"}});
                    if(!inserted.value("ok").toBool()) {stop(inserted.value("error").toString());return;}
                    admission_->finish();
                    stop("Live text inserted (single capture); verify placement and Undo/Redo");
#else
                    stage_=Stage::BeforeSecond; pause_.restart();message_="Answer rendered; Cancel available";
#endif
                }
#ifdef INK_MOVE_DEMO_HOST_TEST
                ,testManager_,testApiTimeoutMs_
#endif
            );
            key_.clear();
#else
            stage_=Stage::BeforeSecond; pause_.restart(); message_="Fixture rendered; Cancel still available";
#endif
        }
    }
#ifdef INK_MOVE_LIVE_TEST
    bool liveValid() {
        return busy_ && stage_==Stage::AwaitAnswer && workflow::fresh(workflow::current()) &&
            admission_ && admission_->visibleBoundary(viewport(workflow::current())).isEmpty();
    }
    QPointer<moveLive::Client> client_;
    QString key_,configPath_="/home/root/.config/ink-diary/config.json";
#ifdef INK_MOVE_DEMO_HOST_TEST
    QNetworkAccessManager *testManager_=nullptr;
    int testApiTimeoutMs_=120000;
#endif
#endif
    QTimer timer_; QElapsedTimer clock_,pause_,readyDelay_;
    int pauseMs_=1500,settleMs_=300,captureTimeoutMs_=10000,overallTimeoutMs_=
#ifdef INK_MOVE_LIVE_TEST
        150000;
#else
        30000;
#endif
    QPointer<DiaryCaptureUiSession> lease_;
    LivePose native_; std::unique_ptr<Admission> admission_;
    Stage stage_=Stage::Idle; bool busy_=false,started_=false,ready_=false,captureSecond_=false;
    QString message_=
#ifdef INK_MOVE_PER_REQUEST
        "Normal zoom: Blank, Dots small or Grid small";
#elif defined(INK_MOVE_LIVE_TEST)
        "Live text: config and new reviewed reference required";
#else
        "Awaiting reviewed offline reference";
#endif
};
QPointer<Runtime> runtime;
}
#ifdef INK_MOVE_LIVE_TEST
extern "C" char *ink_diary_move_live(
#else
extern "C" char *ink_diary_move_demo(
#endif
const char *value) {
    QJsonObject out{{"ok",false},{"error","unsupported-runtime"}};
    auto *app=QCoreApplication::instance();
    if (diagnostic::allowed() && app) {
        const QByteArray command=value?QByteArray(value):QByteArray();
        auto job=[&] {
            if (command=="inspect") {out=diagnostic::collect(true);return;}
            if (!moveDemo::runtime) moveDemo::runtime=new moveDemo::Runtime(app);
            if (command=="start") moveDemo::runtime->start();
            else if (command=="cancel") moveDemo::runtime->cancel();
            else if (command!="status") {out={{"ok",false},{"error","unsupported-command"}};return;}
            out=moveDemo::runtime->status();
        };
        if (QThread::currentThread()==app->thread()) job();
        else if (!QMetaObject::invokeMethod(app,job,Qt::BlockingQueuedConnection)) out={{"ok",false},{"error","gui-unavailable"}};
    }
    return strdup(QJsonDocument(out).toJson(QJsonDocument::Compact).constData());
}
