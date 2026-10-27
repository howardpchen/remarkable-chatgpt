// Diagnostic-only: no capture, network, page identifiers, notebook content, or writes.
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QFile>
#include <QCryptographicHash>
#include <QSet>
#include <QHash>
#include <QThread>
#include <cstring>
#include <dlfcn.h>
#include "policy.h"
#include "readonly-mapping.h"
#include "readonly-template.h"
#include "native-pose.h"
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
QJsonObject documentViewResource(int chunk=-1) {
    // Fixed application resource, never a caller-supplied path or notebook file.
    const QString path=QStringLiteral(":/qml/device/view/documentview/DocumentView.qml");
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return {{"ok",false},{"error","document-view-resource-unavailable"}};
    constexpr qint64 limit=1024*1024, chunkSize=64*1024;
    const auto bytes=file.read(limit+1);
    if(bytes.isEmpty() || bytes.size()>limit || !file.atEnd() || file.error()!=QFileDevice::NoError)
        return {{"ok",false},{"error","document-view-resource-size-or-read-error"},
            {"observedBytes",bytes.size()},{"fileSize",file.size()},
            {"atEnd",file.atEnd()},{"readError",int(file.error())}};
    if(chunk>=0 && qint64(chunk)*chunkSize>=bytes.size())
        return {{"ok",false},{"error","resource-chunk-out-of-range"}};
    QJsonObject result{{"ok",true},{"diagnosticOnly",true},{"resourcePath",path},
        {"size",bytes.size()},
        {"chunks",int((bytes.size()+chunkSize-1)/chunkSize)},
        {"sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())}};
    if(chunk>=0) {
        result.insert("chunk",chunk);
        result.insert("base64",QString::fromLatin1(bytes.mid(qint64(chunk)*chunkSize,chunkSize).toBase64()));
    }
    return result;
}
QJsonObject collect(bool pose=false) {
    QJsonArray windows,candidates;
    // Response-local association only: no pointer, page ID or durable identifier
    // is exposed. Never compare these ordinals between separate probe calls.
    QHash<QObject *,int> controllers;
    auto controllerOrdinal=[&](QObject *controller) {
        if(!controller)return 0;
        if(!controllers.contains(controller))controllers.insert(controller,controllers.size()+1);
        return controllers.value(controller);
    };
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
            if(pose)candidates.append(QJsonObject{{"kind","native-pose"},{"pose",nativePose(item)}});
            if(controller)candidates.append(QJsonObject{{"kind","document-controller"},
                {"controllerOrdinal",controllerOrdinal(controller)},
                {"templateStatus",
#ifdef INK_DIAGNOSTIC_FERRARI
                    QStringLiteral("unvalidated-ferrari-resource")
#else
                    templateStatus(qmlEngine(object),object)
#endif
                },
                {"insertImageFileAsSceneItem",signatures(controller,"insertImageFileAsSceneItem")}});
        }
        auto mapping=signatures(object,"viewToScene");
        if(mapping.isEmpty())continue;
        QJsonObject candidate{{"kind","visible-view"},{"width",item->width()},{"height",item->height()},
            {"viewToScene",mapping},{"mappingFinite",false}};
        auto *controller=object->property("controller").value<QObject *>();
        candidate.insert("controllerOrdinal",controllerOrdinal(controller));
        candidate.insert("insertImageFileAsSceneItem",signatures(controller,"insertImageFileAsSceneItem"));
        // No component imports: execution is synchronous on the existing UI engine.
        const auto status=mappingStatus(qmlEngine(object),object);
        candidate.insert("mappingStatus",status);
        if(status==QStringLiteral("valid"))candidate.insert("mappingFinite",true);
        else if(status==QStringLiteral("non-finite") || status==QStringLiteral("degenerate"))
            candidate.insert("mappingFinite",false);
        else candidate.insert("mappingFinite",QJsonValue(QJsonValue::Null));
        const auto viewport=viewportSnapshot(qmlEngine(object),item);
        if(!viewport.isEmpty())candidate.insert("viewport",viewport);
        candidates.append(candidate);
    }
    return {{"ok",true},{"diagnosticOnly",true},{"model",model},{"release",release},
        {"qt",QString::fromLatin1(qVersion())},{"windows",windows},{"documentLoaded",documentLoaded},
        {"candidates",candidates},{"scanTruncated",!pending.isEmpty()}};
}
}
extern "C" char _xovi_shouldLoad() {return diagnostic::allowed();}
extern "C" char *ink_diary_probe(const char *argument) {
    QJsonObject result{{"ok",false},{"error","unsupported-runtime"}};
    auto *app=QCoreApplication::instance();
    if(diagnostic::allowed() && app) {
        const QByteArray request=argument ? QByteArray(argument) : QByteArray();
        auto job=[&]{
            if(request.isEmpty())result=diagnostic::collect();
            else if(request=="native-pose") {
#ifdef INK_DIAGNOSTIC_FERRARI
                result={{"ok",false},{"error","native-pose-unvalidated-for-ferrari"}};
#else
                result=diagnostic::collect(true);
#endif
            }
            else if(request=="document-view-resource")result=diagnostic::documentViewResource();
            else {
                result={{"ok",false},{"error","unsupported-diagnostic-request"}};
                for(int chunk=0;chunk<16;++chunk)
                    if(request==QByteArray("document-view-resource-chunk-")+QByteArray::number(chunk))
                        result=diagnostic::documentViewResource(chunk);
            }
        };
        if(QThread::currentThread()==app->thread())job();
        else if(!QMetaObject::invokeMethod(app,job,Qt::BlockingQueuedConnection))
            result={{"ok",false},{"error","gui-unavailable"}};
    }
    return strdup(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());
}
