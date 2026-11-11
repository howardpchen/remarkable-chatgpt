#define INK_PAPER_PRO_PER_REQUEST
#include "page-profile.h"
#include <cassert>
#include <iostream>
#include <limits>

int main() {
    const QJsonObject viewport{{"width",1620},{"height",2160},{"windowWidth",1620},{"windowHeight",2160},
        {"devicePixelRatio",1},{"templateName","Blank"},
        {"points",QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{0,100}}}};
    const QJsonObject pose{{"ok",true}};
    QJsonObject profile{{"schema",1},{"product","ferrari"},{"release","3.28.0.172"},
        {"qt","6.10.3"},{"captureArea",QJsonArray{0,0,1620,2160}},
        {"viewport",viewport},{"nativePose",pose},{"hiddenNativePose",pose},
        {"files",QJsonObject{{"synthetic-file","synthetic-hash"}}}};
    assert(paperProSession::referenceForStart(profile,viewport)==profile);
    auto refuses=[](const QJsonObject &p,const QJsonObject &v) {
        try {paperProSession::referenceForStart(p,v);return false;}
        catch(const std::exception &) {return true;}
    };
    for(const auto &name:{QString("P Dots S"),QString("P Grid small")}) {
        auto current=viewport;current.insert("templateName",name);
        assert(refuses(profile,current)); // Blank calibration never grants patterns.
        const QJsonObject entry{{"viewport",current},{"nativePose",pose},
            {"hiddenNativePose",pose},{"pixelPitch",47.25}};
        auto calibrated=profile;
        calibrated.insert("templates",QJsonObject{{name,entry}});
        const auto selected=paperProSession::referenceForStart(calibrated,current);
        assert(selected["viewport"]==current && selected["pixelPitch"]==47.25);
        assert(selected["files"]==profile["files"] && calibrated["viewport"]==viewport);
        for(const auto *key:{"viewport","nativePose","hiddenNativePose","pixelPitch"}) {
            auto bad=entry;bad.remove(key);
            auto p=calibrated;p.insert("templates",QJsonObject{{name,bad}});
            assert(refuses(p,current));
        }
        for(double pitch:{0.0,7.99,256.01,std::numeric_limits<double>::infinity()}) {
            auto bad=entry;bad.insert("pixelPitch",pitch);
            auto p=calibrated;p.insert("templates",QJsonObject{{name,bad}});
            assert(refuses(p,current));
        }
        auto changed=current;changed.insert("points",QJsonArray{QJsonArray{10,0},QJsonArray{110,0},QJsonArray{10,100}});
        assert(paperProSession::referenceForStart(calibrated,changed)["viewport"]==changed);
        for(double zoom:{0.5,2.0,4.0,0.49,4.01}) {
            changed=current;changed.insert("points",QJsonArray{QJsonArray{-50,30},QJsonArray{-50+100/zoom,30},QJsonArray{-50,30+100/zoom}});
            if(zoom<0.5 || zoom>4)assert(refuses(calibrated,changed));
            else {
                const auto live=paperProSession::referenceForStart(calibrated,changed);
                assert(live["viewport"]==changed);
                assert(std::abs(live["pixelPitch"].toDouble()-47.25*zoom)<1e-8);
                assert(calibrated["templates"].toObject()[name].toObject()["viewport"]==current);
            }
        }
        changed=current;changed.insert("templateName","P Grid medium");
        assert(refuses(calibrated,changed));
        assert(paperProSession::referenceForStart(calibrated,viewport)["viewport"]==viewport);
    }
    for(const QJsonArray badPoints:{
        QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{1,100}},
        QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{0,110}},
        QJsonArray{QJsonArray{0,0},QJsonArray{-100,0},QJsonArray{0,100}}}) {
        auto bad=viewport;bad.insert("points",badPoints);assert(refuses(profile,bad));
    }
    for(int degrees:{90,270}) {
        auto landscape=viewport;landscape.insert("width",2160);landscape.insert("height",1620);
        QTransform r;r.rotate(degrees);
        const auto bounds=r.mapRect(QRectF(0,0,2160,1620));
        const QTransform map(r.m11(),r.m12(),r.m21(),r.m22(),-bounds.x(),-bounds.y());
        QJsonArray corners,points;
        for(const QPointF p:{QPointF(0,0),QPointF(2160,0),QPointF(0,1620),QPointF(2160,1620)}) {
            const auto q=map.map(p);corners.append(QJsonArray{q.x(),q.y()});
        }
        for(const QPointF p:{QPointF(0,0),QPointF(100,0),QPointF(0,100)}) {
            const auto q=map.inverted().map(p);points.append(QJsonArray{q.x(),q.y()});
        }
        landscape.insert("windowCorners",corners);landscape.insert("points",points);
        assert(refuses(profile,landscape));
        const QJsonObject entry{{"viewport",landscape},{"captureArea",QJsonArray{0,0,1620,2160}},
            {"nativePose",pose},{"hiddenNativePose",pose}};
        const QString key=degrees==90?"landscape-cw":"landscape-ccw";
        auto calibrated=profile;calibrated.insert("orientations",QJsonObject{{key,entry}});
        assert(paperProSession::referenceForStart(calibrated,landscape)["viewport"]==landscape);
        for(double zoom:{0.5,2.0,4.0,0.49,4.01}) {
            auto changed=landscape;QJsonArray transformed;
            for(const auto &value:points) {
                const auto p=value.toArray();
                transformed.append(QJsonArray{p[0].toDouble()/zoom-217,p[1].toDouble()/zoom+91});
            }
            changed.insert("points",transformed);
            if(zoom<0.5 || zoom>4)assert(refuses(calibrated,changed));
            else assert(paperProSession::referenceForStart(calibrated,changed)["viewport"]==changed);
            assert(calibrated["orientations"].toObject()[key].toObject()["viewport"]==landscape);
        }
        calibrated.insert("orientations",QJsonObject{{degrees==90?"landscape-ccw":"landscape-cw",entry}});
        assert(refuses(calibrated,landscape)); // Never derive the opposite pose.
        calibrated.insert("orientations",QJsonObject{{key,entry}});
        for(const auto &name:{"P Dots S","P Grid small"}) {
            auto changed=landscape;changed.insert("templateName",name);assert(refuses(calibrated,changed));
            auto patternEntry=entry;patternEntry.insert("viewport",changed);patternEntry.insert("pixelPitch",57.5);
            auto directionEntry=entry;directionEntry.insert("templates",QJsonObject{{name,patternEntry}});
            auto patterns=calibrated;patterns.insert("orientations",QJsonObject{{key,directionEntry}});
            assert(paperProSession::referenceForStart(patterns,changed)["pixelPitch"]==57.5);
            // Each pattern/direction owns its poses, full physical crop and pitch.
            for(const auto *field:{"viewport","captureArea","nativePose","hiddenNativePose","pixelPitch"}) {
                auto bad=patternEntry;bad.remove(field);auto e=directionEntry;e.insert("templates",QJsonObject{{name,bad}});
                auto broken=patterns;broken.insert("orientations",QJsonObject{{key,e}});assert(refuses(broken,changed));
            }
            for(double pitch:{7.99,256.01,std::numeric_limits<double>::infinity()}) {
                auto bad=patternEntry;bad.insert("pixelPitch",pitch);auto e=directionEntry;e.insert("templates",QJsonObject{{name,bad}});
                auto broken=patterns;broken.insert("orientations",QJsonObject{{key,e}});assert(refuses(broken,changed));
            }
            for(double zoom:{0.5,2.0,4.0,0.49,4.01}) {
                auto live=changed;QJsonArray transformed;
                for(const auto &value:points) {const auto xy=value.toArray();transformed.append(QJsonArray{xy[0].toDouble()/zoom-125,xy[1].toDouble()/zoom+239});}
                live.insert("points",transformed);
                if(zoom<0.5 || zoom>4)assert(refuses(patterns,live));
                else {
                    const auto result=paperProSession::referenceForStart(patterns,live);
                    assert(result["viewport"]==live && std::abs(result["pixelPitch"].toDouble()-57.5*zoom)<1e-8);
                    assert(result["files"]==profile["files"]);
                }
            }
            assert(patterns["orientations"].toObject()[key].toObject()["templates"].toObject()[name].toObject()["viewport"]==changed);
            auto wrongDirection=patterns;wrongDirection.insert("orientations",QJsonObject{{degrees==90?"landscape-ccw":"landscape-cw",directionEntry}});
            assert(refuses(wrongDirection,changed));
            auto other=changed;other.insert("templateName",QString(name)=="P Grid small"?"P Dots S":"P Grid small");assert(refuses(patterns,other));
        }
        auto changed=landscape;changed.remove("windowCorners");assert(refuses(calibrated,changed));
        changed=landscape;changed.insert("devicePixelRatio",2);assert(refuses(calibrated,changed));
    }
    std::cout<<"PASS Ferrari profiles: separate pattern/CW/CCW calibration, bounded zoom/pan, required poses/pitch, unchanged Blank/files, independent landscape pattern admission/refusal\n";
}
