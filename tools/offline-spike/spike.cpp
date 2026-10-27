// Bounded offline spike; production firmware support remains disabled.
#include "../diagnostic/probe.cpp"
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QPointer>
#include <QPointF>
#include <QUrl>
#include <cmath>
namespace spike {
#ifdef INK_SPIKE_HOST_TEST
const QString base=QString::fromLocal8Bit(qgetenv("INK_SPIKE_TEST_DIR"));
#else
const QString base=QStringLiteral("/home/root/xovi/offline-spike");
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
// Only fixed code is evaluated; no user script, path, or expression is accepted.
bool validDimensions(qreal width,qreal height) {
    return std::isfinite(width) && std::isfinite(height) && width>0 && height>0;
}
QString geometry(const Page &p) {
    if(!p.view || !validDimensions(p.view->width(),p.view->height()))return {};
    auto *e=qmlEngine(p.view);if(!e)return {};
    auto f=e->evaluate(QStringLiteral(R"JS((function(v){try {
        if(!v || !isFinite(v.width) || !isFinite(v.height) || v.width<=0 || v.height<=0)return '';
        let pts=[];
        for(const xy of [[0,0],[100,0],[0,100],[477,700]]) {
            const q=v.viewToScene(v.mapFromItem(null,xy[0],xy[1]));
            if(!q || !isFinite(q.x) || !isFinite(q.y))return '';
            pts.push([q.x,q.y]);
        }
        const dx=pts[1][0]-pts[0][0],dy=pts[2][1]-pts[0][1];
        if(dx<=0 || dy<=0 || Math.abs(pts[1][1]-pts[0][1])>0.001 ||
           Math.abs(pts[2][0]-pts[0][0])>0.001)return '';
        return JSON.stringify({width:v.width,height:v.height,points:pts});
    }catch(e){return '';}}))JS"));
    if(!f.isCallable())return {};
    auto own=QQmlEngine::objectOwnership(p.view);auto obj=e->newQObject(p.view);
    QQmlEngine::setObjectOwnership(p.view,own);
    auto r=f.call({obj});return r.isError()?QString():r.toString();
}
QPointer<QObject> originDocument,originController;
QPointer<QQuickItem> originView;
QString originId,originGeometry;
bool prepared=false,attempted=false;
QJsonObject fail(const char *e){return {{"ok",false},{"error",e}};}
QImage calibrationImage() {
    QImage fixture(240,160,QImage::Format_RGB32);fixture.fill(Qt::white);
    QPainter paint(&fixture);paint.setPen(QPen(Qt::black,4));paint.drawRect(4,4,231,151);
    paint.drawLine(120,15,120,145);paint.drawLine(15,80,225,80);
    paint.fillRect(20,20,25,25,Qt::black);paint.end();
    return fixture;
}
bool isCalibrationImage(const QImage &fixture) {
    // Compare decoded canonical RGBA pixels, not PNG encoding or dimensions
    // alone. Alpha is significant: an invisible same-size image is not valid.
    return fixture.size()==QSize(240,160) &&
        fixture.convertToFormat(QImage::Format_RGBA8888)==
        calibrationImage().convertToFormat(QImage::Format_RGBA8888);
}
QJsonObject run(const QByteArray &op) {
    if(op!="prepare" && op!="insert" && op!="capture")return fail("unknown-operation");
    auto p=current();if(!p.window || !p.document || !p.controller || !p.view)return fail("ambiguous-or-missing-page");
    if(p.window->size()!=QSize(954,1696) || p.window->devicePixelRatio()!=1)return fail("unsupported-window");
    auto g=geometry(p);if(g.isEmpty())return fail("unsupported-geometry");
    if(op=="prepare") {
        if(prepared || attempted)return fail("already-prepared-or-attempted");
        if(!QDir().mkpath(base))return fail("artifact-directory-failed");
        QFile::setPermissions(base,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
        const auto fixture=calibrationImage();
        if(!fixture.save(base+"/calibration.png"))return fail("fixture-save-failed");
        originDocument=p.document;originController=p.controller;originView=p.view;originId=p.id;originGeometry=g;prepared=true;
        return {{"ok",true},{"prepared",true},{"geometry",QJsonDocument::fromJson(g.toUtf8()).object()},
                {"fixtureWidth",240},{"fixtureHeight",160},{"insertionAttempted",false}};
    }
    if(!prepared || p.document!=originDocument || p.controller!=originController || p.view!=originView ||
       p.id!=originId || g!=originGeometry)return fail("stale-or-unprepared-context");
    if(op=="capture") {
        // Qt render capture is a separate gate, not rm-shot validation.
        auto image=p.window->grabWindow();
        if(image.isNull() || image.size()!=QSize(954,1696))return fail("qt-capture-unavailable");
        if(current().id!=originId || geometry(p)!=originGeometry)return fail("context-changed-during-capture");
        const auto name=attempted?"/after.png":"/before.png";
        if(!image.save(base+name))return fail("capture-save-failed");
        return {{"ok",true},{"captureBackend","QQuickWindow::grabWindow"},{"width",image.width()},{"height",image.height()},{"afterInsertionAttempt",attempted}};
    }
    if(attempted)return fail("insertion-already-attempted");
    if(diagnostic::signatures(p.controller,"insertImageFileAsSceneItem")!=QJsonArray{QStringLiteral("insertImageFileAsSceneItem(QUrl,QPointF)")})return fail("unexpected-insertion-signature");
    QImage fixture(base+"/calibration.png");if(fixture.size()!=QSize(240,160))return fail("fixture-unavailable");
    if(!isCalibrationImage(fixture))return fail("fixture-content-changed");
    const auto points=QJsonDocument::fromJson(g.toUtf8()).object().value("points").toArray();
    const auto target=points.at(3).toArray();const QPointF point(target.at(0).toDouble(),target.at(1).toDouble());
    // Consume before invocation. An uncertain reply must never trigger a retry.
    attempted=true;
    const bool invoked=QMetaObject::invokeMethod(p.controller,"insertImageFileAsSceneItem",Qt::DirectConnection,
        Q_ARG(QUrl,QUrl::fromLocalFile(base+"/calibration.png")),Q_ARG(QPointF,point));
    return {{"ok",invoked},{"insertionAttempted",true},{"verifiedPlaced",false},{"sceneX",point.x()},{"sceneY",point.y()}};
}
}
extern "C" char *ink_diary_offline(const char *value) {
    QJsonObject result=spike::fail("unsupported-runtime");auto *app=QCoreApplication::instance();
    if(diagnostic::allowed() && app) {
        const QByteArray op(value?value:"");auto job=[&]{result=spike::run(op);};
        if(QThread::currentThread()==app->thread())job();
        else if(!QMetaObject::invokeMethod(app,job,Qt::BlockingQueuedConnection))result=spike::fail("gui-unavailable");
    }
    return strdup(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());
}
