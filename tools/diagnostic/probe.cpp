// Diagnostic-only: no capture, network, page identifiers, content, or writes.
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QFile>
#include <QSet>
#include <QThread>
#include <cstring>
#include <dlfcn.h>
#include "policy.h"
namespace diagnostic {
QByteArray read(const char *path) {QFile file(QString::fromLatin1(path));return file.open(QIODevice::ReadOnly)?file.readAll():QByteArray();}
bool allowed() {
    // XOVI LD_PRELOAD is inherited by application helpers. The model/Qt tuple
    // alone also matches those processes; mirror the broker's process gate.
    const bool gui=dlsym(RTLD_DEFAULT,"_Z21qRegisterResourceDataiPKhS0_S0_")!=nullptr;
    const bool deviceLock=dlsym(RTLD_DEFAULT,"_ZNK3csl10devicelock10DeviceLock3setENS1_5StateE")!=nullptr;
    if(!gui || !deviceLock)return false;
    return permitted(read("/sys/devices/soc0/machine"),read("/etc/os-release"),qVersion(),gui,deviceLock);
}
QJsonArray signatures(QObject *object,const QByteArray &name) {
    QJsonArray out;
    if(object)for(int i=0;i<object->metaObject()->methodCount();++i) {
        auto method=object->metaObject()->method(i);
        if(method.name()==name)out.append(QString::fromLatin1(method.methodSignature()));
    }
    return out;
}
QJsonObject collect() {
    QJsonArray windows,candidates;
    QList<QObject *> pending;QSet<QObject *> seen;
    for(auto *window:QGuiApplication::allWindows()) {
        if(!window || !window->isVisible())continue;
        windows.append(QJsonObject{{"width",window->width()},{"height",window->height()},{"dpr",window->devicePixelRatio()}});
        pending.append(window);
        if(auto *quick=qobject_cast<QQuickWindow *>(window))pending.append(quick->contentItem());
    }
    bool documentLoaded=false;
    while(!pending.isEmpty() && seen.size()<100000) {
        auto *object=pending.takeLast();if(!object || seen.contains(object))continue;
        seen.insert(object);pending.append(object->children());
        auto *item=qobject_cast<QQuickItem *>(object);
        if(item)for(auto *child:item->childItems())pending.append(child);
        if(!item || !item->isVisible())continue;
        // Read only whitelisted booleans/object references. Never page IDs,
        // text properties, paths, document bounds, object names or all properties.
        if(object->property("documentLoaded").toBool()) {
            documentLoaded=true;
            auto *controller=object->property("sceneController").value<QObject *>();
            if(controller)candidates.append(QJsonObject{{"kind","document-controller"},
                {"insertImageFileAsSceneItem",signatures(controller,"insertImageFileAsSceneItem")}});
        }
        auto mapping=signatures(object,"viewToScene");
        if(mapping.isEmpty())continue;
        QJsonObject candidate{{"kind","visible-view"},{"width",item->width()},{"height",item->height()},
            {"viewToScene",mapping},{"mappingFinite",false}};
        auto *controller=object->property("controller").value<QObject *>();
        candidate.insert("insertImageFileAsSceneItem",signatures(controller,"insertImageFileAsSceneItem"));
        // Read-only affine mapping sanity check. Return only a boolean, never
        // scene/page coordinates or identifiers. No notebook QML import needed.
        if(auto *engine=qmlEngine(object)) {
            QQmlComponent component(engine);
            component.setData(R"(import QtQml
QtObject { function probe(view) {
 try { const points=[[0,0],[100,0],[0,100]]; let out=[];
 for(let i=0;i<3;++i) { const p=view.viewToScene(view.mapFromItem(null,points[i][0],points[i][1]));
 if(!p || !isFinite(p.x) || !isFinite(p.y))return false; out.push(p); }
 const determinant=(out[1].x-out[0].x)*(out[2].y-out[0].y)-(out[1].y-out[0].y)*(out[2].x-out[0].x);
 return isFinite(determinant) && Math.abs(determinant)>0.000001;
 } catch(e) { return false; } }
})",QUrl("diagnostic:readonly"));
            if(auto *helper=component.create()) {
                QVariant result;
                if(QMetaObject::invokeMethod(helper,"probe",Q_RETURN_ARG(QVariant,result),Q_ARG(QVariant,QVariant::fromValue(object))))
                    candidate.insert("mappingFinite",result.toBool());
                delete helper;
            }
        }
        candidates.append(candidate);
    }
    return {{"ok",true},{"diagnosticOnly",true},{"model","chiappa"},{"release","3.28.0.172"},
        {"qt",QString::fromLatin1(qVersion())},{"windows",windows},{"documentLoaded",documentLoaded},
        {"candidates",candidates},{"scanTruncated",!pending.isEmpty()}};
}
}
extern "C" char _xovi_shouldLoad() {return diagnostic::allowed();}
extern "C" char *ink_diary_probe(const char *) {
    QJsonObject result{{"ok",false},{"error","unsupported-runtime"}};
    auto *app=QCoreApplication::instance();
    if(diagnostic::allowed() && app) {
        auto job=[&]{result=diagnostic::collect();};
        if(QThread::currentThread()==app->thread())job();
        else if(!QMetaObject::invokeMethod(app,job,Qt::BlockingQueuedConnection))
            result={{"ok",false},{"error","gui-unavailable"}};
    }
    return strdup(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());
}
