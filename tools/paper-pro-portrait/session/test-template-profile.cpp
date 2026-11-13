#define INK_PAPER_PRO_PER_REQUEST
#include "page-profile.h"
#include <cassert>
#include <iostream>
int main() {
    QJsonObject v{{"width",1620},{"height",2160},{"windowWidth",1620},{"windowHeight",2160},
        {"devicePixelRatio",1},{"templateName","Blank"},
        {"points",QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{0,100}}}};
    QJsonObject pose{{"ok",true}};
    QJsonObject profile{{"schema",1},{"product","ferrari"},{"release","3.28.0.172"},{"qt","6.10.3"},
        {"captureArea",QJsonArray{0,0,1620,2160}},{"viewport",v},{"nativePose",pose},{"hiddenNativePose",pose}};
    auto refuses=[](const QJsonObject &p,const QJsonObject &v){try{paperProSession::referenceForStart(p,v);return false;}catch(...){return true;}};
    const auto original=profile;
    for(const auto &name:{"Blank","P Dots S","P Grid small","nonblank","unavailable"}) {
        auto current=v;current["templateName"]=name;
        assert(!refuses(profile,current));assert(profile==original);
    }
    auto current=v;current.remove("templateName");assert(!refuses(profile,current));
    for(const auto &invalid:{QJsonValue(QJsonObject()),QJsonValue("invalid")}) {
        auto bad=profile;bad["templates"]=QJsonObject{{"Blank",invalid}};
        assert(refuses(bad,v));
    }
    for(double zoom:{0.5,2.0,4.0,0.49,4.01}) {
        current=v;current["points"]=QJsonArray{QJsonArray{-50,30},QJsonArray{-50+100/zoom,30},QJsonArray{-50,30+100/zoom}};
        assert(refuses(profile,current)==(zoom<0.5 || zoom>4));
    }
    for(const QJsonArray points:{QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{1,100}},
        QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{0,110}},
        QJsonArray{QJsonArray{0,0},QJsonArray{-100,0},QJsonArray{0,100}}}) {
        current=v;current["points"]=points;assert(refuses(profile,current));
    }
    for(const auto *key:{"viewport","nativePose","hiddenNativePose","captureArea","product","release","qt"}) {
        auto bad=profile;bad.remove(key);assert(refuses(bad,v));
    }
    for(int degrees:{90,270}) {
        auto landscape=v;landscape["width"]=2160;landscape["height"]=1620;
        QTransform r;r.rotate(degrees);const auto bounds=r.mapRect(QRectF(0,0,2160,1620));
        QTransform map(r.m11(),r.m12(),r.m21(),r.m22(),-bounds.x(),-bounds.y());
        QJsonArray corners,points;
        for(const QPointF p:{QPointF(0,0),QPointF(2160,0),QPointF(0,1620),QPointF(2160,1620)}) {auto q=map.map(p);corners.append(QJsonArray{q.x(),q.y()});}
        for(const QPointF p:{QPointF(0,0),QPointF(100,0),QPointF(0,100)}) {auto q=map.inverted().map(p);points.append(QJsonArray{q.x(),q.y()});}
        landscape["points"]=points;landscape["windowCorners"]=corners;
        assert(refuses(profile,landscape));
        auto entry=profile;entry["viewport"]=landscape;
        const QString key=degrees==90?"landscape-cw":"landscape-ccw";
        auto calibrated=profile;calibrated["orientations"]=QJsonObject{{key,entry}};
        current=landscape;current["templateName"]="nonblank";assert(!refuses(calibrated,current));
        // Explicit per-template records preserve their own exact poses; a broken
        // existing record must not silently fall back to the orientation anchor.
        for(const auto &invalid:{QJsonValue(QJsonObject()),QJsonValue("invalid")}) {
            auto direction=entry;direction["templates"]=QJsonObject{{"nonblank",invalid}};
            auto bad=calibrated;bad["orientations"]=QJsonObject{{key,direction}};
            assert(refuses(bad,current));
        }
        auto broken=entry;broken.remove("hiddenNativePose");auto direction=entry;
        direction["templates"]=QJsonObject{{"nonblank",broken}};
        auto bad=calibrated;bad["orientations"]=QJsonObject{{key,direction}};assert(refuses(bad,current));
        bad=profile;bad["orientations"]=QJsonObject{{degrees==90?"landscape-ccw":"landscape-cw",entry}};assert(refuses(bad,current));
        current=landscape;current["devicePixelRatio"]=2;assert(refuses(calibrated,current));
        current=landscape;current.remove("windowCorners");assert(refuses(calibrated,current));
    }
    std::cout<<"PASS Ferrari native-bounds profiles: arbitrary/missing templates, independent CW/CCW, private product/pose/geometry/zoom guards\n";
}
