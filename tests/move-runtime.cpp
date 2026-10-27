// Local geometry/policy tests only. No tablet, API, config or credentials.
#include <cassert>
#include <iostream>
#include <limits>
#include "../src/capture-layout.h"

static QJsonObject normal(qreal scale=0.85984848) {
    return {{"templateName","Blank"},{"windowWidth",954},{"windowHeight",1696},
        {"devicePixelRatio",1},{"width",954},{"height",1696},
        {"points",QJsonArray{QJsonArray{10,20},QJsonArray{10+100*scale,20},QJsonArray{10,20+100*scale}}}};
}
template<class F> static void rejects(F f) {
    bool rejected=false;
    try {f();} catch(const std::runtime_error &) {rejected=true;}
    assert(rejected);
}
int main() {
    const QString opt="chiappa/3.28.0.172/qt6.10.3/blank-visible-v1";
    const QByteArray os="NAME=reMarkable\nIMG_VERSION=\"3.28.0.172\"\n";
    auto tuple=[&](QByteArray machine,QByteArray version,QByteArray qt,QString flag) {
        return diaryExperimentalMoveTuple(machine,version,qt,flag);
    };
    assert(tuple("reMarkable Chiappa\n",os,"6.10.3",opt));
    assert(!tuple("reMarkable Chiappa",os,"6.10.3",""));
    assert(!tuple("reMarkable Ferrari",os,"6.10.3",opt));
    assert(!tuple("reMarkable Chiappa","IMG_VERSION=3.28.0.171\n","6.10.3",opt));
    assert(!tuple("reMarkable Chiappa",os,"6.10.4",opt));
    assert(!tuple("reMarkable Chiappa",os+"IMG_VERSION=3.28.0.172\n","6.10.3",opt));
    assert(!tuple("reMarkable Chiappa","VERSION_ID=3.28.0.172\n","6.10.3",opt));
    const auto layout=diaryExperimentalMoveLayout({954,1696},{954,1696},1);
    assert(layout.area==QRect(120,112,834,1584));
    rejects([]{diaryExperimentalMoveLayout({954,1696},{954,1696},2);});
    rejects([]{diaryExperimentalMoveLayout({1696,954},{1696,954},1);});
    const auto viewport=normal();
    assert(std::abs(diaryExperimentalMoveMapping(viewport,viewport).sx-0.85984848)<1e-9);
    // Valid metadata must not promote the old UI-contaminated crop.
    bool uiBlocked=false;
    try {diaryRequireExperimentalMoveCaptureReady(viewport,viewport);}
    catch(const std::runtime_error &e) {uiBlocked=std::string(e.what())=="Move capture UI exclusion is not validated";}
    assert(uiBlocked);
    rejects([&]{diaryExperimentalMoveMapping(viewport,{});});
    auto missing=viewport;missing.remove("templateName");
    rejects([&]{diaryExperimentalMoveMapping(missing,missing);});
    auto dotted=viewport;dotted["templateName"]="Dots";
    rejects([&]{diaryExperimentalMoveMapping(dotted,dotted);});
    for(const QJsonArray points : {
            QJsonArray{QJsonArray{0,0},QJsonArray{-85,0},QJsonArray{0,85}}, // Reflection.
            QJsonArray{QJsonArray{0,0},QJsonArray{0,85},QJsonArray{-85,0}}, // Rotation.
            QJsonArray{QJsonArray{0,0},QJsonArray{85,1},QJsonArray{0,85}}, // Shear.
            QJsonArray{QJsonArray{0,0},QJsonArray{85,0},QJsonArray{0,84}}, // Nonuniform.
            QJsonArray{QJsonArray{0,0},QJsonArray{0,0},QJsonArray{0,0}}, // Degenerate.
            QJsonArray{QJsonArray{0,0},QJsonArray{"bad",0},QJsonArray{0,85}},
            QJsonArray{QJsonArray{0,0},QJsonArray{85,0}}}) {
        auto invalid=viewport;invalid["points"]=points;
        rejects([&]{diaryExperimentalMoveMapping(invalid,invalid);});
    }
    // A valid but changed zoom/pan must fail the independently measured reference.
    rejects([&]{diaryExperimentalMoveMapping(normal(1),viewport);});
    auto shifted=viewport;shifted["points"]=QJsonArray{QJsonArray{11,20},QJsonArray{97,20},QJsonArray{11,106}};
    rejects([&]{diaryExperimentalMoveMapping(shifted,viewport);});
    QImage rendered(834,397,QImage::Format_ARGB32);rendered.fill(Qt::white);
    auto result=diaryExperimentalMovePlacement(rendered,834,viewport,viewport);
    assert(result.nativeImage.size()==QSize(717,341));
    assert(result.screenBounds.center().x()==537);
    assert(result.screenBounds.top()==834);
    assert(QRectF(120,112,834,1584).contains(result.screenBounds));
    assert(std::abs(result.screenBounds.width()-717/0.85984848)<1e-9);
    assert(result.nativeImage.width()!=rendered.width()); // Native pixels are scene units.
    auto twice=normal(2);
    assert(diaryExperimentalMovePlacement(rendered,834,twice,twice).nativeImage.size()==QSize(1668,794));
    rejects([&]{diaryExperimentalMovePlacement(rendered,111,viewport,viewport);});
    rejects([&]{diaryExperimentalMovePlacement(rendered,1300,viewport,viewport);});
    rejects([&]{diaryExperimentalMovePlacement(rendered,std::numeric_limits<int>::max(),viewport,viewport);});
    rejects([&]{diaryExperimentalMovePlacement(QImage(835,397,QImage::Format_RGB32),834,viewport,viewport);});
    rejects([&]{diaryExperimentalMovePlacement({},834,viewport,viewport);});
    // Exactly full visible content fits; one pixel below must fail before rounding.
    QImage full(834,1584,QImage::Format_RGB32);full.fill(Qt::white);
    assert(!diaryExperimentalMovePlacement(full,112,viewport,viewport).nativeImage.isNull());
    rejects([&]{diaryExperimentalMovePlacement(full,113,viewport,viewport);});
    std::cout<<"PASS: exact Move tuple opt-in, trusted template/reference, transform rejection, native scene scaling, visible-only bounds\n";
}
