// Host interaction/preview of the unmodified production Move widget at Ferrari
// dimensions. No tablet, provider, credential, native insertion or runtime gate.
#include "layout-candidate.h"
#include "geometry.h"
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <cassert>
#include <iostream>

class FixtureBroker : public QObject {
    Q_OBJECT
public:
    static inline int starts=0, cancels=0;
    static inline QString expectedSignal="ink-diary-move-live$control";
    static inline bool armed=false, busy=false;
    Q_INVOKABLE QString sendSimpleSignal(QString signal,QString command) {
        assert(signal==expectedSignal);
        if(command=="start") { assert(armed && !busy); ++starts; busy=true; }
        else if(command=="cancel") { assert(busy); ++cancels; busy=false; }
        else assert(command=="status");
        return QString::fromUtf8(QJsonDocument(QJsonObject{
            {"canStart",armed},{"busy",busy},
            {"message",busy?"Thinking: offline fixture":"Ready: offline fixture"}
        }).toJson(QJsonDocument::Compact));
    }
};
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    const QString mode=argc>1?QString::fromLocal8Bit(argv[1]):QString();
    const bool session=mode.startsWith("session");
    const int turn=mode=="session-cw"?90:mode=="session-ccw"?270:0;
    if(session)FixtureBroker::expectedSignal="ink-diary-paper-pro-live$control";
    const auto layout=paperProPortrait::extrapolatedLayout({1620,2160});
    // Fail closed for Move, landscape and other screen sizes: this is not a
    // second implementation of production capture admission.
    for(const auto size:{QSize(954,1696),QSize(2160,1620),QSize(1620,2159)}) {
        bool rejected=false;
        try{paperProPortrait::extrapolatedLayout(size);}catch(const std::runtime_error&){rejected=true;}
        assert(rejected);
    }
    qmlRegisterType<FixtureBroker>("net.asivery.XoviMessageBroker",2,0,"XoviMessageBroker");
    QQmlEngine engine;QQuickWindow window;window.resize(layout.window);window.setColor(Qt::white);
    QQmlComponent rootComponent(&engine);
    rootComponent.setData(R"QML(import QtQuick 2.15
Item { id: root; width:1620; height:2160; property bool documentLoaded:true
    property int readingTurn:0
    property bool toolbarSetting:true
    property var diaryNativePoseObjects: ({toolbar:bar,zoomButton:zoom,sceneView:reading})
    Item {id:reading; objectName:"fixtureReading"; width:root.readingTurn ? 2160 : 1620; height:root.readingTurn ? 1620 : 2160; transformOrigin:Item.TopLeft; rotation:root.readingTurn; x:root.readingTurn===90 ? 1620 : 0; y:root.readingTurn===270 ? 2160 : 0}
    Item {id:bar; visible:root.toolbarSetting; property bool expanded:true; readonly property bool shown:visible && expanded}
    Item {id:zoom; visible:false}
})QML",QUrl());
    auto *root=qobject_cast<QQuickItem*>(rootComponent.create());assert(root);
    root->setParentItem(window.contentItem());
    root->setProperty("readingTurn",turn);
    auto *reading=root->findChild<QQuickItem*>("fixtureReading");assert(reading);
    auto expectedBounds=[&](int w,int h){return reading->mapRectToScene(QRectF(reading->width()-16-w,reading->height()-16-h,w,h));};
    QQmlComponent widget(&engine,QUrl::fromLocalFile(QDir::current().absoluteFilePath(session?"tools/paper-pro-portrait/session/PaperProLive.qml":"tools/move-session/MoveLive.qml")));
    auto *panel=qobject_cast<QQuickItem*>(widget.beginCreate(engine.rootContext()));assert(panel);
    panel->setParentItem(root);widget.completeCreate();window.show();QTest::qWait(300);
    auto bounds=[&](QQuickItem *item){return item->mapRectToScene(item->boundingRect());};
    auto click=[&](QQuickItem *item){assert(item->isVisible());QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,item->mapToScene({item->width()/2,item->height()/2}).toPoint());QTest::qWait(30);};
    auto *launcher=panel->findChild<QQuickItem*>("moveUiProbeLauncher");
    auto *button=panel->findChild<QQuickItem*>("moveUiProbeButton");
    auto *card=panel->findChild<QQuickItem*>("moveUiProbeCard");
    auto *collapse=panel->findChild<QQuickItem*>("moveUiProbeCollapseButton");
    assert(launcher && button && card && collapse);
    assert(bounds(panel)==expectedBounds(72,72));
    assert(FixtureBroker::starts==0 && FixtureBroker::cancels==0);
    click(launcher);assert(bounds(panel)==expectedBounds(360,176));
    // Setup-needed clicks must not dispatch. Opening the panel never starts.
    click(button);assert(FixtureBroker::starts==0);
    FixtureBroker::armed=true;QTest::qWait(300);
    assert(panel->property("demoArmed").toBool());
    assert(bounds(card)==expectedBounds(280,152));
    auto save=[&](const char *name){
        auto dir=qEnvironmentVariable("INK_RMPP_UI_PREVIEW_DIR");if(dir.isEmpty())return;
        assert(QDir().mkpath(dir));QTest::qWait(50);
        const auto pixels=window.grabWindow();assert(pixels.size()==layout.window);
        assert(pixels.save(dir+"/"+mode+"-"+name+".png"));
    };
    save("rmpp-expanded-offline");
    click(button);assert(FixtureBroker::starts==1 && panel->property("demoBusy").toBool());
    assert(bounds(panel)==expectedBounds(360,176));
    assert(!collapse->isVisible()); // Cancel remains reachable while busy.
    click(button);assert(FixtureBroker::cancels==1 && !panel->property("demoBusy").toBool());
    QTest::qWait(350);assert(FixtureBroker::starts==1); // No automatic retry.
    // The exact owner survives a compact card, actionable error and suppression.
    panel->setProperty("localMessage","Cannot read question. Please rewrite.");
    QCoreApplication::processEvents();assert(bounds(card)==expectedBounds(360,176));
    save("rmpp-notice-offline");
    panel->setProperty("diaryCaptureSuppressed",true);QCoreApplication::processEvents();assert(!panel->isVisible());
    panel->setProperty("diaryCaptureSuppressed",false);QCoreApplication::processEvents();assert(panel->isVisible());
    root->setProperty("documentLoaded",false);QCoreApplication::processEvents();assert(!panel->isVisible());
    root->setProperty("documentLoaded",true);QCoreApplication::processEvents();assert(panel->isVisible());
    // Synthetic physical/logical identity is explicitly a HYPOTHESIS, not a
    // profile emitted from this test. Scene scale deliberately differs from 1.
    using namespace paperProPortrait;
    Viewport reference{{1620,2160},1,QTransform(1.5,0,0,1.5,17,29),"synthetic-only","Blank"};
    Candidate candidate(reference,QTransform(),layout.proposedContent);
    QImage capture(layout.window,QImage::Format_RGB32);capture.fill(Qt::white);
    {QPainter painter(&capture);painter.fillRect(100,180,420,12,Qt::black);}
    const auto placed=candidate.prepare(capture,reference,"## Offline fixture\n\nThis is a fixed answer, not an AI response.");
    assert(QRectF(layout.proposedContent).contains(placed.physicalBounds));
    assert(placed.nativeCandidate.width()==placed.rendered.width()*1.5);
    auto changed=reference;changed.pageId="changed-page";bool rejected=false;
    try{candidate.prepare(capture,changed,"Refuse stale page");}catch(const std::runtime_error&){rejected=true;}
    assert(rejected);
    const auto dir=qEnvironmentVariable("INK_RMPP_UI_PREVIEW_DIR");
    if(!dir.isEmpty()) {
        QPainter painter(&capture);painter.drawImage(placed.physicalBounds.topLeft(),placed.rendered);painter.end();
        assert(capture.save(dir+"/rmpp-placement-synthetic.png"));
    }
    delete panel;delete root;
    std::cout<<"PASS Ferrari-sized production UI: explicit Start/Cancel, no auto-start/retry, setup refusal, exact owner, visibility/suppression; synthetic placement and stale-page refusal. No runtime admission.\n";
}
#include "test-layout-ui.moc"
