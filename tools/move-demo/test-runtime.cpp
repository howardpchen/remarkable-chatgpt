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
    QPointF origin;
    Q_INVOKABLE QVariant viewToScene(QVariant xy) {return xy.toPointF()*factor+origin;}
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
        r.testManager_=manager;
#ifdef INK_MOVE_PER_REQUEST
        r.configPath_=workflow::sessionRoot+"/config.json";
#else
        r.configPath_=workflow::base+"/config.json";
#endif
        r.testApiTimeoutMs_=200;
    }
#endif
    static void timeout(Runtime &r) {r.overallTimeoutMs_=50;}
};
}
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
#ifdef INK_MOVE_DOLLAR_BUDGET
    QFile policy(workflow::sessionRoot+"/budget-policy.json");assert(policy.open(QIODevice::WriteOnly));
    policy.write("{\"schema\":1,\"model\":\"gpt-6-luna\",\"totalNanoUsd\":5000000000,\"firstRequestId\":1}");
    policy.close();assert(policy.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
#endif
#ifdef INK_MOVE_PER_REQUEST
    const int initialAllowance=moveSession::remaining(workflow::sessionRoot);
#endif
    QQmlEngine engine; QQuickWindow window;
    window.resize(954,1696);
    QQmlComponent rootComponent(&engine);
    rootComponent.setData("import QtQuick 2.15\nItem { width:954; height:1696; property bool documentLoaded:true; property QtObject document; property QtObject sceneController; property int currentPage:0; property string currentPageId:'synthetic-page'; property alias testToolbar:bar; Item { id:bar; visible:true; property bool expanded:true } }",QUrl());
    auto *root=qobject_cast<QQuickItem *>(rootComponent.create()); assert(root);
    root->setParentItem(window.contentItem());
    MockDocument document; MockController controller; MockView view;
    root->setProperty("document",QVariant::fromValue<QObject *>(&document));
    root->setProperty("sceneController",QVariant::fromValue<QObject *>(&controller));
    view.setParentItem(root); view.setSize({954,1696});
    view.setProperty("controller",QVariant::fromValue<QObject *>(&controller));
    QQmlEngine::setContextForObject(&view,engine.rootContext());
    QQmlComponent panelComponent(&engine);
    panelComponent.setData("import QtQuick 2.15\nRectangle { id:panel; objectName:'moveUiProbe'; width:360; height:176; x:578; y:1504; readonly property string diaryCaptureOwner:'ink-diary-owned-control-v1'; property bool diaryCaptureSuppressed:false; visible:!diaryCaptureSuppressed; color:'white'; "
        "property bool requestBridgeAvailable:true; property bool diaryRequestToolbarSuppressed:false; readonly property bool nativeRequestToolbarReady: requestBridgeAvailable && parent && !parent.testToolbar.visible && !parent.testToolbar.expanded; "
        "Binding { target:panel.parent ? panel.parent.testToolbar : null; property:'visible'; value:false; when:panel.diaryRequestToolbarSuppressed; restoreMode:Binding.RestoreBindingOrValue } "
        "Binding { target:panel.parent ? panel.parent.testToolbar : null; property:'expanded'; value:false; when:panel.diaryRequestToolbarSuppressed; restoreMode:Binding.RestoreBindingOrValue } }",QUrl());
    auto *panel=qobject_cast<QQuickItem *>(panelComponent.create()); assert(panel); panel->setParentItem(root);
    auto *toolbar=root->property("testToolbar").value<QObject *>();assert(toolbar);
    window.show(); assert(QTest::qWaitFor([&]{return window.isExposed();}));
    const auto page=workflow::current(); assert(page.view==&view && page.document==root);
    const auto vp=moveDemo::viewport(page); assert(!vp.isEmpty());
#ifdef INK_MOVE_PER_REQUEST
    const QString referenceName="/device-profile.json";
#else
    const QString referenceName="/reviewed-reference.json";
#endif
    QFile reference(workflow::base+referenceName); assert(reference.open(QIODevice::WriteOnly));
#ifdef INK_MOVE_LIVE_TEST
    reference.write(QJsonDocument(QJsonObject{{"schema",
#ifdef INK_MOVE_PER_REQUEST
        3
#else
        2
#endif
    },{"viewport",vp},
        {"nativePose",QJsonObject{{"ok",true}}},{"hiddenNativePose",QJsonObject{{"ok",true}}},
        {"captureArea",QJsonArray{0,0,954,1696}}}).toJson());
#else
    reference.write(QJsonDocument(QJsonObject{{"schema",1},{"viewport",vp}}).toJson());
#endif
    reference.close();
    assert(reference.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
    const QString scenario=argc>1?QString::fromLocal8Bit(argv[1]):"success";
    int captures=0,nativeChecks=0;
    bool captureVisible=false;
    QImage image(954,1696,QImage::Format_RGB32); image.fill(Qt::white); image.setPixelColor(400,160,Qt::black);
#ifdef INK_MOVE_LIVE_TEST
    // Writing at the very first row must survive crop, payload and placement.
    image.setPixelColor(400,160,Qt::white);
    image.setPixelColor(0,0,Qt::black);
#endif
    workflow::captureHandler=[&](char *argument) {
        ++captures; captureVisible |= panel->isVisible();
        const auto path=QString::fromUtf8(argument).section(',',0,0);
        if (scenario!="capture-timeout") {
            auto frame=image;
            if (scenario=="changed-pixels" && captures==2) frame.setPixelColor(450,diaryMoveBoundedCaptureArea().y()+80,Qt::black);
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
    if(scenario=="needs-web") manager.response=mockAnswer("synthetic-private-question","needs_web");
    if(scenario=="clarification") manager.response=mockAnswer("synthetic-private-question","clarification");
    if(scenario=="unknown-kind") manager.response=mockAnswer("synthetic-private-question","image");
    if(scenario=="unsupported") manager.response=mockAnswer("Search not supported","unsupported");
    if(scenario=="too-large") manager.response=mockAnswer(QString("A long line of text.\n").repeated(160));
    if(scenario=="http-error")manager.code=401;
    if(scenario=="shutdown-api" || scenario=="teardown-api" || scenario=="api-timeout" || scenario=="cancel-api" || scenario=="page-change-api" || scenario=="template-change-api" || scenario=="zoom-change-api" || scenario=="dots-change-api")manager.delay=-1;
    manager.onDispatch=[&]{
        assert(panel->isVisible()); // Own UI restored throughout API wait.
#ifdef INK_MOVE_PER_REQUEST
        assert(panel->property("diaryRequestToolbarSuppressed").toBool());
        assert(!toolbar->property("visible").toBool() && !toolbar->property("expanded").toBool());
#endif
        if(scenario=="dots-change-api")QTimer::singleShot(10,&app,[&]{document.name="P Grid small";});
        if(scenario=="same-page-ink")image.setPixelColor(450,80,Qt::black);
        const auto body=QJsonDocument::fromJson(manager.sent).object();
        const auto parts=body["input"].toArray().first().toObject()["content"].toArray();
        const auto uri=parts[1].toObject()["image_url"].toString();
        const auto sentImage=QImage::fromData(QByteArray::fromBase64(uri.section(',',1).toLatin1()));
        assert(sentImage.size()==QSize(954,1696) && sentImage.pixelColor(0,0)==Qt::black);
        if(scenario=="dots-before-start" || (scenario=="grid-before-start" || scenario=="grid-table-before-start"))assert(workflow::samePixels(sentImage,image));
        if(scenario=="shutdown-api")QTimer::singleShot(10,&app,[&]{assert(QMetaObject::invokeMethod(&app,"aboutToQuit",Qt::DirectConnection));});
        if(scenario=="cancel-api")QTimer::singleShot(10,&app,[&]{moveDemo::runtime->cancel();});
        if(scenario=="page-change-api")QTimer::singleShot(10,&app,[&]{root->setProperty("currentPageId","changed");});
        if(scenario=="template-change-api")QTimer::singleShot(10,&app,[&]{document.name="Dots";});
        if(scenario=="zoom-change-api")QTimer::singleShot(10,&app,[&]{view.factor*=1.2;});
    };
    QFile config(workflow::base+"/config.json"); assert(config.open(QIODevice::WriteOnly));
    config.write("{\"api_key\":\"synthetic-not-a-key\",\"model\":\"gpt-6-luna\"}");config.close();
    assert(config.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
#endif
    auto runtimeOwner=std::make_unique<moveDemo::Runtime>(&app);
    auto &runtime=*runtimeOwner;
#ifdef INK_MOVE_LIVE_TEST
    moveDemo::runtime=&runtime;
    moveDemo::RuntimeTest::live(runtime,&manager);
#endif
    moveDemo::RuntimeTest::setup(runtime,[&] {
        ++nativeChecks;
#ifdef INK_MOVE_PER_REQUEST
        assert(!toolbar->property("visible").toBool() && !toolbar->property("expanded").toBool());
#endif
        return scenario=="native-pose-change" && nativeChecks>1?QString("native-ui-pose-changed"):QString();
    });
    if (scenario=="overall-timeout") moveDemo::RuntimeTest::timeout(runtime);
#ifdef INK_MOVE_LIVE_TEST
    if(scenario=="old-reference") {
        QFile old(workflow::base+referenceName); assert(old.open(QIODevice::WriteOnly|QIODevice::Truncate));
        old.write(QJsonDocument(QJsonObject{{"schema",1},{"viewport",vp}}).toJson());old.close();
        runtime.start(); assert(!runtime.status()["busy"].toBool());
        assert(manager.requests==0 && controller.calls==0 && !QFile::exists(workflow::base+"/START_CONSUMED"));
        std::cout<<"PASS integrated old cropped reference rejected before request or insertion\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
#endif
#ifdef INK_MOVE_PER_REQUEST
    if(scenario=="toolbar-unavailable" || scenario=="toolbar-already-held") {
        if(scenario=="toolbar-unavailable")panel->setProperty("requestBridgeAvailable",false);
        else panel->setProperty("diaryRequestToolbarSuppressed",true);
        runtime.start();
        assert(!runtime.status()["busy"].toBool() && manager.requests==0 && captures==0 && controller.calls==0);
        assert(moveSession::remaining(workflow::sessionRoot)==initialAllowance);
        assert(!QFile::exists(workflow::sessionRoot+"/1"));
        assert(panel->property("diaryRequestToolbarSuppressed").toBool()==(scenario=="toolbar-already-held"));
        panel->setProperty("diaryRequestToolbarSuppressed",false);
        assert(toolbar->property("visible").toBool() && toolbar->property("expanded").toBool());
        std::cout<<"PASS toolbar refusal before allocation, no stolen lease: "<<scenario.toStdString()<<"\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
    if(scenario=="missing-profile" || scenario=="public-profile" || scenario=="zoomed-before-start" || scenario=="template-before-start") {
        if(scenario=="missing-profile")assert(reference.remove());
        if(scenario=="public-profile")assert(reference.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadOther));
        if(scenario=="zoomed-before-start")view.factor*=1.2;
        if(scenario=="template-before-start")document.name="Dots";
        runtime.start();
        assert(!runtime.status()["busy"].toBool() && controller.calls==0 && manager.requests==0 && captures==0);
        assert(moveSession::remaining(workflow::sessionRoot)==initialAllowance);
        assert(!QFile::exists(workflow::sessionRoot+"/1"));
        std::cout<<"PASS automatic Start refusal before request allocation: "<<scenario.toStdString()<<"\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
    if(scenario=="translated-before-start")view.origin=QPointF(-410,137);
    assert(!QFile::exists(workflow::sessionRoot+"/reviewed-reference.json"));
#endif
#ifdef INK_MOVE_PER_REQUEST
    if(scenario=="dots-before-start" || scenario=="dots-empty" || scenario=="dots-change-api")document.name="P Dots S";
    if((scenario=="grid-before-start" || scenario=="grid-table-before-start") || scenario=="grid-empty")document.name="P Grid small";
    if(scenario=="dots-before-start" || (scenario=="grid-before-start" || scenario=="grid-table-before-start") || scenario=="dots-empty" || scenario=="grid-empty") {
        image.fill(Qt::white);
        QPainter painter(&image);painter.setPen(QColor(180,180,180));
        const double pitch=(document.name=="P Dots S"?42.5:52)/view.factor;
        if(document.name=="P Grid small") {
            for(double y=0;y<1696;y+=pitch)painter.drawLine(0,int(y),953,int(y));
            for(double x=17;x<954;x+=pitch)painter.drawLine(int(x),0,int(x),1695);
        } else {
            for(double y=0;y<1696;y+=pitch)for(double x=7.5;x<954;x+=pitch)
                painter.fillRect(int(x),int(y),2,2,Qt::black);
        }
        if(!scenario.endsWith("-empty"))painter.fillRect(400,140,40,20,Qt::black);
        if(scenario=="grid-table-before-start") {
            painter.fillRect(180,320,560,12,Qt::black);
            painter.fillRect(200,410,520,2,Qt::black);
            painter.fillRect(240,200,8,600,Qt::black);
        }
        painter.end();
        if(!scenario.endsWith("-empty"))image.setPixelColor(0,0,Qt::black);
    }
#endif
    runtime.start(); assert(runtime.status()["busy"].toBool());
#ifdef INK_MOVE_PER_REQUEST
    const auto firstRequest=workflow::base;
    runtime.start(); // busy duplicate: same identity, one request, no cancellation
    assert(workflow::base==firstRequest && runtime.status()["busy"].toBool());
#endif
#ifdef INK_MOVE_LIVE_TEST
    if(scenario=="teardown-api" || scenario=="teardown-hidden") {
        assert(QTest::qWaitFor([&]{return scenario=="teardown-api"?manager.requests==1:!panel->isVisible();},3000));
        runtimeOwner.reset();QTest::qWait(300);
#ifdef INK_MOVE_PER_REQUEST
        assert(!panel->property("diaryRequestToolbarSuppressed").toBool());
        assert(toolbar->property("visible").toBool() && toolbar->property("expanded").toBool());
#endif
        assert(controller.calls==0 && panel->isVisible() && !panel->property("diaryCaptureSuppressed").toBool());
        assert(manager.requests==(scenario=="teardown-api"?1:0));
        assert(manager.aborts==(scenario=="teardown-api"?1:0));
        assert(QFile::exists(workflow::base+"/START_CONSUMED") && !QFile::exists(workflow::base+"/INSERTION_CONSUMED"));
        moveDemo::Runtime reloaded(&app);
        moveDemo::RuntimeTest::setup(reloaded,[]{return QString();});moveDemo::RuntimeTest::live(reloaded,&manager);
        #ifdef INK_MOVE_PER_REQUEST
        assert(moveSession::remaining(workflow::sessionRoot)==initialAllowance-1);
#else
        assert(!reloaded.status()["canStart"].toBool());reloaded.start();assert(!reloaded.status()["busy"].toBool());
#endif
        assert(manager.requests==(scenario=="teardown-api"?1:0));
        std::cout<<
#ifdef INK_MOVE_PER_REQUEST
        "PASS integrated teardown and persistent quota across reload: "
#else
        "PASS integrated teardown and persistent reload refusal: "
#endif
        <<scenario.toStdString()<<"\n";
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
#ifdef INK_MOVE_PER_REQUEST
    assert(!panel->property("diaryRequestToolbarSuppressed").toBool());
    assert(toolbar->property("visible").toBool() && toolbar->property("expanded").toBool());
#endif
    assert(!captureVisible);
#ifdef INK_MOVE_SINGLE_CAPTURE
    assert(captures<=1 && !QFile::exists(workflow::base+"/check.png"));
#endif
    assert(QFile::exists(workflow::base+"/START_CONSUMED"));
    const bool successExpected=scenario=="success" || scenario=="translated-before-start" || scenario=="dots-before-start" || (scenario=="grid-before-start" || scenario=="grid-table-before-start")
#ifdef INK_MOVE_SINGLE_CAPTURE
        || scenario=="same-page-ink" || scenario=="changed-pixels"
#endif
        ;
    if (successExpected) {
        assert(controller.calls==1 && captures==
#ifdef INK_MOVE_SINGLE_CAPTURE
            1
#else
            2
#endif
            && nativeChecks>=5);
        assert(QFile::exists(workflow::base+"/INSERTION_CONSUMED"));
    } else {assert(controller.calls==0);assert(!QFile::exists(workflow::base+"/INSERTION_CONSUMED"));}
#ifdef INK_MOVE_LIVE_TEST
    const bool apiExpected=scenario=="success" || scenario=="translated-before-start" || scenario=="dots-before-start" || (scenario=="grid-before-start" || scenario=="grid-table-before-start") || scenario=="same-page-ink" || scenario=="changed-pixels" || scenario=="malformed" || scenario=="unsupported" || scenario=="needs-web" || scenario=="clarification" || scenario=="unknown-kind" || scenario=="too-large" || scenario=="http-error" || scenario=="api-timeout" || scenario.endsWith("-api");
    assert(manager.requests==(apiExpected?1:0));
    if(scenario=="shutdown-api" || scenario=="cancel-api" || scenario=="api-timeout" || scenario.endsWith("change-api"))assert(manager.aborts==1);
#endif
    const auto terminalMessage=runtime.status()["message"].toString();
    if(scenario=="dots-empty" || scenario=="grid-empty") {
        assert(captures==1 && controller.calls==0);
#ifdef INK_MOVE_LIVE_TEST
        assert(manager.requests==0);
#endif
        assert(terminalMessage.contains("no-visible-ink"));
    }
#ifdef INK_MOVE_LIVE_TEST
    if(scenario=="needs-web" || scenario=="clarification" || scenario=="unsupported" || scenario=="unknown-kind") {
        assert(captures==1 && manager.requests==1 && controller.calls==0);
        assert(!terminalMessage.contains("synthetic-private-question"));
        if(scenario=="needs-web")assert(terminalMessage=="Needs web access; search is off.");
        if(scenario=="clarification")assert(terminalMessage=="Cannot read question. Please rewrite.");
        if(scenario=="unsupported")assert(terminalMessage=="Text answers only; try another question.");
        if(scenario=="unknown-kind")assert(terminalMessage=="Unknown answer kind");
    }
#endif
#ifdef INK_MOVE_PER_REQUEST
    assert(moveSession::remaining(workflow::sessionRoot)==initialAllowance-1);
    if(scenario=="success" || scenario=="http-error" || scenario=="cancel") {
        QFile profileBefore(workflow::sessionRoot+referenceName);assert(profileBefore.open(QIODevice::ReadOnly));
        const auto originalProfile=profileBefore.readAll();profileBefore.close();
        for(int i=2;i<=3;++i) {
            // Different pages/origins reuse one unchanged device profile.
            root->setProperty("currentPageId",QString("synthetic-page-%1").arg(i));
            view.origin=QPointF(i*123,-i*17);
            runtime.start();assert(runtime.status()["busy"].toBool());
            assert(workflow::base==workflow::sessionRoot+"/"+QString::number(i));
            runtime.start(); // duplicate while busy
            assert(QTest::qWaitFor([&]{return !runtime.status()["busy"].toBool();},6000));
        }
        assert(profileBefore.open(QIODevice::ReadOnly));assert(profileBefore.readAll()==originalProfile);profileBefore.close();
#ifdef INK_MOVE_DOLLAR_BUDGET
        // Three requests do not exhaust the new guard. Consume the remainder
        // without networking to verify runtime blocks the next dispatch/reload.
        assert(runtime.status()["requestsRemaining"].toInt()==-1);
        assert(moveSession::remaining(workflow::sessionRoot)>0);
        while(!moveSession::reserve(workflow::sessionRoot).isEmpty()) {}
#endif
        assert(moveSession::remaining(workflow::sessionRoot)==0);
        const auto requests=manager.requests, inserts=controller.calls;
        runtime.start();QTest::qWait(50);
        assert(!runtime.status()["busy"].toBool() && manager.requests==requests && controller.calls==inserts);
        assert(requests==(scenario=="cancel"?2:3));
        assert(inserts==(scenario=="http-error"?0:(scenario=="cancel"?2:3)));
        moveDemo::Runtime reloaded(&app);moveDemo::RuntimeTest::setup(reloaded,[]{return QString();});
        moveDemo::RuntimeTest::live(reloaded,&manager); // config path remains session root
        assert(!reloaded.status()["canStart"].toBool());reloaded.start();
        assert(!reloaded.status()["busy"].toBool() && manager.requests==requests);
        assert(QFile::exists(firstRequest+"/START_CONSUMED"));
    }
#else
    runtime.start(); QTest::qWait(50); assert(controller.calls==(scenario=="success"?1:0));
#endif
    std::cout<<"PASS integrated bounded runtime: "<<scenario.toStdString()<<"; "<<terminalMessage.toStdString()<<"\n";
    delete panel;
    view.setParentItem(nullptr); delete root;
}
#include "test-runtime.moc"
