#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QQuickItem>
#include <QTest>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <cassert>
#include <iostream>
#ifdef INK_MOVE_LIVE_TEST
#define INK_DIARY_EXPERIMENTAL_MOVE
#include "admission.h"
#endif
class FakeBroker : public QObject {
    Q_OBJECT
public:
    static inline int starts=0,cancels=0;
    static inline bool busy=false;
    static inline bool armed=false;
    Q_INVOKABLE QString sendSimpleSignal(QString signal,QString command) {
        #ifdef INK_MOVE_LIVE_TEST
        assert(signal=="ink-diary-move-live$control");
#else
        assert(signal=="ink-diary-move-demo$control");
#endif
        if(command=="start") {++starts;busy=true;}
        else if(command=="cancel") {++cancels;busy=false;}
        else assert(command=="status");
        return QString::fromUtf8(QJsonDocument(QJsonObject{{"ok",true},{"canStart",armed},{"busy",busy},{"message",busy?"Synthetic busy":"Synthetic idle"}}).toJson(QJsonDocument::Compact));
    }
};
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    qmlRegisterType<FakeBroker>("net.asivery.XoviMessageBroker",2,0,"XoviMessageBroker");
    QQmlEngine engine; QQuickWindow window; window.resize(954,1696);
    QQmlComponent rootComponent(&engine);
    rootComponent.setData("import QtQuick 2.15\nItem {width:954;height:1696;property bool documentLoaded:true; "
        "property QtObject testToolbar: QtObject {property bool shown:true}; "
        "property var diaryNativePoseObjects: ({toolbar:testToolbar})}",QUrl());
    auto *root=qobject_cast<QQuickItem *>(rootComponent.create());assert(root);root->setParentItem(window.contentItem());
    QQmlComponent component(&engine,QUrl::fromLocalFile(QDir::current().absoluteFilePath(
#ifdef INK_MOVE_LIVE_TEST
        "tools/move-live/MoveLive.qml"
#else
        "tools/move-demo/MoveDemo.qml"
#endif
)));
    auto *panel=qobject_cast<QQuickItem *>(component.beginCreate(engine.rootContext())); assert(panel);
    panel->setParentItem(root);component.completeCreate();window.show();QTest::qWait(300);
    assert(FakeBroker::starts==0 && FakeBroker::cancels==0);
#ifdef INK_MOVE_LIVE_TEST
    auto clickItem=[&](QQuickItem *item) {
        QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,item->mapToScene({item->width()/2,item->height()/2}).toPoint());
        QTest::qWait(20);
    };
    auto *launcher=panel->findChild<QQuickItem *>("moveUiProbeLauncher");assert(launcher);
    auto *collapse=panel->findChild<QQuickItem *>("moveUiProbeCollapseButton");assert(collapse);
    auto *dismiss=panel->findChild<QQuickItem *>("moveUiProbeDismissButton");assert(dismiss);
    assert(panel->mapRectToScene(panel->boundingRect())==QRectF(866,1608,72,72));
    assert(launcher->isVisible() && !dismiss->isVisible());
    assert(!moveDemo::inspectOwnControl(&window,false).isEmpty()); // Never capture/start collapsed.
    clickItem(launcher);assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    assert(!dismiss->isVisible() && collapse->isVisible());
    clickItem(collapse);assert(launcher->isVisible() && !dismiss->isVisible());
    clickItem(launcher);assert(!launcher->isVisible());
    assert(FakeBroker::starts==0 && FakeBroker::cancels==0);
#endif
    assert(panel->mapRectToScene(panel->boundingRect())==QRectF(578,1504,360,176));
    auto *button=panel->findChild<QQuickItem *>("moveUiProbeButton");assert(button);
    const auto point=button->mapToScene({button->width()/2,button->height()/2}).toPoint();
    QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,point);QTest::qWait(20);
    assert(FakeBroker::starts==0 && !panel->property("demoArmed").toBool());
    FakeBroker::armed=true;QTest::qWait(300);
    QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,point);QTest::qWait(20);
    assert(FakeBroker::starts==1 && panel->property("demoBusy").toBool());
#ifdef INK_MOVE_LIVE_TEST
    assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    assert(!launcher->isVisible() && !dismiss->isVisible() && !collapse->isVisible());
#endif
    QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,point);QTest::qWait(20);
    assert(FakeBroker::cancels==1 && !panel->property("demoBusy").toBool());
#ifdef INK_MOVE_LIVE_TEST
    assert(launcher->isVisible() && !dismiss->isVisible());
#endif
    panel->setProperty("diaryCaptureSuppressed",true);assert(!panel->isVisible());
    panel->setProperty("diaryCaptureSuppressed",false);assert(panel->isVisible());
#ifdef INK_MOVE_LIVE_TEST
    auto *toolbar=root->property("testToolbar").value<QObject *>();assert(toolbar);
    FakeBroker::armed=false;QTest::qWait(300);assert(!panel->property("demoArmed").toBool());
    const int startsBefore=FakeBroker::starts,cancelsBefore=FakeBroker::cancels;
    assert(launcher->isVisible() && !dismiss->isVisible());
    toolbar->setProperty("shown",false);QTest::qWait(20);
    assert(launcher->isVisible() && !dismiss->isVisible()); // No × on collapsed launcher.
    clickItem(launcher);assert(dismiss->isVisible() && collapse->isVisible());
    assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    clickItem(dismiss);
    assert(!panel->isVisible() && panel->property("dismissedUntilToolbar").toBool());
    assert(!toolbar->property("shown").toBool());
    QTest::qWait(300);assert(!panel->isVisible());
    toolbar->setProperty("shown",true);QTest::qWait(20);
    assert(panel->isVisible() && launcher->isVisible() && !dismiss->isVisible());
    assert(!panel->property("dismissedUntilToolbar").toBool());
    toolbar->setProperty("shown",false);QTest::qWait(20);
    assert(launcher->isVisible() && !dismiss->isVisible());
    clickItem(launcher);assert(dismiss->isVisible());
    FakeBroker::busy=true;QTest::qWait(300);
    assert(panel->property("demoBusy").toBool() && !dismiss->isVisible() && !collapse->isVisible());
    assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    FakeBroker::busy=false;QTest::qWait(300);
    assert(launcher->isVisible() && !dismiss->isVisible()); // Completion collapses too.
    panel->setProperty("diaryCaptureSuppressed",true);assert(!panel->isVisible());
    toolbar->setProperty("shown",true);QTest::qWait(20);assert(!panel->isVisible());
    panel->setProperty("diaryCaptureSuppressed",false);assert(panel->isVisible());
    root->setProperty("diaryNativePoseObjects",QVariantMap{});QTest::qWait(20);
    clickItem(launcher);assert(panel->isVisible() && !dismiss->isVisible() && collapse->isVisible());
    assert(FakeBroker::starts==startsBefore && FakeBroker::cancels==cancelsBefore);
    std::cout<<"PASS: compact idle launcher, explicit expand/collapse, exact expanded capture gate, busy Cancel, toolbar-aware × and hidden recovery, suppression precedence, unknown-state fallback\n";
#endif
    root->setProperty("documentLoaded",false);assert(!panel->isVisible());
    delete panel;delete root;
}
#include "test-ui.moc"
