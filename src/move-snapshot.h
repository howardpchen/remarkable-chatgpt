#pragma once
#include "readonly-template.h"
#include "readonly-mapping.h"
#include <QSet>
#include <QMetaMethod>

// Fixed, read-only metadata only. Association is pointer-local on the GUI thread;
// no pointer, template filename or page index is serialized. Ambiguity rejects.
inline QJsonObject diaryVerifiedMoveSnapshot(QQmlEngine *engine,
        const QList<QObject *> &objects, QObject *document, QObject *controller) {
    if (!engine || !document || !controller) return {};
    QSet<QObject *> documents;
    QSet<QQuickItem *> views;
    for (QObject *object : objects) {
        if (!object) continue;
        auto *item=qobject_cast<QQuickItem *>(object);
        if (item && !item->isVisible()) continue;
        if (object->property("documentLoaded").toBool() &&
            object->property("sceneController").value<QObject *>()) documents.insert(object);
        if (!item || object->property("controller").value<QObject *>() != controller) continue;
        for (int i=0;i<object->metaObject()->methodCount();++i)
            if (object->metaObject()->method(i).name()=="viewToScene") { views.insert(item);break; }
    }
    if (documents.size()!=1 || !documents.contains(document) || views.size()!=1 ||
        document->property("sceneController").value<QObject *>()!=controller ||
        diaryMetadata::templateStatus(engine,document)!=QLatin1String("blank")) return {};
    auto result=diaryMetadata::viewportSnapshot(engine,*views.begin());
    if (result.value("width").toDouble()!=954 || result.value("height").toDouble()!=1696 ||
        result.value("windowWidth").toDouble()!=954 || result.value("windowHeight").toDouble()!=1696 ||
        result.value("devicePixelRatio").toDouble()!=1) return {};
    result.insert("templateName",QStringLiteral("Blank"));
    return result;
}
inline bool diaryViewportFresh(const QJsonObject &origin,const QJsonObject &now) {
    return !origin.isEmpty() && origin==now;
}
