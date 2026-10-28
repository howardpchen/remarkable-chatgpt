#include "page-profile.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <cassert>
#include <iostream>
using namespace moveSession;
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    const QJsonObject viewport{{"width",954},{"height",1696},{"windowWidth",954},{"windowHeight",1696},
        {"devicePixelRatio",1},{"templateName","Blank"},{"points",QJsonArray{QJsonArray{0,0},QJsonArray{86,0},QJsonArray{0,86}}}};
    const QJsonObject profile{{"schema",3},{"captureArea",QJsonArray{0,0,954,1696}},{"viewport",viewport},
        {"nativePose",QJsonObject{{"ok",true}}},{"hiddenNativePose",QJsonObject{{"ok",true}}}};
    const auto original=profile;
    auto translated=viewport;translated["points"]=QJsonArray{QJsonArray{-410,137},QJsonArray{-324,137},QJsonArray{-410,223}};
    const auto frozen=referenceForStart(profile,translated);
    assert(frozen["viewport"]==translated && profile==original);
    auto rejects=[&](const QJsonObject &p,const QJsonObject &v){try{referenceForStart(p,v);return false;}catch(const std::exception &){return true;}};
    assert(rejects(profile,{}));
    for(const auto &name:{"P Dots S","P Grid small"}) {
        auto supported=viewport;supported["templateName"]=name;
        assert(referenceForStart(profile,supported)["viewport"]==supported);
    }
    auto wrong=viewport;wrong["templateName"]="Dots";assert(rejects(profile,wrong));
    wrong=viewport;wrong["points"]=QJsonArray{QJsonArray{0,0},QJsonArray{172.1,0},QJsonArray{0,172.1}};assert(rejects(profile,wrong));
    wrong=viewport;wrong["points"]=QJsonArray{QJsonArray{0,0},QJsonArray{86,1},QJsonArray{0,86}};assert(rejects(profile,wrong));
    wrong=viewport;wrong["points"]=QJsonArray{QJsonArray{0,0},QJsonArray{86,0}};assert(rejects(profile,wrong));
    wrong=profile;wrong["schema"]=2;assert(rejects(wrong,viewport));
    wrong=profile;wrong.remove("hiddenNativePose");assert(rejects(wrong,viewport));
    wrong=profile;wrong["captureArea"]=QJsonArray{120,112,834,1584};assert(rejects(wrong,viewport));
    auto landscape=viewport;
    landscape["width"]=1696;landscape["height"]=954;
    landscape["windowWidth"]=1696;landscape["windowHeight"]=954;
    landscape["points"]=QJsonArray{QJsonArray{100,200},QJsonArray{100,286},QJsonArray{14,200}};
    auto landCalibration=profile;landCalibration["viewport"]=landscape;
    landCalibration["captureArea"]=QJsonArray{0,0,1696,954};
    const QJsonObject multi{{"schema",4},{"orientations",QJsonObject{{"portrait",profile},{"landscape",landCalibration}}}};
    assert(referenceForStart(multi,viewport)["viewport"]==viewport);
    assert(referenceForStart(multi,landscape)["viewport"]==landscape);
    assert(rejects(profile,landscape)); // Never infer a landscape normal from portrait.
    auto badMulti=multi;badMulti["orientations"]=QJsonObject{{"portrait",profile}};
    assert(rejects(badMulti,landscape));
    auto badLandscape=landscape;
    badLandscape["points"]=viewport["points"];assert(rejects(multi,badLandscape));
    badLandscape=landscape;badLandscape["devicePixelRatio"]=2;assert(rejects(multi,badLandscape));
    badLandscape=landscape;badLandscape["points"]=QJsonArray{QJsonArray{100,200},QJsonArray{101,286},QJsonArray{14,200}};
    assert(rejects(multi,badLandscape)); // shear
    badLandscape=landscape;badLandscape["points"]=QJsonArray{QJsonArray{100,200},QJsonArray{100,286},QJsonArray{186,200}};
    assert(rejects(multi,badLandscape)); // reflection
    for(const auto &name:{"P Dots S","P Grid small"}) {
        badLandscape=landscape;badLandscape["templateName"]=name;
        assert(rejects(multi,badLandscape));
    }
    // Independent affine fixtures: zoom + pan, both reading orientations and
    // all native quarter turns; never mutate or scale the saved calibration.
    for(bool split:{false,true})for(int turn:{0,90,180,270}) {
        auto normal=viewport;
        if(split) {
            normal["width"]=1696;normal["height"]=954;
            normal["windowCorners"]=QJsonArray{QJsonArray{954,0},QJsonArray{954,1696},QJsonArray{0,0},QJsonArray{0,1696}};
        }
        auto makePoints=[&](double scale) {
            QTransform t;t.rotate(turn);QJsonArray points;
            for(const QPointF p:{QPointF(),QPointF(100,0),QPointF(0,100)}) {
                const auto q=t.map(p*scale)+QPointF(-410,137);
                points.append(QJsonArray{q.x(),q.y()});
            }
            return points;
        };
        normal["points"]=makePoints(0.86);
        auto anchor=profile;anchor["viewport"]=normal;
        const QJsonObject saved{{"schema",4},{"orientations",QJsonObject{{split?"landscape":"portrait",anchor}}}};
        for(double zoom:{0.5,0.75,1.0,1.25,2.0,4.0}) {
            auto live=normal;live["points"]=makePoints(0.86/zoom);
            const auto frozen=referenceForStart(saved,live);
            assert(frozen["viewport"]==live && saved["orientations"].toObject().value(split?"landscape":"portrait")==anchor);
            auto changed=live;changed["points"]=makePoints(0.86/zoom*1.01);
            try {diaryExperimentalMoveMapping(changed,frozen["viewport"].toObject());assert(false);}catch(const std::exception &){}
            changed=live;auto pts=changed["points"].toArray();auto zero=pts[0].toArray();zero[0]=zero[0].toDouble()+1;pts[0]=zero;changed["points"]=pts;
            try {diaryExperimentalMoveMapping(changed,frozen["viewport"].toObject());assert(false);}catch(const std::exception &){}
        }
        for(double zoom:{0.49,4.01}) {
            auto live=normal;live["points"]=makePoints(0.86/zoom);assert(rejects(saved,live));
        }
    }
    std::cout<<"PASS variable zoom: 48 orientation/turn/zoom combinations, pan, immutable anchors, bounds and request freeze\n";
    std::cout<<"PASS orientation profiles: independent normals, quarter turns, missing calibration, DPR, shear/reflection and unmeasured templates rejected\n";
    QTemporaryDir dir;assert(dir.isValid());const auto path=dir.path()+"/device-profile.json";
    auto rejectsFile=[&](const QString &p){try{loadProfile(p);return false;}catch(const std::exception &){return true;}};
    assert(rejectsFile(path));
    QFile file(path);assert(file.open(QIODevice::WriteOnly));file.write(QJsonDocument(profile).toJson());file.close();
    assert(file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));assert(loadProfile(path)==profile);
    assert(QFile::link(path,dir.path()+"/symlink"));assert(rejectsFile(dir.path()+"/symlink"));
    assert(::link(QFile::encodeName(path).constData(),QFile::encodeName(dir.path()+"/hardlink").constData())==0);
    assert(rejectsFile(path));assert(QFile::remove(dir.path()+"/hardlink"));
    assert(file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadOther));assert(rejectsFile(path));
    assert(file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
    assert(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write("invalid-json");file.close();assert(rejectsFile(path));
    assert(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write(QByteArray(1024*1024+1,'x'));file.close();assert(rejectsFile(path));
    std::cout<<"PASS reusable profile: translated origins, immutable calibration, zoom bounds/template/geometry/schema rejection; missing/private/symlink/hardlink/malformed/oversized files\n";
}
