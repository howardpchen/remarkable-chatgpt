#define INK_SPIKE_HOST_TEST
#include "spike.cpp"
#include <QTemporaryDir>
#include <iostream>
#include <limits>
class View : public QQuickItem {
 Q_OBJECT
public:
 int mode=0;
 Q_INVOKABLE QVariant viewToScene(QVariant p) { auto q=p.toPointF();return mode==1?QPointF(0,0):mode==2?QPointF(-q.x(),q.y()):QPointF(q.x()*2+10,q.y()*2+20); }
};
class Controller : public QObject {
 Q_OBJECT
public:
 int calls=0;
 Q_INVOKABLE void insertImageFileAsSceneItem(QUrl,QPointF){++calls;}
};
int main(int argc,char **argv) {
 QGuiApplication app(argc,argv);QQmlEngine engine;QQuickWindow window;window.resize(954,1696);
 QQuickItem document(window.contentItem());document.setSize(QSizeF(954,1696));
 Controller controller;document.setProperty("documentLoaded",true);document.setProperty("sceneController",QVariant::fromValue<QObject *>(&controller));document.setProperty("currentPageId","test-page");
 View view;view.setParentItem(&document);view.setSize(QSizeF(954,1696));view.setProperty("controller",QVariant::fromValue<QObject *>(&controller));QQmlEngine::setContextForObject(&view,engine.rootContext());
 window.show();app.processEvents();
 auto p=spike::current();if(p.controller!=&controller || p.view!=&view)return 1;
 if(spike::geometry(p).isEmpty())return 2;
 view.mode=1;if(!spike::geometry(p).isEmpty())return 3;
 view.mode=2;if(!spike::geometry(p).isEmpty())return 4;
 view.mode=0;
 for(qreal invalid : {qreal(0),qreal(-1),std::numeric_limits<qreal>::infinity(),std::numeric_limits<qreal>::quiet_NaN()}) {
  if(spike::validDimensions(invalid,1696) || spike::validDimensions(954,invalid))return 11;
  // Qt ignores NaN setters. Exercise NaN through the actual shared guard;
  // test the other invalid values through a real QQuickItem/QML context.
  if(std::isnan(invalid))continue;
  view.setWidth(invalid);if(!spike::geometry(p).isEmpty())return 11;view.setWidth(954);
  view.setHeight(invalid);if(!spike::geometry(p).isEmpty())return 12;view.setHeight(1696);
 }
 if(spike::run("insert").value("ok").toBool())return 5;
 if(!spike::run("prepare").value("ok").toBool())return 6;
 document.setProperty("currentPageId","other-page");
 if(spike::run("insert").value("ok").toBool() || controller.calls)return 7;
 document.setProperty("currentPageId","test-page");view.setX(5);
 if(spike::run("insert").value("ok").toBool() || controller.calls)return 8;
 view.setX(0);
 const QString path=spike::base+"/calibration.png";
 QImage original(path);if(!spike::isCalibrationImage(original))return 13;
 // Same dimensions are insufficient: changed ink, blank content and alpha
 // changes must fail before the single-attempt guard or native call is touched.
 for(int mode=0;mode<3;++mode) {
  QImage changed=original.convertToFormat(QImage::Format_ARGB32);
  if(mode==0)changed.setPixelColor(120,80,Qt::white);
  if(mode==1)changed.fill(Qt::white);
  if(mode==2)changed.setPixelColor(20,20,QColor(0,0,0,0));
  if(!changed.save(path))return 14;
  auto result=spike::run("insert");
  if(result.value("ok").toBool() || result.value("error")!="fixture-content-changed" || controller.calls || spike::attempted)return 15;
 }
 // A different valid QImage format / PNG encoding with identical canonical
 // pixels is accepted, and must still obey the existing one-shot semantics.
 if(!original.convertToFormat(QImage::Format_RGBA8888).save(path))return 16;
 if(!spike::run("insert").value("ok").toBool() || controller.calls!=1)return 9;
 if(spike::run("insert").value("ok").toBool() || controller.calls!=1)return 10;
 std::cout<<"Offline spike: finite positive geometry, fixed fixture content, page/viewport freshness, native invocation and single-attempt checks passed\n";
}
#include "test-spike.moc"
