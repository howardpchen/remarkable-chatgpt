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
#include "../../src/capture-ui.h"
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
    rootComponent.setData("import QtQuick 2.15\nItem {id:root; width:954;height:1696;property bool documentLoaded:true; "
        "property bool toolbarVisibleSetting:true; property alias testToolbar:bar; "
        "Item {id:bar; visible:root.toolbarVisibleSetting; width:112;height:1696; property bool expanded:true; readonly property bool shown:visible && expanded; "
        "Rectangle {objectName:'fakeNativeClose'; x:842; width:112;height:112; color:'black'; visible:bar.expanded}} "
        "property var diaryNativePoseObjects: ({toolbar:bar})}",QUrl());
    auto *root=qobject_cast<QQuickItem *>(rootComponent.create());assert(root);root->setParentItem(window.contentItem());
    QQmlComponent component(&engine,QUrl::fromLocalFile(QDir::current().absoluteFilePath(
#if defined(INK_MOVE_PER_REQUEST)
        "tools/move-session/MoveLive.qml"
#elif defined(INK_MOVE_LIVE_TEST)
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
#ifdef INK_MOVE_PER_REQUEST
    auto *statusText=panel->findChild<QQuickItem *>("moveUiProbeStatus");assert(statusText);
    // Exercise the actual narrow status region with and without dismiss control.
    for(bool hidden:{false,true}) {
        root->setProperty("toolbarVisibleSetting",!hidden);
        for(const auto notice:{"Needs web access; search is off.","Cannot read question. Please rewrite.","Text answers only; try another question."}) {
            panel->setProperty("localMessage",notice);QCoreApplication::processEvents();
            assert(statusText->property("text").toString()==notice);
            assert(statusText->property("contentHeight").toReal()<=statusText->height());
        }
    }
    panel->setProperty("tokenBudget",true);
    panel->setProperty("budgetAvailable",50000000.0);
    panel->setProperty("localMessage","Normal zoom: Blank, Dots small or Grid small");
    QCoreApplication::processEvents();
    assert(statusText->property("text").toString()=="AI · 50M credits");
    assert(statusText->property("contentHeight").toReal()<=statusText->height());
    panel->setProperty("localMessage","Live text inserted (single capture); verify placement and Undo/Redo");
    QCoreApplication::processEvents();assert(statusText->property("text").toString()=="Done · 50M credits");
    panel->setProperty("tokenBudget",false);
    root->setProperty("toolbarVisibleSetting",true);
#endif

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
#ifdef INK_MOVE_PER_REQUEST
    assert(!launcher->isVisible() && panel->findChild<QQuickItem *>("moveUiProbeStatus")->isVisible());
    clickItem(collapse); // Result/error remains visible until explicitly collapsed.
#else
    assert(launcher->isVisible() && !dismiss->isVisible());
#endif
#endif
    panel->setProperty("diaryCaptureSuppressed",true);assert(!panel->isVisible());
    panel->setProperty("diaryCaptureSuppressed",false);assert(panel->isVisible());
#ifdef INK_MOVE_LIVE_TEST
    auto *toolbar=root->property("testToolbar").value<QObject *>();assert(toolbar);
    FakeBroker::armed=false;QTest::qWait(300);assert(!panel->property("demoArmed").toBool());
    const int startsBefore=FakeBroker::starts,cancelsBefore=FakeBroker::cancels;
    assert(launcher->isVisible() && !dismiss->isVisible());
    toolbar->setProperty("expanded",false);QTest::qWait(20);
    assert(launcher->isVisible() && !dismiss->isVisible()); // No × on collapsed launcher.
    clickItem(launcher);assert(dismiss->isVisible() && collapse->isVisible());
    assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    clickItem(dismiss);
    assert(!panel->isVisible() && panel->property("dismissedUntilToolbar").toBool());
    assert(!toolbar->property("shown").toBool());
    QTest::qWait(300);assert(!panel->isVisible());
    toolbar->setProperty("expanded",true);QTest::qWait(20);
    assert(panel->isVisible() && launcher->isVisible() && !dismiss->isVisible());
    assert(!panel->property("dismissedUntilToolbar").toBool());
    toolbar->setProperty("expanded",false);QTest::qWait(20);
    assert(launcher->isVisible() && !dismiss->isVisible());
    clickItem(launcher);assert(dismiss->isVisible());
    FakeBroker::busy=true;QTest::qWait(300);
    assert(panel->property("demoBusy").toBool() && !dismiss->isVisible() && !collapse->isVisible());
    assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    FakeBroker::busy=false;QTest::qWait(300);
#ifdef INK_MOVE_PER_REQUEST
    assert(!launcher->isVisible() && panel->findChild<QQuickItem *>("moveUiProbeStatus")->isVisible());
    clickItem(collapse);
#else
    assert(launcher->isVisible() && !dismiss->isVisible()); // Completion collapses too.
#endif
    panel->setProperty("diaryCaptureSuppressed",true);assert(!panel->isVisible());
    toolbar->setProperty("expanded",true);QTest::qWait(20);assert(!panel->isVisible());
    panel->setProperty("diaryCaptureSuppressed",false);assert(panel->isVisible());
    // Real capture lease + actual live QML: shown, collapsed and hidden states
    // are restored, including the original visibility binding after changes.
    for (bool originalVisible:{false,true}) for (bool originalExpanded:{false,true}) {
        root->setProperty("toolbarVisibleSetting",originalVisible);
        toolbar->setProperty("expanded",originalExpanded);
        int renders=0,failures=0;
        DiaryCaptureUiSession lease(&window,[&](auto event,const auto &){
            event==DiaryCaptureUiSession::Event::RenderReady?++renders:++failures;
        });
        lease.begin();
        assert(!toolbar->property("visible").toBool() && !toolbar->property("shown").toBool());
        assert(!root->findChild<QQuickItem *>("fakeNativeClose")->isVisible());
        assert(QTest::qWaitFor([&]{return renders || failures;},1000));
        assert(renders==1 && failures==0 && lease.readyForCapture());
        lease.release(); lease.release();
        assert(toolbar->property("visible").toBool()==originalVisible);
        assert(toolbar->property("expanded").toBool()==originalExpanded);
        root->setProperty("toolbarVisibleSetting",!originalVisible);
        assert(toolbar->property("visible").toBool()==!originalVisible); // Binding survives.
    }
    root->setProperty("toolbarVisibleSetting",true);
    {
        DiaryCaptureUiSession cancelled(&window,[](auto,const auto &){});
        cancelled.begin(); assert(!toolbar->property("visible").toBool());
        cancelled.release(); assert(toolbar->property("visible").toBool());
    }
    {
        auto destroyed=std::make_unique<DiaryCaptureUiSession>(&window,[](auto,const auto &){});
        destroyed->begin(); assert(!toolbar->property("visible").toBool());
        destroyed.reset(); assert(toolbar->property("visible").toBool());
    }
    {
        int failures=0;
        DiaryCaptureUiSession failed(&window,[&](auto event,const auto &){
            if(event==DiaryCaptureUiSession::Event::Failed)++failures;
        });
        failed.begin(); assert(!toolbar->property("visible").toBool());
        auto *newOverlay=new QQuickItem(root); // Actual invalidation/error path.
        assert(failures==1 && toolbar->property("visible").toBool());
        delete newOverlay;
    }
    {
        // Native binding source changes while hidden: restore the current bound
        // value, not a stale snapshot that could undo a real app state change.
        DiaryCaptureUiSession changed(&window,[](auto,const auto &){});
        changed.begin(); root->setProperty("toolbarVisibleSetting",false);
        changed.release(); assert(!toolbar->property("visible").toBool());
        root->setProperty("toolbarVisibleSetting",true);
        assert(toolbar->property("visible").toBool());
    }
    std::cout<<"PASS: native toolbar and close hidden for actual lease; shown/collapsed/hidden restoration, binding retention, cancellation and teardown\n";
    root->setProperty("diaryNativePoseObjects",QVariantMap{});QTest::qWait(20);
    clickItem(launcher);assert(panel->isVisible() && !dismiss->isVisible() && collapse->isVisible());
    assert(FakeBroker::starts==startsBefore && FakeBroker::cancels==cancelsBefore);
    std::cout<<"PASS: compact idle launcher, explicit expand/collapse, exact expanded capture gate, busy Cancel, toolbar-aware × and hidden recovery, suppression precedence, unknown-state fallback\n";
#endif
    root->setProperty("documentLoaded",false);assert(!panel->isVisible());
#ifdef INK_MOVE_LIVE_TEST
    root->setProperty("diaryNativePoseObjects",QVariantMap{{"toolbar",QVariant::fromValue(toolbar)}});
    root->setProperty("toolbarVisibleSetting",true);
    panel->setProperty("diaryCaptureSuppressed",true);
    assert(!toolbar->property("visible").toBool());
    delete panel;
    assert(!toolbar->property("visible").toBool()); // Owner-intended destruction exception.
    std::cout<<"PASS: destroying suppressed live widget may intentionally leave native toolbar hidden\n";
    delete root;
#else
    delete panel;delete root;
#endif
}
#include "test-ui.moc"
