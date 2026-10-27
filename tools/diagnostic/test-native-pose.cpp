#include "native-pose.h"
#include <QQmlComponent>
#include <cassert>
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv); QQmlEngine engine; QQuickWindow window;
    window.resize(954,1696); window.show();
    QQuickItem orphan; assert(!diagnostic::nativePose(&orphan)["ok"].toBool());
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.15\nItem { id: root; width:954; height:1696; readonly property var diaryNativePoseObjects: ({knownBound: toolbar}); Item { id: toolbar; width:100; height:400 } Item { id: sceneView; Rectangle { color:'black'; width:50; height:50 } } }",QUrl());
    auto *doc=qobject_cast<QQuickItem *>(component.create()); assert(doc);
    doc->setParentItem(window.contentItem());
    assert(diagnostic::poseObject(doc,"toolbar"));
    assert(diagnostic::poseObject(doc,"knownBound")==diagnostic::poseObject(doc,"toolbar"));
    auto first=diagnostic::nativePose(doc); assert(!first["ok"].toBool());
    assert(!first["missing"].toArray().isEmpty());
    QObject knownInactive;
    for (const auto &id:first["missing"].toArray()) engine.rootContext()->setContextProperty(id.toString(),&knownInactive);
    assert(diagnostic::nativePose(doc)["ok"].toBool());
    assert(!first["truncated"].toBool());
    assert(diagnostic::nativePose(doc,1)["truncated"].toBool());
    auto *scene=qobject_cast<QQuickItem *>(diagnostic::poseObject(doc,"sceneView"));
    const auto poseBaseline=diagnostic::nativePose(doc);
    const auto baseline=poseBaseline.value("items"); scene->childItems().first()->setWidth(700);
    assert(diagnostic::nativePose(doc)["items"]==baseline);
    auto *toolbar=qobject_cast<QQuickItem *>(diagnostic::poseObject(doc,"toolbar"));
    toolbar->setVisible(false); assert(diagnostic::nativePose(doc)["items"]!=baseline);
    assert(!diagnostic::poseObject(doc,"missingFixedId"));
    QQuickWindow extra; extra.show(); assert(diagnostic::nativePose(doc)["visibleWindows"].toInt()==2);
    delete doc;
}
