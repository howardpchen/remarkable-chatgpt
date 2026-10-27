#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QQuickItem>
#include <QTest>
#include <QDir>
#include <cassert>
#include <iostream>
int main(int argc,char**argv){
 QGuiApplication app(argc,argv); QQmlEngine engine;
 QQmlComponent rootComponent(&engine);rootComponent.setData("import QtQuick 2.15\nItem { property bool documentLoaded: true; width:954; height:1696 }",QUrl());
 auto *root=qobject_cast<QQuickItem*>(rootComponent.create());
 if(!root){std::cerr<<rootComponent.errorString().toStdString();return 1;}
 QQuickWindow window;window.resize(954,1696);root->setParentItem(window.contentItem());
 QQmlComponent specimen(&engine,QUrl::fromLocalFile(QDir::current().absoluteFilePath("tools/move-ui-probe/MoveUiProbe.qml")));
 auto *overlay=qobject_cast<QQuickItem*>(specimen.beginCreate(engine.rootContext()));
 if(!overlay){std::cerr<<specimen.errorString().toStdString();delete root;return 1;}
 overlay->setParentItem(root);specimen.completeCreate();window.show();QTest::qWait(100);
 assert(overlay->isVisible()&&overlay->width()==360&&overlay->height()==176);
 assert(overlay->x()==578&&overlay->y()==1504);
 assert(overlay->property("clicks").toInt()==0&&!overlay->property("simulatedBusy").toBool());
 auto *button=overlay->findChild<QQuickItem*>("moveUiProbeButton");assert(button&&button->width()==336);
 auto click=[&]{auto point=button->mapToScene({button->width()/2,button->height()/2});QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,point.toPoint());QTest::qWait(20);};
 click();assert(overlay->property("simulatedBusy").toBool()&&overlay->property("clicks").toInt()==1);
 click();assert(!overlay->property("simulatedBusy").toBool()&&overlay->property("clicks").toInt()==2);
 root->setProperty("documentLoaded",false);QTest::qWait(20);assert(!overlay->isVisible());
 root->setProperty("documentLoaded",true);QTest::qWait(20);assert(overlay->isVisible());
 root->setWidth(320);QTest::qWait(20);assert(overlay->width()==288&&button->width()==264&&overlay->isVisible());
 root->setWidth(319);QTest::qWait(20);assert(!overlay->isVisible());
 root->setWidth(954);root->setHeight(351);QTest::qWait(20);assert(!overlay->isVisible());
 delete overlay;delete root;app.processEvents();
 std::cout<<"PASS: real QML creation, no on-load action, pointer Start/Cancel, document visibility, narrow bounds, teardown\n";
}
