#define INK_MOVE_DEMO_HOST_TEST
#define INK_WORKFLOW_HOST_TEST
#include "runtime.cpp"
#include <QQmlComponent>
#include <QTest>
#include <cassert>
#include <iostream>
#ifdef INK_MOVE_LIVE_TEST
#include "../move-live/mock-transport.h"
#endif
class MockDocument : public QObject {
    Q_OBJECT
public:
    QString name="Blank";
    Q_INVOKABLE QString templateForPage(int) {return name;}
};
class MockView : public QQuickItem {
    Q_OBJECT
public:
    qreal factor=0.859848484848485;
    Q_INVOKABLE QVariant viewToScene(QVariant xy) {return xy.toPointF()*factor;}
};
class MockController : public QObject {
    Q_OBJECT
public:
    int calls=0;
    Q_INVOKABLE void insertImageFileAsSceneItem(QUrl,QPointF) {++calls;}
};
namespace moveDemo {
struct RuntimeTest {
    static void setup(Runtime &r,std::function<QString()> check) {
        r.native_.testVerify=std::move(check); r.pauseMs_=20; r.settleMs_=20;
        r.captureTimeoutMs_=200; r.overallTimeoutMs_=5000;
    }
#ifdef INK_MOVE_LIVE_TEST
    static void live(Runtime &r,QNetworkAccessManager *manager) {
        r.testManager_=manager;r.configPath_=workflow::base+"/config.json";
        r.testApiTimeoutMs_=200;
    }
#endif
    static void timeout(Runtime &r) {r.overallTimeoutMs_=50;}
};
}
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv); QQmlEngine engine; QQuickWindow window;
    window.resize(954,1696);
    QQmlComponent rootComponent(&engine);
    rootComponent.setData("import QtQuick 2.15\nItem { width:954; height:1696; property bool documentLoaded:true; property QtObject document; property QtObject sceneController; property int currentPage:0; property string currentPageId:'synthetic-page' }",QUrl());
    auto *root=qobject_cast<QQuickItem *>(rootComponent.create()); assert(root);
    root->setParentItem(window.contentItem());
    MockDocument document; MockController controller; MockView view;
    root->setProperty("document",QVariant::fromValue<QObject *>(&document));
    root->setProperty("sceneController",QVariant::fromValue<QObject *>(&controller));
    view.setParentItem(root); view.setSize({954,1696});
    view.setProperty("controller",QVariant::fromValue<QObject *>(&controller));
    QQmlEngine::setContextForObject(&view,engine.rootContext());
    QQmlComponent panelComponent(&engine);
    panelComponent.setData("import QtQuick 2.15\nRectangle { objectName:'moveUiProbe'; width:360; height:176; x:578; y:1504; readonly property string diaryCaptureOwner:'ink-diary-owned-control-v1'; property bool diaryCaptureSuppressed:false; visible:!diaryCaptureSuppressed; color:'white' }",QUrl());
    auto *panel=qobject_cast<QQuickItem *>(panelComponent.create()); assert(panel); panel->setParentItem(root);
    window.show(); assert(QTest::qWaitFor([&]{return window.isExposed();}));
    const auto page=workflow::current(); assert(page.view==&view && page.document==root);
    const auto vp=moveDemo::viewport(page); assert(!vp.isEmpty());
    QFile reference(workflow::base+"/reviewed-reference.json"); assert(reference.open(QIODevice::WriteOnly));
    reference.write(QJsonDocument(QJsonObject{{"schema",1},{"viewport",vp}}).toJson()); reference.close();
    const QString scenario=argc>1?QString::fromLocal8Bit(argv[1]):"success";
    int captures=0,nativeChecks=0;
    bool captureVisible=false;
    QImage image(954,1696,QImage::Format_RGB32); image.fill(Qt::white); image.setPixelColor(400,160,Qt::black);
    workflow::captureHandler=[&](char *argument) {
        ++captures; captureVisible |= panel->isVisible();
        const auto path=QString::fromUtf8(argument).section(',',0,0);
        if (scenario!="capture-timeout") {
            auto frame=image;
            if (scenario=="changed-pixels" && captures==2) frame.setPixelColor(450,200,Qt::black);
            QTimer::singleShot(10,&app,[frame,path]{assert(frame.save(path));});
        }
        if (scenario=="page-change") root->setProperty("currentPageId","changed");
        if (scenario=="template-change") document.name="Dots";
        if (scenario=="zoom-change") view.factor*=1.2;
        return strdup("success");
    };
#ifdef INK_MOVE_LIVE_TEST
    LiveMockManager manager;
    if(scenario=="malformed") manager.response="not json";
    if(scenario=="unsupported") manager.response=mockAnswer("Search not supported","unsupported");
    if(scenario=="too-large") manager.response=mockAnswer(QString("A long line of text.\n").repeated(160));
    if(scenario=="http-error")manager.code=401;
    if(scenario=="teardown-api" || scenario=="api-timeout" || scenario=="cancel-api" || scenario=="page-change-api" || scenario=="template-change-api" || scenario=="zoom-change-api")manager.delay=-1;
    manager.onDispatch=[&]{
        assert(panel->isVisible()); // Own UI restored throughout API wait.
        if(scenario=="cancel-api")QTimer::singleShot(10,&app,[&]{moveDemo::runtime->cancel();});
        if(scenario=="page-change-api")QTimer::singleShot(10,&app,[&]{root->setProperty("currentPageId","changed");});
        if(scenario=="template-change-api")QTimer::singleShot(10,&app,[&]{document.name="Dots";});
        if(scenario=="zoom-change-api")QTimer::singleShot(10,&app,[&]{view.factor*=1.2;});
    };
    QFile config(workflow::base+"/config.json"); assert(config.open(QIODevice::WriteOnly));
    config.write("{\"api_key\":\"synthetic-not-a-key\",\"model\":\"gpt-4.1-mini\"}");config.close();
    assert(config.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
#endif
    auto runtimeOwner=std::make_unique<moveDemo::Runtime>(&app);
    auto &runtime=*runtimeOwner;
#ifdef INK_MOVE_LIVE_TEST
    moveDemo::runtime=&runtime;
    moveDemo::RuntimeTest::live(runtime,&manager);
#endif
    moveDemo::RuntimeTest::setup(runtime,[&] {++nativeChecks;return scenario=="native-pose-change" && nativeChecks>1?QString("native-ui-pose-changed"):QString();});
    if (scenario=="overall-timeout") moveDemo::RuntimeTest::timeout(runtime);
    runtime.start(); assert(runtime.status()["busy"].toBool());
#ifdef INK_MOVE_LIVE_TEST
    if(scenario=="teardown-api" || scenario=="teardown-hidden") {
        assert(QTest::qWaitFor([&]{return scenario=="teardown-api"?manager.requests==1:!panel->isVisible();},3000));
        runtimeOwner.reset();QTest::qWait(300);
        assert(controller.calls==0 && panel->isVisible() && !panel->property("diaryCaptureSuppressed").toBool());
        assert(manager.requests==(scenario=="teardown-api"?1:0));
        assert(manager.aborts==(scenario=="teardown-api"?1:0));
        assert(QFile::exists(workflow::base+"/START_CONSUMED") && !QFile::exists(workflow::base+"/INSERTION_CONSUMED"));
        moveDemo::Runtime reloaded(&app);
        moveDemo::RuntimeTest::setup(reloaded,[]{return QString();});moveDemo::RuntimeTest::live(reloaded,&manager);
        assert(!reloaded.status()["canStart"].toBool());reloaded.start();assert(!reloaded.status()["busy"].toBool());
        assert(manager.requests==(scenario=="teardown-api"?1:0));
        std::cout<<"PASS integrated teardown and persistent reload refusal: "<<scenario.toStdString()<<"\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
#endif
    if (scenario=="cancel") QTimer::singleShot(10,&runtime,[&]{runtime.cancel();});
    if (scenario=="cancel-hidden") {
        QTimer cancelTimer; cancelTimer.setInterval(5);
        QObject::connect(&cancelTimer,&QTimer::timeout,&app,[&]{if(!panel->isVisible())runtime.cancel();});
        cancelTimer.start(); assert(QTest::qWaitFor([&]{return !runtime.status()["busy"].toBool();},6000));
    } else assert(QTest::qWaitFor([&]{return !runtime.status()["busy"].toBool();},6000));
    assert(panel->isVisible() && !panel->property("diaryCaptureSuppressed").toBool());
    assert(!captureVisible);
    assert(QFile::exists(workflow::base+"/START_CONSUMED"));
    if (scenario=="success") {
        assert(controller.calls==1 && captures==2 && nativeChecks>=5);
        assert(QFile::exists(workflow::base+"/INSERTION_CONSUMED"));
    } else {assert(controller.calls==0);assert(!QFile::exists(workflow::base+"/INSERTION_CONSUMED"));}
#ifdef INK_MOVE_LIVE_TEST
    const bool apiExpected=scenario=="success" || scenario=="changed-pixels" || scenario=="malformed" || scenario=="unsupported" || scenario=="too-large" || scenario=="http-error" || scenario=="api-timeout" || scenario.endsWith("-api");
    assert(manager.requests==(apiExpected?1:0));
    if(scenario=="cancel-api" || scenario=="api-timeout" || scenario.endsWith("change-api"))assert(manager.aborts==1);
#endif
    const auto terminalMessage=runtime.status()["message"].toString();
    runtime.start(); QTest::qWait(50); assert(controller.calls==(scenario=="success"?1:0));
    std::cout<<"PASS integrated bounded runtime: "<<scenario.toStdString()<<"; "<<terminalMessage.toStdString()<<"\n";
    delete panel;
    view.setParentItem(nullptr); delete root;
}
#include "test-runtime.moc"
