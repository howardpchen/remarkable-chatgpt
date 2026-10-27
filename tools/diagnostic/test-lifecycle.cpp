// Host-only lifecycle coverage; does not simulate XOVI unload or diagnose a device crash.
#include "readonly-mapping.h"
#include "readonly-template.h"
#include <QGuiApplication>
#include <QQuickItem>
#include <QPointer>
#include <QEvent>
#include <cassert>
#include <iostream>

class LifecycleDocument : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QString templateForPage(int page) { assert(page==0); return "Blank"; }
};
class LifecycleView : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(bool documentLoaded READ loaded)
    Q_PROPERTY(QObject* document READ document)
    Q_PROPERTY(int currentPage READ page)
public:
    LifecycleDocument *doc=nullptr;
    bool loaded() const { return true; }
    QObject *document() const { return doc; }
    int page() const { return 0; }
    Q_INVOKABLE QVariant viewToScene(QVariant input) { return input.toPointF(); }
};

static void checkLifecycle(QQmlEngine::ObjectOwnership owner, bool parented, bool engineFirst) {
    auto *engine=new QQmlEngine;
    QObject parent;
    auto *view=new LifecycleView;
    if(parented)view->setParent(&parent);
    view->doc=new LifecycleDocument;
    view->doc->setParent(view);
    const QPointer<LifecycleView> watchedView=view;
    const QPointer<LifecycleDocument> watchedDoc=view->doc;
    int destroyedViews=0,destroyedDocs=0;
    QObject::connect(view,&QObject::destroyed,[&]{++destroyedViews;});
    QObject::connect(view->doc,&QObject::destroyed,[&]{++destroyedDocs;});
    QQmlEngine::setObjectOwnership(view,owner);
    QQmlEngine::setObjectOwnership(view->doc,owner);
    // Keep a caller-owned wrapper live: helper scope must not alter that lifetime.
    auto retained=engine->newQObject(view);
    QQmlEngine::setObjectOwnership(view,owner);
    for(int i=0;i<32;++i) {
        assert(diagnostic::mappingStatus(engine,view)=="valid");
        assert(diagnostic::templateStatus(engine,view)=="blank");
        assert(QQmlEngine::objectOwnership(view)==owner);
        assert(QQmlEngine::objectOwnership(view->doc)==owner);
        engine->collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        assert(watchedView && watchedDoc);
        assert(destroyedViews==0 && destroyedDocs==0);
    }
    if(engineFirst) {
        retained=QJSValue();
        delete engine;
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        // QObject-parented or C++-owned instances must outlive the engine.
        if(parented || owner==QQmlEngine::CppOwnership)assert(watchedView && watchedDoc);
        if(watchedView)delete watchedView.data();
    } else {
        delete view;
        assert(!watchedView && !watchedDoc);
        // Existing JS wrappers lose their QObject target after native deletion.
        assert(retained.toQObject()==nullptr);
        engine->collectGarbage();
        retained=QJSValue();
        delete engine;
    }
    QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
    assert(!watchedView && !watchedDoc);
    assert(destroyedViews==1 && destroyedDocs==1);
}

int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    for(auto owner:{QQmlEngine::CppOwnership,QQmlEngine::JavaScriptOwnership})
        for(bool parented:{false,true})
            for(bool engineFirst:{false,true})checkLifecycle(owner,parented,engineFirst);
    // Qt drops context-bound queued callbacks when their recipient is destroyed.
    // This is the mechanism relied on by invokeMethod(app,...), not a shutdown
    // admission guard: aboutToQuit happens while app still exists.
    bool dispatched=false;
    auto *context=new QObject;
    assert(QMetaObject::invokeMethod(context,[&]{dispatched=true;},Qt::QueuedConnection));
    delete context;
    QCoreApplication::sendPostedEvents();
    assert(!dispatched);
    int quitCalls=0;
    QObject::connect(&app,&QCoreApplication::aboutToQuit,[&]{
        ++quitCalls;
        assert(!QCoreApplication::closingDown());
        assert(QCoreApplication::instance()==&app);
    });
    QMetaObject::invokeMethod(&app,&QCoreApplication::quit,Qt::QueuedConnection);
    assert(app.exec()==0);
    assert(quitCalls==1);
    std::cout<<"Diagnostic lifecycle checks passed: 8 ownership/destruction cases, 512 helper calls, queued-context destruction and aboutToQuit boundary\n";
}
#include "test-lifecycle.moc"
