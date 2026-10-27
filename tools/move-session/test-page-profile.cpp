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
    wrong=viewport;wrong["points"]=QJsonArray{QJsonArray{0,0},QJsonArray{103.2,0},QJsonArray{0,103.2}};assert(rejects(profile,wrong));
    wrong=viewport;wrong["points"]=QJsonArray{QJsonArray{0,0},QJsonArray{86,1},QJsonArray{0,86}};assert(rejects(profile,wrong));
    wrong=viewport;wrong["points"]=QJsonArray{QJsonArray{0,0},QJsonArray{86,0}};assert(rejects(profile,wrong));
    wrong=profile;wrong["schema"]=2;assert(rejects(wrong,viewport));
    wrong=profile;wrong.remove("hiddenNativePose");assert(rejects(wrong,viewport));
    wrong=profile;wrong["captureArea"]=QJsonArray{120,112,834,1584};assert(rejects(wrong,viewport));
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
    std::cout<<"PASS reusable profile: translated origins, immutable calibration, zoom/template/geometry/schema rejection; missing/private/symlink/hardlink/malformed/oversized files\n";
}
