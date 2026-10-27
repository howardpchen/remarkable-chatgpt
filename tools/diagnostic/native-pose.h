#pragma once
// Fixed, bounded, read-only UI telemetry. Never reads text/notebook contents.
#include <QQmlExpression>
#include <QQmlEngine>
#include <QJSValue>
#include <QGuiApplication>
#include <cmath>
#include <QQmlContext>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSet>
namespace diagnostic {
inline QObject *poseObject(QQuickItem *document, const QString &id) {
    // AOT-compiled DocumentView hides many lexical IDs from QQmlExpression.
    // The exact-resource own QMD exposes a fixed read-only map in its ORIGINAL
    // lexical context; absent entries still fail closed (never scan by guesses).
    const auto bound=document->property("diaryNativePoseObjects");
    if (bound.metaType().id()==qMetaTypeId<QJSValue>()) {
        const auto value=bound.value<QJSValue>().property(id);
        if (auto *object=value.toQObject()) return object;
    }
    if (bound.canConvert<QVariantMap>()) {
        const auto value=bound.toMap().value(id);
        if (auto *object=value.value<QObject *>()) return object;
    }
    auto *context=QQmlEngine::contextForObject(document);
    if (!context) return nullptr;
    // id comes ONLY from the fixed list below, never caller input.
    QQmlExpression expression(context,document,id);
    const auto value=expression.evaluate();
    if (expression.hasError()) return nullptr;
    if (value.canConvert<QObject *>()) return value.value<QObject *>();
    return value.value<QJSValue>().toQObject();
}
inline QJsonObject nativePose(QQuickItem *document, int limit=20000) {
    if (!document || !document->window() || !QQmlEngine::contextForObject(document))
        return {{"ok",false},{"error","document-qml-context-unavailable"}};
    const QStringList ids={"sceneView","toolbar","_uiContainer","pages","pageSlider",
        "pageLabel","titleLabel","newPageButton","_toolbarTooltipOverlay","selectionHelper",
        "formatSettings","formatMenuDelegate","addTagPopup","_searchResultsHeader",
        "searchResultsProgress","notificationBar","hwcDialog","adjustViewPopup","_popupOverlay",
        "zoomButton","scrollLockVisualCue","writingFormatsToolTip","connectivityDialogContainerItem",
        "_tocDeleteLoader","_tocRenamePopup","_tocCrossDocErrorLoader","_tocCrossDocDownloadLoader"};
    QJsonArray missing; QJsonObject states; QObject *scene=nullptr;
    for (const auto &id:ids) {
        auto *object=poseObject(document,id);
        if (!object) {missing.append(id); continue;}
        if (id=="sceneView") scene=object;
        QJsonObject state;
        for (const auto &name:{"visible","expanded","shown","position","innerWidth","innerHeight","active","processing"}) {
            const auto v=object->property(name);
            if (v.metaType().id()==QMetaType::Bool) state.insert(name,v.toBool());
            else if (v.isValid() && v.canConvert<double>() && std::isfinite(v.toDouble())) state.insert(name,v.toDouble());
        }
        states.insert(id,state);
    }
    QJsonObject documentState;
    for (const auto &name:{"subdialogOpen","fullscreenSubDialogOpen","isLoading","isWorking","backgroundFilterVisible","notePage","documentLoaded"}) {
        auto value=document->property(name);
        if (value.metaType().id()==QMetaType::Bool) documentState.insert(name,value.toBool());
    }
    QJsonArray items; QList<QQuickItem *> pending{document->window()->contentItem()}; QSet<QQuickItem *> seen;
    while (!pending.isEmpty() && seen.size()<limit) {
        auto *item=pending.takeLast();
        if (!item || seen.contains(item)) continue;
        seen.insert(item);
        if (item==scene || (item->objectName()=="moveUiProbe" &&
            item->property("diaryCaptureOwner").toString()=="ink-diary-owned-control-v1")) continue;
        for (auto *child:item->childItems()) pending.append(child);
        if (!item->isVisible() || item->opacity()==0) continue;
        const auto rect=item->mapRectToScene(item->boundingRect());
        if (!std::isfinite(rect.x()) || !std::isfinite(rect.y()) || !std::isfinite(rect.width()) || !std::isfinite(rect.height()))
            return {{"ok",false},{"error","nonfinite-ui-geometry"}};
        QString type=QString::fromLatin1(item->metaObject()->className());
        type.remove(QRegularExpression("_QML(TYPE)?_[0-9]+"));
        items.append(QJsonObject{{"type",type},{"rect",QJsonArray{rect.x(),rect.y(),rect.width(),rect.height()}},
            {"opacity",item->opacity()},{"rotation",item->rotation()},{"scale",item->scale()}});
    }
    int windows=0;
    for (auto *window:QGuiApplication::allWindows()) if (window->isVisible()) ++windows;
    return {{"ok",missing.isEmpty() && pending.isEmpty() && windows==1},{"missing",missing},
        {"truncated",!pending.isEmpty()},{"visibleWindows",windows},{"documentState",documentState},{"states",states},{"items",items}};
}
}
