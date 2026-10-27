#include "readonly-mapping.h"
#include <QGuiApplication>
#include <QQuickItem>
#include <QPointF>
#include <QJsonArray>
#include <cassert>
#include <limits>
#include <iostream>
class TestView : public QQuickItem {
    Q_OBJECT
public:
    int mode=0;
    Q_INVOKABLE QVariant viewToScene(QVariant input) {
        const auto point=input.toPointF();
        if(mode==1)return QPointF(0,0);
        if(mode==2)return QPointF(std::numeric_limits<double>::quiet_NaN(),0);
        if(mode==3)return QPointF(-point.x(),point.y());
        if(mode==4)return QPointF(point.y(),point.x());
        return QPointF(point.x()*2+17,point.y()*3-9);
    }
};
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    QQmlEngine engine;
    QQuickItem parent;
    TestView view;view.setParentItem(&parent);
    view.setTransformOrigin(QQuickItem::TopLeft);
    view.setPosition(QPointF(23,41));view.setScale(1.5);
    const auto ownership=QQmlEngine::objectOwnership(&view);
    if(diagnostic::mappingStatus(&engine,&view)!="valid")return 1;
    view.mode=1;if(diagnostic::mappingStatus(&engine,&view)!="degenerate")return 2;
    view.mode=2;if(diagnostic::mappingStatus(&engine,&view)!="non-finite")return 3;
    if(diagnostic::mappingStatus(&engine,&parent)!="invocation-failed")return 4;
    if(diagnostic::mappingStatus(nullptr,&view)!="engine-unavailable")return 5;
    if(QQmlEngine::objectOwnership(&view)!=ownership)return 6;
    assert(diagnostic::viewportSnapshot(&engine,&view).isEmpty()); // No window.
    QQuickWindow window;
    window.setWidth(954);window.setHeight(1696);
    view.setParentItem(window.contentItem());
    view.setWidth(834);view.setHeight(1584);view.mode=0;
    const auto snapshot=diagnostic::viewportSnapshot(&engine,&view);
    assert(snapshot.size()==6 && snapshot.value("width")==834 && snapshot.value("height")==1584);
    assert(snapshot.value("windowWidth")==954 && snapshot.value("windowHeight")==1696);
    assert(snapshot.value("devicePixelRatio").toDouble()>0);
    assert(snapshot.value("points").toArray().size()==3);
    assert(snapshot.value("points").toArray()[0].toArray().size()==2);
    // Arbitrary test mapping is deliberately nonuniform: metadata is evidence,
    // not runtime eligibility. Real runtime policy separately rejects this map.
    const auto origin=snapshot.value("points").toArray()[0].toArray();
    assert(std::abs(origin[0].toDouble()-(-23.0/1.5*2+17))<0.0001);
    assert(std::abs(origin[1].toDouble()-(-41.0/1.5*3-9))<0.0001);
    view.setScale(2);
    assert(diagnostic::viewportSnapshot(&engine,&view)!=snapshot);
    view.setScale(1.5);assert(diagnostic::viewportSnapshot(&engine,&view)==snapshot);
    view.setX(24);assert(diagnostic::viewportSnapshot(&engine,&view)!=snapshot);
    view.setX(23);assert(diagnostic::viewportSnapshot(&engine,&view)==snapshot);
    for(int mode:{1,2,3,4}) {
        view.mode=mode;assert(diagnostic::viewportSnapshot(&engine,&view).isEmpty());
    }
    view.mode=0;view.setWidth(0);assert(diagnostic::viewportSnapshot(&engine,&view).isEmpty());
    view.setWidth(834);view.setVisible(false);assert(diagnostic::viewportSnapshot(&engine,&view).isEmpty());
    view.setVisible(true);
    assert(diagnostic::viewportSnapshot(nullptr,&view).isEmpty());
    assert(diagnostic::viewportSnapshot(&engine,nullptr).isEmpty());
    assert(diagnostic::viewportSnapshot(&engine,&parent).isEmpty());
    assert(QQmlEngine::objectOwnership(&view)==ownership);
    std::cout<<"Read-only mapping integration checks passed\n";
}
#include "test-mapping.moc"
