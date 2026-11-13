#define INK_PAPER_PRO_DEMO_HOST_TEST
#define INK_PAPER_PRO_WORKFLOW_HOST_TEST
#include "runtime.cpp"
#include <QQmlComponent>
#include <QTest>
#include <cassert>
#include <iostream>
#include <QLineF>
#ifdef INK_PAPER_PRO_LIVE_TEST
#include "mock-transport.h"
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
    int degrees=0;
    Q_INVOKABLE QVariant viewToScene(QVariant xy) {return QTransform().rotate(degrees).map(xy.toPointF()*factor)+origin;}
};
class MockController : public QObject {
    Q_OBJECT
public:
    int calls=0;
    QPointF lastTarget;
    Q_INVOKABLE void insertImageFileAsSceneItem(QUrl,QPointF point) {++calls;lastTarget=point;}
};
namespace paperProRuntime {
struct RuntimeTest {
    static void setup(Runtime &r,std::function<QString()> check) {
        r.native_.testVerify=std::move(check); r.pauseMs_=20; r.settleMs_=20;
        r.captureTimeoutMs_=200; r.overallTimeoutMs_=5000;
    }
#ifdef INK_PAPER_PRO_LIVE_TEST
    static void live(Runtime &r,QNetworkAccessManager *manager) {
        r.testManager_=manager;
#ifdef INK_PAPER_PRO_PER_REQUEST
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
#ifdef INK_PAPER_PRO_DOLLAR_BUDGET
    QFile policy(workflow::sessionRoot+"/budget-policy.json");assert(policy.open(QIODevice::WriteOnly));
    policy.write("{\"schema\":1,\"model\":\"gpt-6-luna\",\"totalNanoUsd\":100000000,\"firstRequestId\":1}");
    policy.close();assert(policy.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
#endif
#ifdef INK_PAPER_PRO_PER_REQUEST
    const int initialAllowance=paperProSession::remaining(workflow::sessionRoot);
#endif
    QQmlEngine engine; QQuickWindow window;
    const QString scenario=argc>1?QString::fromLocal8Bit(argv[1]):"success";
    const bool landscape=scenario.startsWith("landscape");
    const bool split=scenario.startsWith("landscape-split");
    const bool landscapePattern=split && (scenario.contains("-grid-") || scenario.contains("-dots-"));
    const bool reverseLandscape=split && scenario.contains("reverse");
    const bool variableZoom=(scenario.startsWith("zoom-") && scenario!="zoom-change" && scenario!="zoom-change-api") || (split && scenario.contains("-zoom-"));
    const bool zoomFailure=variableZoom && scenario.contains("change-");
    const QSize size=landscape?QSize(2160,1620):QSize(1620,2160);
    window.resize(split?QSize(1620,2160):size);
    QQmlComponent rootComponent(&engine);
    rootComponent.setData("import QtQuick 2.15\nItem { width:1620; height:2160; property bool documentLoaded:true; property QtObject document; property QtObject sceneController; property int currentPage:0; property string currentPageId:'synthetic-page'; property alias testToolbar:bar; Item { id:bar; visible:true; property bool expanded:true; readonly property bool shown:visible && expanded } }",QUrl());
    auto *root=qobject_cast<QQuickItem *>(rootComponent.create()); assert(root);
    root->setParentItem(window.contentItem());root->setSize(split?QSize(1620,2160):size);
    MockDocument document; MockController controller; MockView view;
    root->setProperty("document",QVariant::fromValue<QObject *>(&document));
    root->setProperty("sceneController",QVariant::fromValue<QObject *>(&controller));
    view.setParentItem(root); view.setSize(size);
    if(split) {view.setTransformOrigin(QQuickItem::TopLeft);view.setRotation(reverseLandscape?270:90);view.setPosition(reverseLandscape?QPointF(0,2160):QPointF(1620,0));}
    if(scenario=="landscape-cw")view.degrees=90;
    if(scenario=="landscape-ccw")view.degrees=270;
    view.setProperty("controller",QVariant::fromValue<QObject *>(&controller));
    QQmlEngine::setContextForObject(&view,engine.rootContext());
    engine.rootContext()->setContextProperty("testDocument",root);
    QQmlComponent panelComponent(&engine);
    panelComponent.setData("import QtQuick 2.15\nRectangle { id:panel; objectName:'moveUiProbe'; width:360; height:176; x:1244; y:1968; readonly property string diaryCaptureOwner:'ink-diary-owned-control-v1'; property bool diaryCaptureSuppressed:false; visible:!diaryCaptureSuppressed; color:'white'; "
        "property bool requestBridgeAvailable:true; property bool diaryRequestToolbarSuppressed:false; readonly property bool nativeRequestToolbarReady: requestBridgeAvailable && parent && !testDocument.testToolbar.visible && !testDocument.testToolbar.expanded; "
        "Binding { target:panel.parent ? testDocument.testToolbar : null; property:'visible'; value:false; when:panel.diaryRequestToolbarSuppressed; restoreMode:Binding.RestoreBindingOrValue } "
        "Binding { target:panel.parent ? testDocument.testToolbar : null; property:'expanded'; value:false; when:panel.diaryRequestToolbarSuppressed; restoreMode:Binding.RestoreBindingOrValue } }",QUrl());
    auto *panel=qobject_cast<QQuickItem *>(panelComponent.create()); assert(panel); panel->setParentItem(split?&view:root);
    panel->setPosition({qreal(size.width()-376),qreal(size.height()-192)});
    auto *toolbar=root->property("testToolbar").value<QObject *>();assert(toolbar);
    window.show(); assert(QTest::qWaitFor([&]{return window.isExposed();}));
    const auto page=workflow::current(); assert(page.view==&view && page.document==root);
    const auto vp=paperProRuntime::viewport(page); assert(!vp.isEmpty());
#ifdef INK_PAPER_PRO_PER_REQUEST
    const QString referenceName="/device-profile.json";
#else
    const QString referenceName="/reviewed-reference.json";
#endif
    QFile reference(workflow::base+referenceName); assert(reference.open(QIODevice::WriteOnly));
#ifdef INK_PAPER_PRO_LIVE_TEST
    auto profile=QJsonObject{{"schema",
#ifdef INK_PAPER_PRO_PER_REQUEST
        1
#else
        2
#endif
    },{"product","ferrari"},{"release","3.28.0.172"},{"qt","6.10.3"},{"viewport",vp},
        {"nativePose",QJsonObject{{"ok",true}}},{"hiddenNativePose",QJsonObject{{"ok",true}}},
        {"captureArea",QJsonArray{0,0,window.width(),window.height()}}};
#ifdef INK_PAPER_PRO_PER_REQUEST
    if(landscape && scenario!="landscape-missing" && scenario!="landscape-split-missing") {
        auto calibration=profile;
        profile.insert("orientations",QJsonObject{{reverseLandscape?"landscape-ccw":"landscape-cw",calibration}});
    }
#endif
    reference.write(QJsonDocument(profile).toJson());
#else
    reference.write(QJsonDocument(QJsonObject{{"schema",1},{"viewport",vp}}).toJson());
#endif
    reference.close();
    assert(reference.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
#ifdef INK_PAPER_PRO_PER_REQUEST
    if(scenario=="landscape-split-preview") {
        QVariantMap bindings;
        const QStringList names={"_uiContainer","pages","pageSlider","pageLabel","titleLabel","newPageButton","_toolbarTooltipOverlay","selectionHelper","formatSettings","formatMenuDelegate","addTagPopup","_searchResultsHeader","searchResultsProgress","notificationBar","hwcDialog","adjustViewPopup","_popupOverlay","zoomButton","scrollLockVisualCue","writingFormatsToolTip","connectivityDialogContainerItem","_tocDeleteLoader","_tocRenamePopup","_tocCrossDocErrorLoader","_tocCrossDocDownloadLoader"};
        QQuickItem hidden(root);hidden.setVisible(false);
        for(const auto &name:names)bindings.insert(name,QVariant::fromValue<QObject *>(&hidden));
        bindings.insert("sceneView",QVariant::fromValue<QObject *>(&view));bindings.insert("toolbar",QVariant::fromValue(toolbar));
        root->setProperty("diaryNativePoseObjects",bindings);
        const auto before=diagnostic::nativePose(root);assert(before["ok"].toBool());
        const auto preview=paperProRuntime::calibrationPreview(false);assert(preview["ok"].toBool());
        assert(preview["setupTelemetryOnly"].toBool() && !preview["normalZoomCertified"].toBool());
        assert(preview["viewport"]==vp && preview["hiddenNativePose"].toObject()["states"].toObject()["toolbar"].toObject()["visible"]==false);
        assert(diagnostic::nativePose(root)==before && !panel->property("diaryRequestToolbarSuppressed").toBool());
        assert(!paperProRuntime::calibrationPreview(true)["ok"].toBool());
        assert(paperProSession::remaining(workflow::sessionRoot)==initialAllowance && !QFile::exists(workflow::sessionRoot+"/1"));
        std::cout<<"PASS calibration telemetry: exact pose restoration, busy refusal, no capture, transport, reservation or insertion\n";
        hidden.setParentItem(nullptr);hidden.setParent(nullptr);delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
#endif
    int captures=0,nativeChecks=0;
    bool captureVisible=false;
    QImage image(size,QImage::Format_RGB32); image.fill(Qt::white); image.setPixelColor(400,160,Qt::black);
#ifdef INK_PAPER_PRO_LIVE_TEST
    // Writing at the very first row must survive crop, payload and placement.
    image.setPixelColor(400,160,Qt::white);
    image.setPixelColor(0,0,Qt::black);
#endif
    workflow::captureHandler=[&](char *argument) {
        ++captures; captureVisible |= panel->isVisible();
        if(scenario=="native-change-capture")controller.setProperty("itemsBoundingRect",QRectF(0,400,100,100));
        if(scenario=="landscape-change-capture")window.resize(1620,2160);
        if(scenario=="landscape-split-change-capture")view.setRotation(270);
        if(scenario=="landscape-split-reverse-change-capture")view.setRotation(90);
        const auto path=QString::fromUtf8(argument).section(',',0,0);
        if (scenario!="capture-timeout") {
            auto frame=split?image.transformed(QTransform().rotate(reverseLandscape?270:90)):image;
            if(scenario=="landscape-frame-mismatch")frame=QImage(1620,2160,QImage::Format_RGB32);
            if(scenario=="landscape-split-frame-mismatch")frame=image;
            if (scenario=="changed-pixels" && captures==2) frame.setPixelColor(450,diaryPaperProBoundedCaptureArea().y()+80,Qt::black);
            QTimer::singleShot(10,&app,[frame,path]{assert(frame.save(path));});
        }
        if (scenario=="page-change") root->setProperty("currentPageId","changed");
        if (scenario=="template-change") document.name="Dots";
        if (scenario=="zoom-change") view.factor*=1.2;
        if(variableZoom && scenario.endsWith("change-capture"))view.origin+=QPointF(10,20);
        return strdup("success");
    };
#ifdef INK_PAPER_PRO_LIVE_TEST
    LiveMockManager manager;
    if(scenario=="count-over-cap")manager.countResponse="{\"object\":\"response.input_tokens\",\"input_tokens\":922000}";
    if(scenario=="count-malformed")manager.countResponse="{\"input_tokens\":1}";
    if(scenario=="count-http-error")manager.countCode=401;
    if(scenario=="count-timeout" || scenario=="cancel-count" || scenario=="page-change-count")manager.countDelay=-1;
    manager.onCount=[&]{
        if(scenario=="cancel-count")QTimer::singleShot(10,&app,[&]{paperProRuntime::runtime->cancel();});
        if(scenario=="page-change-count")QTimer::singleShot(10,&app,[&]{root->setProperty("currentPageId","changed");});
    };

    if(scenario=="malformed") manager.response="not json";
    if(scenario=="needs-web") manager.response=mockAnswer("synthetic-private-question","needs_web");
    if(scenario=="clarification") manager.response=mockAnswer("synthetic-private-question","clarification");
    if(scenario=="unknown-kind") manager.response=mockAnswer("synthetic-private-question","image");
    if(scenario=="unsupported") manager.response=mockAnswer("Search not supported","unsupported");
    if(scenario=="too-large") manager.response=mockAnswer(QString("A long line of text.\n").repeated(160));
    if(scenario=="http-error")manager.code=401;
    if(scenario=="shutdown-api" || scenario=="teardown-api" || scenario=="api-timeout" || scenario=="cancel-api" || scenario=="page-change-api" || scenario=="template-change-api" || scenario=="zoom-change-api" || scenario=="dots-change-api" || scenario=="landscape-change-api" || scenario=="landscape-split-change-api")manager.delay=-1;
    if(variableZoom && scenario.endsWith("change-api"))manager.delay=-1;
    if(scenario=="landscape-split-reverse-change-api")manager.delay=-1;
    if(scenario=="native-change-api")manager.delay=-1;
    manager.onDispatch=[&]{
        if(variableZoom && scenario.endsWith("change-api"))QTimer::singleShot(10,&app,[&]{if(scenario.contains("template-change"))document.name="Blank";else view.origin+=QPointF(10,20);});
        assert(panel->isVisible()); // Own UI restored throughout API wait.
#ifdef INK_PAPER_PRO_PER_REQUEST
        assert(panel->property("diaryRequestToolbarSuppressed").toBool());
        assert(!toolbar->property("visible").toBool() && !toolbar->property("expanded").toBool());
#endif
        if(scenario=="landscape-split-change-api")QTimer::singleShot(10,&app,[&]{view.setRotation(270);});
        if(scenario=="landscape-split-reverse-change-api")QTimer::singleShot(10,&app,[&]{view.setRotation(90);});
        if(scenario=="landscape-change-api")QTimer::singleShot(10,&app,[&]{window.resize(1620,2160);});
        if(scenario=="dots-change-api")QTimer::singleShot(10,&app,[&]{document.name="P Grid small";});
        if(scenario=="native-change-api")controller.setProperty("itemsBoundingRect",QRectF(0,100,30,30));
        if(scenario=="same-page-ink")image.setPixelColor(450,80,Qt::black);
        const auto body=QJsonDocument::fromJson(manager.sent).object();
        const auto parts=body["input"].toArray().first().toObject()["content"].toArray();
        const auto uri=parts[1].toObject()["image_url"].toString();
        const auto sentImage=QImage::fromData(QByteArray::fromBase64(uri.section(',',1).toLatin1()));
        assert(sentImage.size()==size && sentImage.pixelColor(0,0)==Qt::black);
        if(landscapePattern || scenario=="zoom-dots" || scenario=="dots-before-start" || (scenario=="zoom-grid" || scenario=="grid-before-start" || scenario=="grid-table-before-start"))assert(workflow::samePixels(sentImage,image));
        if(scenario=="shutdown-api")QTimer::singleShot(10,&app,[&]{assert(QMetaObject::invokeMethod(&app,"aboutToQuit",Qt::DirectConnection));});
        if(scenario=="cancel-api")QTimer::singleShot(10,&app,[&]{paperProRuntime::runtime->cancel();});
        if(scenario=="page-change-api")QTimer::singleShot(10,&app,[&]{root->setProperty("currentPageId","changed");});
        if(scenario=="template-change-api")QTimer::singleShot(10,&app,[&]{document.name="Dots";});
        if(scenario=="zoom-change-api")QTimer::singleShot(10,&app,[&]{view.factor*=1.2;});
    };
    QFile config(workflow::base+"/config.json"); assert(config.open(QIODevice::WriteOnly));
    config.write("{\"api_key\":\"synthetic-not-a-key\",\"model\":\"gpt-6-luna\"}");config.close();
    assert(config.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
#endif
    auto runtimeOwner=std::make_unique<paperProRuntime::Runtime>(&app);
    auto &runtime=*runtimeOwner;
#ifdef INK_PAPER_PRO_LIVE_TEST
    paperProRuntime::runtime=&runtime;
    paperProRuntime::RuntimeTest::live(runtime,&manager);
#endif
    paperProRuntime::RuntimeTest::setup(runtime,[&] {
        ++nativeChecks;
#ifdef INK_PAPER_PRO_PER_REQUEST
        assert(!toolbar->property("visible").toBool() && !toolbar->property("expanded").toBool());
#endif
        return scenario=="native-pose-change" && nativeChecks>1?QString("native-ui-pose-changed"):QString();
    });
    if (scenario=="overall-timeout") paperProRuntime::RuntimeTest::timeout(runtime);
#ifdef INK_PAPER_PRO_LIVE_TEST
    if(scenario=="old-reference") {
        QFile old(workflow::base+referenceName); assert(old.open(QIODevice::WriteOnly|QIODevice::Truncate));
        old.write(QJsonDocument(QJsonObject{{"schema",1},{"viewport",vp}}).toJson());old.close();
        runtime.start(); assert(!runtime.status()["busy"].toBool());
        assert(manager.requests==0 && controller.calls==0 && !QFile::exists(workflow::base+"/START_CONSUMED"));
        std::cout<<"PASS integrated old cropped reference rejected before request or insertion\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
#endif
#ifdef INK_PAPER_PRO_PER_REQUEST
    if(scenario=="toolbar-unavailable" || scenario=="toolbar-already-held") {
        if(scenario=="toolbar-unavailable")panel->setProperty("requestBridgeAvailable",false);
        else panel->setProperty("diaryRequestToolbarSuppressed",true);
        runtime.start();
        assert(!runtime.status()["busy"].toBool() && manager.requests==0 && captures==0 && controller.calls==0);
        assert(paperProSession::remaining(workflow::sessionRoot)==initialAllowance);
        assert(!QFile::exists(workflow::sessionRoot+"/1"));
        assert(panel->property("diaryRequestToolbarSuppressed").toBool()==(scenario=="toolbar-already-held"));
        panel->setProperty("diaryRequestToolbarSuppressed",false);
        assert(toolbar->property("visible").toBool() && toolbar->property("expanded").toBool());
        std::cout<<"PASS toolbar refusal before allocation, no stolen lease: "<<scenario.toStdString()<<"\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
    if(scenario=="landscape-missing" || scenario=="landscape-split-missing" || scenario=="missing-profile" || scenario=="public-profile" || scenario=="zoomed-before-start") {
        if(scenario=="missing-profile")assert(reference.remove());
        if(scenario=="public-profile")assert(reference.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadOther));
        if(landscapePattern && scenario.endsWith("-missing"))document.name=scenario.contains("-dots-")?"P Dots S":"P Grid small";
        if(scenario=="zoomed-before-start")view.factor/=4.1;
        if(scenario=="template-before-start")document.name="Dots";
        runtime.start();
        assert(!runtime.status()["busy"].toBool() && controller.calls==0 && manager.requests==0 && captures==0);
        assert(paperProSession::remaining(workflow::sessionRoot)==initialAllowance);
        assert(!QFile::exists(workflow::sessionRoot+"/1"));
        std::cout<<"PASS automatic Start refusal before request allocation: "<<scenario.toStdString()<<"\n";
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
    if(scenario=="translated-before-start")view.origin=QPointF(-410,137);
    assert(!QFile::exists(workflow::sessionRoot+"/reviewed-reference.json"));
#endif
#ifdef INK_PAPER_PRO_PER_REQUEST
    if((landscapePattern && scenario.contains("-dots-")) || scenario=="zoom-dots" || scenario=="dots-before-start" || scenario=="dots-empty" || scenario=="dots-change-api")document.name="P Dots S";
    if((landscapePattern && scenario.contains("-grid-")) || (scenario=="zoom-grid" || scenario=="grid-before-start" || scenario=="grid-table-before-start") || scenario=="grid-empty")document.name="P Grid small";
    if(landscapePattern || scenario=="zoom-dots" || scenario=="dots-before-start" || (scenario=="zoom-grid" || scenario=="grid-before-start" || scenario=="grid-table-before-start") || scenario=="dots-empty" || scenario=="grid-empty") {
        image.fill(Qt::white);
        QPainter painter(&image);painter.setPen(QColor(180,180,180));
        const double pitch=(document.name=="P Dots S"?63.75:47.25)*(variableZoom?(scenario.endsWith("out")?0.5:scenario.endsWith("max")?4.0:2.0):1.0); // Synthetic normal pitch scaled to live zoom.
        if(document.name=="P Grid small") {
            for(double y=0;y<size.height();y+=pitch)painter.drawLine(0,int(y),size.width()-1,int(y));
            for(double x=17;x<size.width();x+=pitch)painter.drawLine(int(x),0,int(x),size.height()-1);
        } else {
            for(double y=0;y<size.height();y+=pitch)for(double x=7.5;x<size.width();x+=pitch)
                painter.fillRect(int(x),int(y),
                    variableZoom?(scenario.endsWith("out")?1:scenario.endsWith("max")?8:4):2,
                    variableZoom?(scenario.endsWith("out")?1:scenario.endsWith("max")?8:4):2,Qt::black);
        }
        if(!scenario.endsWith("-empty"))painter.fillRect(400,140,40,landscapePattern?15:20,Qt::black);
        if(scenario=="grid-table-before-start") {
            painter.fillRect(180,320,560,12,Qt::black);
            painter.fillRect(200,410,520,2,Qt::black);
            painter.fillRect(240,200,8,600,Qt::black);
        }
        painter.end();
        if(!scenario.endsWith("-empty"))image.setPixelColor(0,0,Qt::black);
    }
#endif
#ifdef INK_PAPER_PRO_PER_REQUEST
    if(document.name=="P Dots S" || document.name=="P Grid small") {
        auto patternViewport=paperProRuntime::viewport(page);
        auto entry=QJsonObject{{"viewport",patternViewport},
            {"nativePose",QJsonObject{{"ok",true}}},
            {"hiddenNativePose",QJsonObject{{"ok",true}}},
            {"pixelPitch",document.name=="P Dots S"?63.75:47.25}};
        if(landscapePattern && !scenario.endsWith("-missing")) {
            entry.insert("captureArea",QJsonArray{0,0,1620,2160});
            auto orientations=profile.value("orientations").toObject();
            const auto key=reverseLandscape?"landscape-ccw":"landscape-cw";
            auto orientation=orientations.value(key).toObject();
            orientation.insert("templates",QJsonObject{{document.name,entry}});
            orientations.insert(key,orientation);profile.insert("orientations",orientations);
        } else if(!landscapePattern) profile.insert("templates",QJsonObject{{document.name,entry}});
        assert(reference.open(QIODevice::WriteOnly|QIODevice::Truncate));
        reference.write(QJsonDocument(profile).toJson());reference.close();
    }
    if(variableZoom) {
        view.factor/=scenario.endsWith("out")?0.5:scenario.endsWith("max")?4.0:2.0;
        view.origin=QPointF(-417,213);
        if(scenario=="zoom-grid")document.name="P Grid small";
        if(scenario=="zoom-dots")document.name="P Dots S";
    }
#endif
#ifdef INK_PAPER_PRO_PER_REQUEST
    if(scenario=="template-before-start")document.name="Synthetic ruled";
    if(scenario=="template-unavailable")document.name="";
    const auto nativeAxes=diaryPaperProReadingAxes(paperProRuntime::viewport(page));
    auto nativeRect=QRectF(nativeAxes.map({0,0}),nativeAxes.map({1,0})).normalized();
    if(scenario=="native-placement")nativeRect=QRectF(0,100,200,200);
    if(scenario=="native-offscreen")nativeRect=QRectF(0,3000,200,200);
    if(scenario.endsWith("-empty"))nativeRect={};
    controller.setProperty("itemsBoundingRect",nativeRect);
    controller.setProperty("boundingRect",nativeRect);
    controller.setProperty("rootDocumentBoundingRect",QRectF());
    if(scenario=="native-missing")controller.setProperty("boundingRect",QVariant());
#endif
    runtime.start();
    if(scenario=="native-missing") {
        assert(!runtime.status()["busy"].toBool() && captures==0 && manager.requests==0 && controller.calls==0);
        assert(runtime.status()["message"].toString().contains("native-content-bounds-unavailable"));
        delete panel;view.setParentItem(nullptr);delete root;return 0;
    }
    assert(runtime.status()["busy"].toBool());
#ifdef INK_PAPER_PRO_PER_REQUEST
    const auto firstRequest=workflow::base;
    runtime.start(); // busy duplicate: same identity, one request, no cancellation
    assert(workflow::base==firstRequest && runtime.status()["busy"].toBool());
#endif
#ifdef INK_PAPER_PRO_LIVE_TEST
    if(scenario=="teardown-api" || scenario=="teardown-hidden") {
        assert(QTest::qWaitFor([&]{return scenario=="teardown-api"?manager.requests==1:!panel->isVisible();},3000));
        runtimeOwner.reset();QTest::qWait(300);
#ifdef INK_PAPER_PRO_PER_REQUEST
        assert(!panel->property("diaryRequestToolbarSuppressed").toBool());
        assert(toolbar->property("visible").toBool() && toolbar->property("expanded").toBool());
#endif
        assert(controller.calls==0 && panel->isVisible() && !panel->property("diaryCaptureSuppressed").toBool());
        assert(manager.requests==(scenario=="teardown-api"?1:0));
        assert(manager.aborts==(scenario=="teardown-api"?1:0));
        assert(QFile::exists(workflow::base+"/START_CONSUMED") && !QFile::exists(workflow::base+"/INSERTION_CONSUMED"));
        paperProRuntime::Runtime reloaded(&app);
        paperProRuntime::RuntimeTest::setup(reloaded,[]{return QString();});paperProRuntime::RuntimeTest::live(reloaded,&manager);
        #ifdef INK_PAPER_PRO_PER_REQUEST
#ifdef INK_PAPER_PRO_DOLLAR_BUDGET
        assert(paperProSession::remaining(workflow::sessionRoot)==initialAllowance-(scenario=="teardown-api"?1:0));
#else
        assert(paperProSession::remaining(workflow::sessionRoot)==initialAllowance-1);
#endif
#else
        assert(!reloaded.status()["canStart"].toBool());reloaded.start();assert(!reloaded.status()["busy"].toBool());
#endif
        assert(manager.requests==(scenario=="teardown-api"?1:0));
        std::cout<<
#ifdef INK_PAPER_PRO_PER_REQUEST
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
#ifdef INK_PAPER_PRO_PER_REQUEST
    assert(!panel->property("diaryRequestToolbarSuppressed").toBool());
    assert(toolbar->property("visible").toBool() && toolbar->property("expanded").toBool());
#endif
    assert(!captureVisible);
#ifdef INK_PAPER_PRO_SINGLE_CAPTURE
    assert(captures<=1 && !QFile::exists(workflow::base+"/check.png"));
#endif
    assert(QFile::exists(workflow::base+"/START_CONSUMED"));
    const bool successExpected=scenario=="native-placement" || scenario=="template-before-start" || scenario=="template-unavailable" || (variableZoom && !zoomFailure) || (landscape && !variableZoom && !scenario.contains("change-") && !scenario.contains("frame-mismatch")) || scenario=="success" || scenario=="translated-before-start" || scenario=="dots-before-start" || (scenario=="grid-before-start" || scenario=="grid-table-before-start")
#ifdef INK_PAPER_PRO_SINGLE_CAPTURE
        || scenario=="same-page-ink" || scenario=="changed-pixels"
#endif
        ;
    std::cerr<<"TERMINAL "<<scenario.toStdString()<<": "<<runtime.status()["message"].toString().toStdString()<<"\n";
    if (successExpected) {
        assert(controller.calls==1 && captures==
#ifdef INK_PAPER_PRO_SINGLE_CAPTURE
            1
#else
            2
#endif
            && nativeChecks>=5);
        assert(QFile::exists(workflow::base+"/INSERTION_CONSUMED"));
        if(scenario=="native-placement") {
            const double height=workflow::answer.height()/view.factor;
            const auto expected=nativeAxes.map({120+(size.width()-120)/2.0,std::ceil(300/view.factor)+45+height/2});
            assert(QLineF(expected,controller.lastTarget).length()<1e-6);
        }
        if(landscape || variableZoom) {
            const auto axes=diaryPaperProReadingAxes(paperProRuntime::viewport(page));
            assert(axes.degrees==view.degrees);
            if(!variableZoom)assert(view.degrees ? workflow::answer.height()>1000 : workflow::answer.width()>1000);
            const double height=(view.degrees?workflow::answer.width():workflow::answer.height())/view.factor;
            const double x=120+(size.width()-120)/2.0;
            const auto expected=QTransform().rotate(view.degrees).map(QPointF(x,45+height/2)*view.factor)+view.origin;
            assert(QLineF(expected,controller.lastTarget).length()<1e-6);
        }
    } else {assert(controller.calls==0);assert(!QFile::exists(workflow::base+"/INSERTION_CONSUMED"));}
#ifdef INK_PAPER_PRO_LIVE_TEST
    const bool apiExpected=scenario=="native-placement" || scenario=="template-before-start" || scenario=="template-unavailable" || (variableZoom && !scenario.endsWith("change-capture")) || (landscape && !variableZoom && !scenario.contains("change-capture") && !scenario.contains("frame-mismatch")) || scenario=="success" || scenario=="translated-before-start" || scenario=="dots-before-start" || (scenario=="grid-before-start" || scenario=="grid-table-before-start") || scenario=="same-page-ink" || scenario=="changed-pixels" || scenario=="malformed" || scenario=="unsupported" || scenario=="needs-web" || scenario=="clarification" || scenario=="unknown-kind" || scenario=="too-large" || scenario=="http-error" || scenario=="api-timeout" || scenario.endsWith("-api");
    assert(manager.requests==(apiExpected?1:0));
    if(scenario=="shutdown-api" || scenario=="cancel-api" || scenario=="api-timeout" || scenario.endsWith("change-api"))assert(manager.aborts==1);
#endif
    const auto terminalMessage=runtime.status()["message"].toString();
    if(manager.requests) {
        const auto sent=QJsonDocument::fromJson(manager.sent).object();
        const auto counted=QJsonDocument::fromJson(manager.countBody).object();
        for(const auto *field:{"model","input","instructions","text","reasoning"})assert(sent[field]==counted[field]);
        assert(sent["max_output_tokens"]==800 && !sent.contains("tools"));
        assert(manager.countRequests==1);
    }
    if(scenario.startsWith("count-") || scenario.endsWith("-count")) {
        assert(manager.countRequests==1 && manager.requests==0 && controller.calls==0);
        assert(paperProSession::budgetStatus(workflow::sessionRoot).available==0);
    }

    if(scenario=="dots-empty" || scenario=="grid-empty") {
        assert(captures==1 && controller.calls==0);
#ifdef INK_PAPER_PRO_LIVE_TEST
        assert(manager.requests==0);
#endif
        assert(terminalMessage.contains("no-visible-ink"));
    }
#ifdef INK_PAPER_PRO_LIVE_TEST
    if(scenario=="needs-web" || scenario=="clarification" || scenario=="unsupported" || scenario=="unknown-kind") {
        assert(captures==1 && manager.requests==1 && controller.calls==0);
        assert(!terminalMessage.contains("synthetic-private-question"));
        if(scenario=="needs-web")assert(terminalMessage=="Needs web access; search is off.");
        if(scenario=="clarification")assert(terminalMessage=="Cannot read question. Please rewrite.");
        if(scenario=="unsupported")assert(terminalMessage=="Text answers only; try another question.");
        if(scenario=="unknown-kind")assert(terminalMessage=="Unknown answer kind");
    }
#endif
    assert(paperProSession::remaining(workflow::sessionRoot)==initialAllowance-((manager.countRequests || manager.requests)?1:0));
    assert(QFile::exists(workflow::base+((manager.countRequests || manager.requests)?"/BUDGET_DISPATCH":"/BUDGET_RELEASE")));
    if(manager.requests) {
        const auto requests=manager.requests;
        runtime.start();QTest::qWait(50);
        assert(!runtime.status()["busy"].toBool() && manager.requests==requests);
        paperProRuntime::Runtime reloaded(&app);
        assert(!reloaded.status()["canStart"].toBool());
    }
    std::cout<<"PASS integrated bounded runtime: "<<scenario.toStdString()<<"; "<<terminalMessage.toStdString()<<"\n";
    delete panel;
    view.setParentItem(nullptr); delete root;
}
#include "test-runtime.moc"
