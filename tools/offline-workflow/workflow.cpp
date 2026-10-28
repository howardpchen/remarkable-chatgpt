#if defined(INK_MOVE_SINGLE_CAPTURE) && (!defined(INK_MOVE_LIVE_TEST) || !defined(INK_MOVE_PER_REQUEST))
#error "Single capture is only an explicitly built per-request live trial"
#endif
// Shared bounded capture/render/insertion workflow; no production Move support.
// Default is a fixed offline fixture. Separate live build freezes the question
// before networking and accepts text only through its validated C++ client.
#include "../diagnostic/probe.cpp"
#include "../../src/capture-layout.h"
#include "../../src/layout.h"
#ifdef INK_MOVE_PER_REQUEST
#include "../../src/move-template-ink.h"
#endif
#include "../../src/markdown.h"
#include <QDir>
#include <QFontDatabase>
#include <QPointer>
#include <QElapsedTimer>
#include <QUrl>
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#ifdef INK_MOVE_LIVE_TEST
#include "../move-live/run-directory.h"
#endif
#ifndef INK_WORKFLOW_HOST_TEST
#include "xovi.h"
#endif
namespace workflow {
#ifdef INK_MOVE_PER_REQUEST
#ifdef INK_WORKFLOW_HOST_TEST
const QString sessionRoot=QString::fromLocal8Bit(qgetenv("INK_WORKFLOW_TEST_DIR"));
const QString fontBase=QStringLiteral("fonts");
#else
const QString sessionRoot=QString::fromLatin1(moveLive::runDirectory);
const QString fontBase=sessionRoot;
#endif
QString base=sessionRoot;
#endif
#ifndef INK_MOVE_PER_REQUEST
#ifdef INK_WORKFLOW_HOST_TEST
const QString base=QString::fromLocal8Bit(qgetenv("INK_WORKFLOW_TEST_DIR"));
std::function<char *(char *)> captureHandler;
const QString fontBase=QStringLiteral("fonts");
#else
#if defined(INK_MOVE_LIVE_TEST)
const QString base=QString::fromLatin1(moveLive::runDirectory);
#elif defined(INK_MOVE_DEMO)
const QString base=QStringLiteral("/home/root/xovi/move-demo-capture-v2");
#else
const QString base=QStringLiteral("/home/root/xovi/offline-workflow-v1");
#endif
const QString fontBase=base;
#endif
#endif
#ifdef INK_MOVE_PER_REQUEST
#ifdef INK_WORKFLOW_HOST_TEST
std::function<char *(char *)> captureHandler;
#endif
#endif
struct Page { QQuickWindow *window=nullptr; QObject *document=nullptr; QObject *controller=nullptr; QQuickItem *view=nullptr; QString id; bool truncated=false; };
Page current() {
    Page p; QList<QObject *> pending; QSet<QObject *> seen;
    int windows=0,documents=0,views=0;
    for(auto *w:QGuiApplication::allWindows())if(w && w->isVisible()) {
        auto *q=qobject_cast<QQuickWindow *>(w); if(!q)continue;
        ++windows;p.window=q;pending.append(q->contentItem());
    }
    while(!pending.isEmpty() && seen.size()<100000) {
        auto *o=pending.takeLast();if(!o || seen.contains(o))continue;
        seen.insert(o);pending.append(o->children());
        auto *item=qobject_cast<QQuickItem *>(o);if(!item)continue;
        for(auto *c:item->childItems())pending.append(c);
        if(!item->isVisible())continue;
        if(o->property("documentLoaded").toBool()) {
            auto *c=o->property("sceneController").value<QObject *>();
            const auto id=o->property("currentPageId").toString();
            if(c && !id.isEmpty()) {++documents;p.document=o;p.controller=c;p.id=id;}
        }
    }
    p.truncated=!pending.isEmpty();
    for(auto *o:seen) {
        auto *v=qobject_cast<QQuickItem *>(o);
        if(v && v->isVisible() && o->property("controller").value<QObject *>()==p.controller &&
           !diagnostic::signatures(o,"viewToScene").isEmpty()) {++views;p.view=v;}
    }
    if(windows!=1 || documents!=1 || views!=1 || p.truncated)return {};
    return p;
}
int placementInkRow(const QImage &image) {
#ifdef INK_MOVE_PER_REQUEST
    const auto p=current();
    if(!p.view || !p.document)return image.height()-1;
    const auto name=diaryMetadata::captureTemplateName(qmlEngine(p.view),p.document);
    const auto snap=diagnostic::viewportSnapshot(qmlEngine(p.view),p.view);
    const auto points=snap.value("points").toArray();
    if(points.size()!=3)return image.height()-1;
    const double scale=diaryMoveAxes(points).scale;
    try {return moveTemplateInk::lastRow(image,name,scale);}
    catch(const std::exception &) {return image.height()-1;} // no placement space on uncertainty
#else
    return lastInkRow(image);
#endif
}
// Only fixed code is evaluated; no user script, path, or expression is accepted.
bool validDimensions(qreal width,qreal height) {
    return std::isfinite(width) && std::isfinite(height) && width>0 && height>0;
}
QString geometry(const Page &p) {
    if(!p.view || !validDimensions(p.view->width(),p.view->height()))return {};
    auto *e=qmlEngine(p.view);if(!e)return {};
    auto f=e->evaluate(QStringLiteral(R"JS((function(v,quarterTurns){try {
        if(!v || !isFinite(v.width) || !isFinite(v.height) || v.width<=0 || v.height<=0)return '';
        let pts=[];
        for(const xy of [[0,0],[100,0],[0,100],[477,700]]) {
            const q=v.viewToScene(v.mapFromItem(null,xy[0],xy[1]));
            if(!q || !isFinite(q.x) || !isFinite(q.y))return '';
            pts.push([q.x,q.y]);
        }
        const dx=pts[1][0]-pts[0][0],dy=pts[2][1]-pts[0][1];
        if(!quarterTurns && (dx<=0 || dy<=0 || Math.abs(pts[1][1]-pts[0][1])>0.001 ||
           Math.abs(pts[2][0]-pts[0][0])>0.001))return '';
        if(Math.abs(pts[3][0]-(pts[0][0]+4.77*dx+7*(pts[2][0]-pts[0][0])))>0.001 ||
           Math.abs(pts[3][1]-(pts[0][1]+7*dy+4.77*(pts[1][1]-pts[0][1])))>0.001)return '';
        return JSON.stringify({width:v.width,height:v.height,points:pts});
    }catch(e){return '';}}))JS"));
    if(!f.isCallable())return {};
    auto own=QQmlEngine::objectOwnership(p.view);auto obj=e->newQObject(p.view);
    QQmlEngine::setObjectOwnership(p.view,own);
    auto r=f.call({obj,
#ifdef INK_MOVE_PER_REQUEST
        true
#else
        false
#endif
    });
    if(r.isError())return {};
    try {diaryMoveAxes(QJsonDocument::fromJson(r.toString().toUtf8()).object().value("points").toArray());}
    catch(...) {return {};}
    #ifdef INK_MOVE_PER_REQUEST
    auto frozen=QJsonDocument::fromJson(r.toString().toUtf8()).object();
    const auto snapshot=diagnostic::viewportSnapshot(e,p.view);
    if(snapshot.isEmpty())return {};
    for(const auto *field:{"windowWidth","windowHeight","devicePixelRatio","windowCorners"})frozen.insert(field,snapshot.value(field));
    try {diaryMoveFrame(frozen);}catch(...){return {};}
    return QString::fromUtf8(QJsonDocument(frozen).toJson(QJsonDocument::Compact));
#else
    return r.toString();
#endif
}
enum class Phase { Idle, Prepared, Capturing, AwaitingAnswer, Rendered, Verifying, Consumed, Cancelled };
#ifdef INK_MOVE_LIVE_TEST
constexpr bool fixtureOnly=false;
#else
constexpr bool fixtureOnly=true;
#endif
Phase phase=Phase::Idle;
QPointer<QQuickWindow> originWindow;
QPointer<QObject> originDocument,originController;
QPointer<QQuickItem> originView;
QString originId,originGeometry;
QSize originSize;
QRect area;
QImage question,answer;
QPointF target;
QElapsedTimer captureClock;
#ifdef INK_MOVE_LIVE_TEST
QString liveResponse; // Set only by the validated C++ API completion, never broker JSON.
#endif
const QString fixedResponse=QStringLiteral("## Offline fixture\n\nThis is a fixed test response, not recognition of your handwriting.\n\n- Capture checked\n- Native image test");
QJsonObject fail(const char *e){return {{"ok",false},{"error",e}};}
bool samePixels(const QImage &a,const QImage &b) {
    return !a.isNull() && a.convertToFormat(QImage::Format_RGBA8888)==b.convertToFormat(QImage::Format_RGBA8888);
}
bool fresh(const Page &p) {
    return p.window && p.window==originWindow && p.document==originDocument &&
        p.controller==originController && p.view==originView && p.id==originId &&
        p.window->size()==originSize && p.window->devicePixelRatio()==1 &&
        geometry(p)==originGeometry;
}
QJsonObject startCapture(const QString &name,Phase next) {
    const QString path=base+name;
    if(QFile::exists(path))return fail("capture-path-already-exists");
    QByteArray arg=(path+",0,0").toUtf8();
#ifdef INK_WORKFLOW_HOST_TEST
    char *result=captureHandler?captureHandler(arg.data()):nullptr;
#else
    char *result=rm_shot$screenshotHandler(arg.data());
#endif
    const bool ok=result && QByteArray(result)=="success";free(result);
    // A failed/uncertain dispatch is terminal; never repeat automatically.
    if(!ok){phase=Phase::Cancelled;return fail("capture-start-failed");}
    phase=next;captureClock.start();
    return {{"ok",true},{"capturePending",true},{"backend","rm-shot"}};
}
QJsonObject run(const QJsonObject &request) {
    const QString op=request.value("op").toString();
    if(op=="status")return {{"ok",true},{"phase",int(phase)},
        {"insertionConsumed",QFile::exists(base+"/INSERTION_CONSUMED")},
        {"fixtureOnly",fixtureOnly}};
    if(op=="cancel") {phase=Phase::Cancelled;return {{"ok",true},{"cancelled",true}};}
    #ifdef INK_MOVE_LIVE_TEST
    const bool readQuestion=op=="read-question";
#else
    const bool readQuestion=false;
#endif
    if(!readQuestion && op!="prepare" && op!="capture" && op!="render" && op!="verify" && op!="insert")return fail("unknown-operation");
    if(phase==Phase::Consumed || QFile::exists(base+"/INSERTION_CONSUMED"))return fail("insertion-already-consumed");
    if(phase==Phase::Cancelled)return fail("workflow-terminal");
    const auto p=current();
    if(op=="prepare") {
        if(phase!=Phase::Idle)return fail("already-prepared");
        if(!p.window || !diaryMoveWindowSupported(p.window->size()) || p.window->devicePixelRatio()!=1)return fail("unsupported-window");
        const auto g=geometry(p);if(g.isEmpty())return fail("unsupported-geometry");
        const auto crop=request.value("measuredCrop").toArray();
        if(crop.size()!=4)return fail("explicit-measured-crop-required");
        for(const auto &v:crop)if(!v.isDouble() || !std::isfinite(v.toDouble()) || v.toDouble()!=v.toInt(-1))return fail("invalid-crop");
        area=QRect(crop[0].toInt(),crop[1].toInt(),crop[2].toInt(),crop[3].toInt());
        // Historical offline fixtures exclude native UI geometrically.
        // The live target instead requires the reviewed fully suppressed pose.
#ifdef INK_MOVE_LIVE_TEST
        // Exact live crop only; the controller verifies all native UI
        // at admission and both suppressed capture boundaries.
        if(area!=diaryMoveBoundedCaptureArea(p.window->size()))return fail("invalid-live-crop");
#else
        if(area.x()<112 || area.y()<112)return fail("crop-includes-observed-ui");
#endif
        try {diaryCaptureLayout(p.window->size(),p.window->size(),1,DiaryCaptureProfile::ExperimentalMove,area);}
        catch(const std::exception &){return fail("invalid-crop");}
        if(!QDir().mkpath(base) || !QFile::setPermissions(base,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner))return fail("private-directory-failed");
        // Existing run artifacts are never overwritten or treated as fresh.
        for(const auto &name:{"/question.png","/check.png","/answer.png"})if(QFile::exists(base+name))return fail("existing-run-artifacts");
        originSize=p.window->size();originWindow=p.window;originDocument=p.document;originController=p.controller;
        originView=p.view;originId=p.id;originGeometry=g;phase=Phase::Prepared;
        return {{"ok",true},{"prepared",true},{"fixtureOnly",fixtureOnly}};
    }
    if(!fresh(p)){phase=Phase::Cancelled;return fail("stale-or-unprepared-context");}
    if(op=="capture") {
        if(phase!=Phase::Prepared)return fail("wrong-phase");
        return startCapture("/question.png",Phase::Capturing);
    }
    if(op=="verify") {
        if(phase!=Phase::Rendered)return fail("wrong-phase");
        return startCapture("/check.png",Phase::Verifying);
    }
    if(readQuestion || op=="render" || op=="insert") {
        const bool first=readQuestion || op=="render";
#ifdef INK_MOVE_LIVE_TEST
        const auto expected=readQuestion?Phase::Capturing:(first?Phase::AwaitingAnswer:
#ifdef INK_MOVE_SINGLE_CAPTURE
            Phase::Rendered
#else
            Phase::Verifying
#endif
        );
#else
        const auto expected=first?Phase::Capturing:Phase::Verifying;
#endif
        if(phase!=expected)return fail("wrong-phase");
        const bool frozenImageOnly=
#ifdef INK_MOVE_SINGLE_CAPTURE
            !first ||
#endif
            (first && phase==Phase::AwaitingAnswer);
        if(!frozenImageOnly && captureClock.elapsed()>10000){phase=Phase::Cancelled;return fail("capture-timeout");}
        const QImage screen(base+
#ifdef INK_MOVE_SINGLE_CAPTURE
            "/question.png"
#else
            (first?"/question.png":"/check.png")
#endif
        );
        if(screen.isNull())return fail("capture-pending");
        if(screen.size()!=originSize){phase=Phase::Cancelled;return fail("capture-geometry-changed");}
        if(!fresh(current())){phase=Phase::Cancelled;return fail("context-changed-during-capture");}
        #ifdef INK_MOVE_PER_REQUEST
        const auto frozenViewport=QJsonDocument::fromJson(originGeometry.toUtf8()).object();
        QImage reading;
        try {reading=diaryMoveReadingImage(screen,frozenViewport);}
        catch(...) {phase=Phase::Cancelled;return fail("capture-view-map-changed");}
        const QRect contentArea(QPoint(0,0),reading.size());
        const auto cropped=reading.copy(contentArea);
#else
        const QRect contentArea=area;
        const auto cropped=screen.copy(area);
#endif
        #ifdef INK_MOVE_LIVE_TEST
        if(readQuestion) {
            if(placementInkRow(cropped)<0){phase=Phase::Cancelled;return fail("no-visible-ink");}
            question=cropped; phase=Phase::AwaitingAnswer;
            return {{"ok",true}};
        }
        if(first && (!samePixels(cropped,question) || liveResponse.isEmpty())) {
            phase=Phase::Cancelled;return fail("question-changed-or-answer-missing");
        }
#endif
        if(op=="render") {
            const int last=placementInkRow(cropped);
            if(last<0){phase=Phase::Cancelled;return fail("no-visible-ink");}
            const int font=QFontDatabase::addApplicationFont(fontBase+"/Caveat.ttf");
            if(font<0 || QFontDatabase::addApplicationFont(fontBase+"/DejaVuSansMono.ttf")<0){phase=Phase::Cancelled;return fail("fixture-fonts-missing");}
            try {answer=renderDiaryMarkdown(
#ifdef INK_MOVE_LIVE_TEST
                liveResponse,
#else
                fixedResponse,
#endif
QFontDatabase::applicationFontFamilies(font).first(),
#ifdef INK_MOVE_LIVE_TEST
contentArea.width()-120 // Keep answer placement clear of the restored left toolbar.
#else
area.width()
#endif
).image;}
            catch(const std::exception &){phase=Phase::Cancelled;return fail("render-failed");}
            const auto pts=QJsonDocument::fromJson(originGeometry.toUtf8()).object().value("points").toArray();
            const auto axes=
#ifdef INK_MOVE_PER_REQUEST
                diaryMoveReadingAxes(frozenViewport);
#else
                diaryMoveAxes(pts);
#endif
            const qreal sx=axes.scale, sy=axes.scale;
            // Native image pixels occupy scene units, not window pixels.
            const qreal nw=std::floor(answer.width()*sx),nh=std::floor(answer.height()*sy);
            if(!std::isfinite(nw) || !std::isfinite(nh) || nw<1 || nh<1 ||
               nw>4096 || nh>4096 || nw*nh>16000000){phase=Phase::Cancelled;return fail("native-image-budget-exceeded");}
            answer=answer.scaled(int(nw),int(nh),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            const qreal screenWidth=answer.width()/sx,screenHeight=answer.height()/sy;
            const int top=contentArea.y()+last+45;
            if(answer.isNull() || top+screenHeight>contentArea.y()+contentArea.height()){phase=Phase::Cancelled;return fail("answer-does-not-fit-visible-crop");}
            if(axes.degrees)answer=answer.transformed(QTransform().rotate(axes.degrees));
            if(!answer.save(base+"/answer.png") || !QFile::setPermissions(base+"/answer.png",QFileDevice::ReadOwner|QFileDevice::WriteOwner)){phase=Phase::Cancelled;return fail("answer-save-failed");}
            question=cropped;
            const qreal x=
#ifdef INK_MOVE_LIVE_TEST
                120+(contentArea.width()-120)/2.0,
#else
                area.x()+area.width()/2.0,
#endif
                y=top+screenHeight/2.0;
            target=axes.map(QPointF(x,y));
            if(!std::isfinite(target.x()) || !std::isfinite(target.y())){phase=Phase::Cancelled;return fail("invalid-insertion-target");}
            phase=Phase::Rendered;
            return {{"ok",true},{"fixtureOnly",fixtureOnly},{"answerWidth",answer.width()},{"answerHeight",answer.height()},
                {"screenWidth",screenWidth},{"screenHeight",screenHeight},{"lastInkRow",last},{"answerTop",top},{"verifiedPlaced",false}};
        }
#ifndef INK_MOVE_SINGLE_CAPTURE
        if(!samePixels(cropped,question)){phase=Phase::Cancelled;return fail("page-pixels-changed");}
#endif
        // Single-capture trial intentionally cannot detect same-page ink edits.
        // Answer integrity, native signature, context and durable claim stay enforced.
        if(!samePixels(QImage(base+"/answer.png"),answer)){phase=Phase::Cancelled;return fail("answer-pixels-changed");}
        if(diagnostic::signatures(p.controller,"insertImageFileAsSceneItem")!=QJsonArray{QStringLiteral("insertImageFileAsSceneItem(QUrl,QPointF)")})return fail("unexpected-insertion-signature");
        if(!fresh(current())){phase=Phase::Cancelled;return fail("context-changed-before-insertion");}
        // Durable across reloads: consume BEFORE native invocation, including
        // uncertain invocation results. There is deliberately no reset operation.
        QFile marker(base+"/INSERTION_CONSUMED");
        if(!marker.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail("cannot-consume-one-shot");
        marker.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
        const QByteArray bytes="offline-workflow-v1 insertion attempt consumed; never retry\n";
        bool persisted=marker.write(bytes)==bytes.size() && marker.flush() && ::fsync(marker.handle())==0;marker.close();
        const int directory=::open(QFile::encodeName(base).constData(),O_RDONLY|O_DIRECTORY);
        if(directory<0)persisted=false;
        else {persisted=(::fsync(directory)==0) && persisted;::close(directory);}
        phase=Phase::Consumed;
        if(!persisted)return fail("one-shot-write-uncertain");
        const bool invoked=QMetaObject::invokeMethod(p.controller,"insertImageFileAsSceneItem",Qt::DirectConnection,
            Q_ARG(QUrl,QUrl::fromLocalFile(base+"/answer.png")),Q_ARG(QPointF,target));
        return {{"ok",invoked},{"insertionAttempted",true},{"verifiedPlaced",false},{"fixtureOnly",fixtureOnly}};
    }
    return fail("wrong-phase");
}
}
extern "C" char *ink_diary_workflow(const char *value) {
    QJsonObject result=workflow::fail("unsupported-runtime");auto *app=QCoreApplication::instance();
    if(diagnostic::allowed() && app) {
        const QByteArray raw(value?value:"");
        if(raw.size()>1024)return strdup("{\"ok\":false,\"error\":\"request-too-large\"}");
        const auto request=QJsonDocument::fromJson(raw);
        result=diaryDispatch::run([request]{
            return request.isObject()?workflow::run(request.object()):workflow::fail("invalid-request");
        });
    }
    return strdup(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());
}
