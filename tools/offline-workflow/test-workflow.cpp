#define INK_WORKFLOW_HOST_TEST
#include "workflow.cpp"
#include <QTemporaryDir>
#include <cassert>
#include <iostream>
class View : public QQuickItem {
 Q_OBJECT
public:
 int mode=0;double scale=2;
 Q_INVOKABLE QVariant viewToScene(QVariant p) {
   const auto q=p.toPointF();
   if(mode==1)return QPointF(-q.x(),q.y());
   if(mode==2)return QPointF(q.x()+q.y()/2,q.y());
   return QPointF(q.x()*scale+10,q.y()*scale+20);
 }
};
class Controller : public QObject {
 Q_OBJECT
public:
 int calls=0;QPointF target;
 Q_INVOKABLE void insertImageFileAsSceneItem(QUrl,QPointF point){++calls;target=point;}
};
int main(int argc,char **argv) {
 // Test root is generated before workflow.cpp's global base is initialized by
 // test.sh, avoiding changes to real device or old spike paths.
 QGuiApplication app(argc,argv);QQmlEngine engine;QQuickWindow window;window.resize(954,1696);
 QQuickItem document(window.contentItem());document.setSize({954,1696});
 Controller controller;document.setProperty("documentLoaded",true);document.setProperty("sceneController",QVariant::fromValue<QObject *>(&controller));document.setProperty("currentPageId","fixture-page");
 View view;view.setParentItem(&document);view.setSize({954,1696});view.setProperty("controller",QVariant::fromValue<QObject *>(&controller));QQmlEngine::setContextForObject(&view,engine.rootContext());
 window.show();app.processEvents();
 assert(!workflow::base.isEmpty());
 QImage screen(954,1696,QImage::Format_RGB32);screen.fill(Qt::white);screen.setPixelColor(400,160,Qt::black);
 int captures=0;bool changedCapture=false,writeCapture=true;
 workflow::captureHandler=[&](char *arg) {
    ++captures;const auto path=QString::fromUtf8(arg).section(',',0,0);
    auto image=screen;if(changedCapture)image.setPixelColor(400,161,Qt::black);
    if(writeCapture)assert(image.save(path));return strdup("success");
 };
 auto call=[](const char *op){return workflow::run({{"op",op}});};
 const QJsonObject prep{{"op","prepare"},{"measuredCrop",QJsonArray{112,112,842,1584}}};
 // Synthetic crop for host tests only, not hardware calibration evidence.
 assert(!call("insert").value("ok").toBool());
 // Unknown/stale operations fail terminally; start new synthetic scenario only
 // in the host test. Production has no reset API.
 auto reset=[&]{workflow::phase=workflow::Phase::Idle;workflow::question={};workflow::answer={};
   for(const auto &name:{"/question.png","/check.png","/answer.png","/INSERTION_CONSUMED"})QFile::remove(workflow::base+name);};
 reset();assert(!call("prepare").value("ok").toBool());
 view.mode=1;assert(!workflow::run(prep).value("ok").toBool());view.mode=2;assert(!workflow::run(prep).value("ok").toBool());view.mode=0;
 assert(!workflow::run({{"op","prepare"},{"measuredCrop",QJsonArray{0,0,954,1696}}}).value("ok").toBool());
 assert(workflow::run(prep).value("ok").toBool());
 assert(call("capture").value("ok").toBool());assert(captures==1);
 assert(!call("capture").value("ok").toBool());assert(captures==1);
 auto render=call("render");assert(render.value("ok").toBool());
 assert(render.value("answerWidth").toInt()==1684);
 assert(render.value("screenWidth").toDouble()==842);
 assert(call("verify").value("ok").toBool());
 // Tampering with answer pixels cannot invoke native insertion.
 auto original=QImage(workflow::base+"/answer.png");auto changed=original;changed.setPixelColor(0,0,Qt::black);assert(changed.save(workflow::base+"/answer.png"));
 assert(call("insert").value("error")=="answer-pixels-changed");assert(controller.calls==0);
 reset();assert(workflow::run(prep).value("ok").toBool());assert(call("capture").value("ok").toBool());assert(call("render").value("ok").toBool());
 changedCapture=true;assert(call("verify").value("ok").toBool());assert(call("insert").value("error")=="page-pixels-changed");assert(controller.calls==0);changedCapture=false;
 reset();assert(workflow::run(prep).value("ok").toBool());assert(call("capture").value("ok").toBool());
 document.setProperty("currentPageId","other");assert(!call("render").value("ok").toBool());document.setProperty("currentPageId","fixture-page");assert(call("render").value("error")=="workflow-terminal");
 reset();assert(workflow::run(prep).value("ok").toBool());view.setX(1);assert(!call("capture").value("ok").toBool());view.setX(0);assert(controller.calls==0);
 reset();assert(workflow::run(prep).value("ok").toBool());writeCapture=false;assert(call("capture").value("ok").toBool());
 assert(call("render").value("error")=="capture-pending");assert(controller.calls==0);writeCapture=true;
 reset();auto savedScreen=screen;screen.fill(Qt::white);assert(workflow::run(prep).value("ok").toBool());assert(call("capture").value("ok").toBool());
 assert(call("render").value("error")=="no-visible-ink");screen=savedScreen;
 reset();screen.setPixelColor(400,1650,Qt::black);assert(workflow::run(prep).value("ok").toBool());assert(call("capture").value("ok").toBool());
 assert(call("render").value("error")=="answer-does-not-fit-visible-crop");screen=savedScreen;assert(controller.calls==0);
 reset();assert(workflow::run(prep).value("ok").toBool());assert(call("cancel").value("ok").toBool());assert(call("capture").value("error")=="workflow-terminal");
 reset();assert(workflow::run(prep).value("ok").toBool());assert(call("capture").value("ok").toBool());assert(call("render").value("ok").toBool());assert(call("verify").value("ok").toBool());
 assert(call("insert").value("ok").toBool());assert(controller.calls==1);
 assert(QFile::exists(workflow::base+"/INSERTION_CONSUMED"));assert(!call("insert").value("ok").toBool());assert(controller.calls==1);
 workflow::phase=workflow::Phase::Idle;assert(workflow::run(prep).value("error")=="insertion-already-consumed");assert(controller.calls==1);
 assert(controller.target.x()==2*(112+842/2.0)+10);
 reset();view.scale=0.85984848;controller.calls=0;
 assert(workflow::run(prep).value("ok").toBool());assert(call("capture").value("ok").toBool());
 auto scaled=call("render");assert(scaled.value("ok").toBool());
 assert(scaled.value("answerWidth").toInt()==int(std::floor(842*view.scale)));
 assert(scaled.value("screenWidth").toDouble()<=842);
 assert(call("verify").value("ok").toBool());assert(call("insert").value("ok").toBool());assert(controller.calls==1);
 assert(std::abs(controller.target.x()-(10+(112+842/2.0)*view.scale))<0.001);
 assert(std::abs(controller.target.y()-(20+(scaled.value("answerTop").toDouble()+scaled.value("screenHeight").toDouble()/2)*view.scale))<0.001);
 std::cout<<"PASS: bounded capture/render/verify fixture workflow; page/viewport/cancel/content/duplicate guards; one mock native invocation\n";
}
#include "test-workflow.moc"
