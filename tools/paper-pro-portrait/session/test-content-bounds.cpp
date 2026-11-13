#include "content-bounds.h"
#include <cassert>
#include <limits>
int main() {
    using diaryContentBounds::read;
    assert(!read(nullptr).value("ok").toBool());
    QObject c;
    assert(!read(&c).value("ok").toBool());
    c.setProperty("itemsBoundingRect",QString("0,0,10,10"));
    assert(!read(&c).value("ok").toBool());
    c.setProperty("itemsBoundingRect",QRectF(-100,800,300,153));
    c.setProperty("rootDocumentBoundingRect",QRectF(0,1200,500,80));
    auto out=read(&c);
    assert(out.value("ok").toBool());
    assert(out.value("rectangles").toObject().size()==2);
    assert(!out.value("coverageVerified").toBool());
    assert(!out.value("nativeInsertionAuthorized").toBool());
    assert(c.property("itemsBoundingRect").toRectF()==QRectF(-100,800,300,153));
    for (auto r : {QRectF(0,0,-1,10),QRectF(0,0,1,-10),
                   QRectF(0,std::numeric_limits<double>::quiet_NaN(),1,1),
                   QRectF(0,0,std::numeric_limits<double>::infinity(),1)}) {
        c.setProperty("itemsBoundingRect",r);
        assert(!read(&c).value("ok").toBool());
    }
    for (auto r : {QRectF(),QRectF(10,900,20,0),QRectF(10,900,0,20)}) {
        c.setProperty("itemsBoundingRect",r);
        assert(read(&c).value("ok").toBool());
        assert(!read(&c).value("coverageVerified").toBool());
    }
    c.setProperty("privateText","synthetic-do-not-export");
    assert(!read(&c).contains("privateText"));
}
