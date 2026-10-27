#include "probe.cpp"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QTest>
#include <cassert>
#include <iostream>
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv); QQmlEngine engine;
    assert(request("begin").value("error")=="visible-probe-required");
    assert(request("invalid").value("ok")==false);
    QQuickWindow window; window.resize(954,1696);
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.15\nItem { width:954; height:1696; Rectangle { objectName: \"moveUiProbe\"; readonly property string diaryCaptureOwner: \"ink-diary-owned-control-v1\"; property bool diaryCaptureSuppressed:false; visible:!diaryCaptureSuppressed; width:100; height:100 } }",QUrl());
    auto *root=qobject_cast<QQuickItem *>(component.create()); assert(root);
    root->setParentItem(window.contentItem()); window.show();
    assert(QTest::qWaitFor([&]{return window.isExposed();}));
    auto *control=root->childItems().first();
    assert(request("begin").value("ok")==true);
    assert(request("begin").value("error")=="lease-already-held");
    assert(QTest::qWaitFor([&]{return request("status").value("ready").toBool();}));
    assert(!control->isVisible());
    request("release"); assert(control->isVisible() && !lease);
    request("begin"); request("release");
    QTest::qWait(50); assert(control->isVisible() && phase=="released");
    request("begin");
    assert(QTest::qWaitFor([&]{return phase=="expired";},16000));
    assert(control->isVisible() && !lease);
    request("release"); request("release"); delete root;
    std::cout << "PASS: explicit-only lease protocol, missing/invalid/duplicate rejection, render ready, release/cancel, 15s automatic restoration\n";
}
