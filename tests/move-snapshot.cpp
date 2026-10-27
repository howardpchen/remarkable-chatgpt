#include "../src/move-snapshot.h"
#include "../src/capture-layout.h"
#include <QGuiApplication>
#include <cassert>
#include <iostream>
class Document : public QObject {
    Q_OBJECT
public:
    QString name="Blank";
    Q_INVOKABLE QString templateForPage(int page) { return page==0 ? name : QString(); }
};
class Root : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(bool documentLoaded MEMBER loaded)
    Q_PROPERTY(QObject* document MEMBER document)
    Q_PROPERTY(QObject* sceneController MEMBER controller)
    Q_PROPERTY(int currentPage MEMBER page)
public:
    bool loaded=true; QObject *document=nullptr,*controller=nullptr; int page=0;
};
class View : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QObject* controller MEMBER controller)
public:
    QObject *controller=nullptr; qreal scale=0.8598484848484848,pan=-410.1477272727273;
    Q_INVOKABLE QVariant viewToScene(QVariant input) {
        const auto p=input.toPointF(); return QPointF(p.x()*scale+pan,p.y()*scale);
    }
};
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);QQmlEngine engine;QQuickWindow window;
    window.resize(954,1696);window.show();
    Document document;QObject controller;Root root;View view;
    root.document=&document;root.controller=&controller;view.controller=&controller;
    view.setParentItem(window.contentItem());view.setSize({954,1696});
    QList<QObject*> objects{&root,&view};
    const auto rootOwner=QQmlEngine::objectOwnership(&root),viewOwner=QQmlEngine::objectOwnership(&view);
    const auto documentOwner=QQmlEngine::objectOwnership(&document),controllerOwner=QQmlEngine::objectOwnership(&controller);
    const auto capture=[&] {return diaryVerifiedMoveSnapshot(&engine,objects,&root,&controller);};
    const auto origin=capture();assert(!origin.isEmpty());
    assert(origin.value("templateName")=="Blank");
    assert(diaryViewportFresh(origin,capture()));
    assert(diaryExperimentalMoveMapping(origin,origin).sx>0);
    // A pending request must cancel on template changes/unavailability even when
    // geometry is identical; exact geometry restoration alone is insufficient.
    bool cancelled=false;
    auto guard=[&] {if(!diaryViewportFresh(origin,capture()))cancelled=true;return !cancelled;};
    document.name="Dots";assert(!guard());assert(capture().isEmpty());
    document.name="Blank";assert(!guard()); // sticky request cancellation
    assert(diaryViewportFresh(origin,capture()));
    root.loaded=false;assert(capture().isEmpty());root.loaded=true;
    root.document=nullptr;assert(capture().isEmpty());root.document=&document;
    cancelled=false;document.name="";assert(!guard());assert(capture().isEmpty());
    document.name="Blank";assert(!guard());
    cancelled=false;view.scale*=1.1;assert(!guard());assert(!diaryViewportFresh(origin,capture()));
    view.scale/=1.1; // use exact original below, do not assume FP roundtrip
    view.scale=0.8598484848484848;assert(diaryViewportFresh(origin,capture()));
    assert(!guard()); // restoration permits a NEW request, never resumes old work
    view.pan+=1;assert(!diaryViewportFresh(origin,capture()));view.pan-=1;
    assert(diaryViewportFresh(origin,capture()));
    Root other;other.document=&document;other.controller=&controller;
    objects.append(&other);assert(capture().isEmpty());objects.removeLast();
    View duplicate;duplicate.controller=&controller;duplicate.setParentItem(window.contentItem());
    duplicate.setSize({954,1696});objects.append(&duplicate);assert(capture().isEmpty());objects.removeLast();
    root.controller=&document;assert(capture().isEmpty());root.controller=&controller;
    view.setVisible(false);assert(capture().isEmpty());view.setVisible(true);
    window.resize(955,1696);assert(capture().isEmpty());window.resize(954,1696);
    assert(diaryViewportFresh(origin,capture()));
    engine.collectGarbage();
    assert(QQmlEngine::objectOwnership(&root)==rootOwner);
    assert(QQmlEngine::objectOwnership(&view)==viewOwner);
    assert(QQmlEngine::objectOwnership(&document)==documentOwner);
    assert(QQmlEngine::objectOwnership(&controller)==controllerOwner);
    assert(!diaryExperimentalMoveDemoAllowed(true,false));
    assert(diaryExperimentalMoveDemoAllowed(true,true));
    assert(diaryExperimentalMoveDemoAllowed(false,false));
    std::cout<<"PASS: shared runtime snapshot, Blank transitions, sticky pending cancellation, exact restoration, association ambiguity, ownership, Move demo-only policy\n";
}
#include "move-snapshot.moc"
