#pragma once
#ifndef INK_DIARY_EXPERIMENTAL_MOVE
#error "Move demo admission requires the global offline-only API build policy"
#endif
// Demo-only admission interface. No extension entry point, network or insertion.
// Never include this to replace the production capture gate without a concrete
// reviewed live-native-pose provider. All methods run on the GUI thread.
#include "../../src/capture-layout.h"
#include "../../src/readonly-mapping.h"
#include <QMetaMethod>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <QPointer>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QSet>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#ifdef INK_MOVE_LIVE_TEST
#include "../move-live/run-directory.h"
#endif

namespace moveDemo {
#ifdef INK_MOVE_LIVE_TEST
inline constexpr const char *attemptDirectory = moveLive::runDirectory;
#else
inline constexpr const char *attemptDirectory = "/home/root/xovi/move-demo-capture-v2";
#endif
inline constexpr const char *attemptMarker = "START_CONSUMED";
inline constexpr const char *evidenceProfile = "chiappa-3.28.0.172-qt6.10.3-probe360x176-left-pose-v1";

// Backend must verify the exact machine/OS/Qt tuple, actual Blank template,
// and obtain native toolbar + overlay state from the actual live GUI,
// not config/JSON assertions. No permissive/default implementation is supplied.
// See README for the precise missing resource query and rejection requirements.
class NativePoseSource {
public:
    virtual ~NativePoseSource() = default;
    virtual QString verifyLivePose(QQuickWindow *window, const QRect &crop, bool suppressed) = 0;
};

inline QString inspectOwnControl(QQuickWindow *window, bool suppressed) {
    if (!qGuiApp || QThread::currentThread()!=qGuiApp->thread()) return "not-gui-thread";
    if (!window || !window->isVisible() || !diaryMoveWindowSupported(window->size()) ||
        window->devicePixelRatio()!=1 || !window->contentItem()) return "unsupported-window";
    QQuickItem *control=nullptr;
    QList<QQuickItem *> pending{window->contentItem()};
    QSet<QQuickItem *> visited;
    while (!pending.isEmpty()) {
        auto *item=pending.takeLast();
        if (!item || visited.contains(item)) return "invalid-scene-tree";
        visited.insert(item);
        if (visited.size()>20000) return "scene-scan-truncated";
        const auto owner=item->property("diaryCaptureOwner").toString();
        if (!owner.isEmpty() || item->objectName()=="moveUiProbe" || item->objectName()=="inkDiaryControl") {
            if (control || item->objectName()!="moveUiProbe" ||
                owner!="ink-diary-owned-control-v1") return "unknown-or-duplicate-own-control";
            control=item;
        }
        for (auto *child:item->childItems()) pending.append(child);
    }
    QRectF expected(window->width()-376,window->height()-192,360,176);
#ifdef INK_MOVE_PER_REQUEST
    // Resolve the unique visible SceneView; a random parent cannot supply the
    // expected rotated owner footprint. Admission still freezes its calibration.
    QQuickItem *view=nullptr;
    for(auto *item:visited)if(item->isVisible() && item->property("controller").value<QObject *>()) {
        bool hasMapping=false;
        for(int i=0;i<item->metaObject()->methodCount();++i)
            if(item->metaObject()->method(i).name()=="viewToScene")hasMapping=true;
        if(hasMapping) {if(view)return "ambiguous-owner-view";view=item;}
    }
    if(view) {
        const auto snapshot=diaryMetadata::viewportSnapshot(qmlEngine(view),view);
        try {diaryMoveFrame(snapshot);}catch(...){return "unsupported-owner-view-map";}
        expected=view->mapRectToScene(QRectF(view->width()-376,view->height()-192,360,176));
        // Bounding-box equality alone would admit reflections and wrong turns.
        for(const QPointF local:{QPointF(0,0),QPointF(360,0),QPointF(0,176),QPointF(360,176)}) {
            if(control && control->mapToScene(local)!=view->mapToScene(QPointF(view->width()-376,view->height()-192)+local))
                return "owner-corner-map-changed";
        }
    }
#endif
    if (!control || control->window()!=window || control->width()!=360 || control->height()!=176 ||
        control->mapRectToScene(control->boundingRect())!=expected ||
        control->rotation()!=0 || control->scale()!=1 || control->opacity()!=1 ||
        control->property("diaryCaptureSuppressed").metaType().id()!=QMetaType::Bool ||
        control->property("diaryCaptureSuppressed").toBool()!=suppressed ||
        control->isVisible()==suppressed) return "own-control-not-exact-tested-profile";
    return {};
}

// Claim at explicit Start, not at insertion. Thus reload, cancellation and any
// uncertain operation cannot reuse a previous attempt. Existing marker contents
// are never read, removed or reset. The parent directory must be separately
// provisioned as private; this interface cannot create a fresh test identity.
inline QString claimAttempt(const QString &directory) {
    const auto path=QFile::encodeName(directory);
    int dirfd=::open(path.constData(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if (dirfd<0) return "attempt-directory-unavailable";
    struct stat st{};
    if (::fstat(dirfd,&st)!=0 || st.st_uid!=::geteuid() || (st.st_mode&0777)!=0700) {
        ::close(dirfd); return "attempt-directory-not-private";
    }
    int fd=::openat(dirfd,attemptMarker,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if (fd<0) {::close(dirfd); return "attempt-already-consumed-or-unavailable";}
    constexpr char record[]="move-demo-capture-v2 explicit Start; never reset\n";
    bool ok=::write(fd,record,sizeof(record)-1)==sizeof(record)-1 && ::fsync(fd)==0;
    if (::close(fd)!=0) ok=false;
    if (::fsync(dirfd)!=0) ok=false;
    ::close(dirfd);
    return ok?QString():QStringLiteral("attempt-claim-uncertain-terminal");
}

class Admission final {
public:
    enum class Phase { Idle, Started, Cancelled, Failed, Finished };
    // The reference is the separately reviewed exact current-page calibration,
    // not a configurable acceptance flag. Native source remains mandatory.
    Admission(QQuickWindow *window, QQuickItem *document, QJsonObject reference, NativePoseSource *native)
        : window_(window), document_(document),
          page_(document ? document->property("currentPageId").toString() : QString()),
          reference_(std::move(reference)), native_(native) {}
    QString start(const QJsonObject &liveViewport) {
        return startInDirectory(liveViewport,QString::fromLatin1(attemptDirectory));
    }
    // Local interface test seam. Not exposed by any loader, command or config.
    QString startInDirectory(const QJsonObject &liveViewport, const QString &directory) {
        if (phase_!=Phase::Idle) return "demo-attempt-terminal-or-already-started";
        auto error=check(liveViewport,false);
        if (!error.isEmpty()) {phase_=Phase::Failed;return error;}
        error=claimAttempt(directory);
        phase_=error.isEmpty()?Phase::Started:Phase::Failed;
        return error;
    }
    // Use at BOTH capture boundaries before dispatch and after decoded PNG;
    // existing DiaryCaptureUiSession supplies actual suppression/restoration.
    QString captureBoundary(const QJsonObject &liveViewport) {
        if (phase_!=Phase::Started) return "demo-not-started";
        auto error=check(liveViewport,true);
        if (!error.isEmpty()) phase_=Phase::Failed;
        return error;
    }
    QString visibleBoundary(const QJsonObject &liveViewport) {
        if (phase_!=Phase::Started) return "demo-not-started";
        auto error=check(liveViewport,false);
        if (!error.isEmpty()) phase_=Phase::Failed;
        return error;
    }
    void cancel() {if (phase_==Phase::Idle || phase_==Phase::Started) phase_=Phase::Cancelled;}
    void finish() {if (phase_==Phase::Started) phase_=Phase::Finished;}
    Phase phase() const {return phase_;}
private:
    QString check(const QJsonObject &viewport,bool suppressed) {
        if (!document_ || document_->window()!=window_ || !document_->isVisible() ||
            !document_->property("documentLoaded").toBool() || page_.isEmpty() ||
            document_->property("currentPageId").toString()!=page_) return "document-or-page-changed";
        auto error=inspectOwnControl(window_,suppressed);
        if (!error.isEmpty()) return error;
        try {diaryExperimentalMoveMapping(viewport,reference_);}
        catch (...) {return "viewport-not-exact-reviewed-blank-reference";}
        if (!native_) return "live-native-pose-source-required";
        return native_->verifyLivePose(window_,diaryMoveBoundedCaptureArea(window_->size()),suppressed);
    }
    QPointer<QQuickWindow> window_;
    QPointer<QQuickItem> document_;
    const QString page_;
    const QJsonObject reference_;
    NativePoseSource *native_; // caller outlives Admission; no QObject ownership transfer
    Phase phase_=Phase::Idle;
};
}
