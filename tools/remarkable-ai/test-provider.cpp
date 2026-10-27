#include "provider-status.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <cassert>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);QTemporaryDir dir;assert(dir.isValid());
    const auto id=remarkableAi::keyId("synthetic-not-real-key");
    remarkableAi::observe(dir.path(),id,429,"insufficient_quota");
    auto value=moveLive::readPrivateConfig(dir.path()+"/provider-status.json");
    assert(value["code"]=="insufficient_quota" && value["keyId"]==id);
    remarkableAi::observe(dir.path(),id,401,"SECRET RAW PROVIDER BODY");
    value=moveLive::readPrivateConfig(dir.path()+"/provider-status.json");assert(value["code"].toString().isEmpty());
    assert(value.keys()==QStringList({"code","httpStatus","keyId","observedAt","schema"}));
    QFile::remove(dir.path()+"/provider-status.json");
    const auto target=dir.path()+"/do-not-touch";QFile file(target);assert(file.open(QIODevice::WriteOnly));file.write("unchanged");file.close();
    assert(QFile::link(target,dir.path()+"/provider-status.json"));
    remarkableAi::observe(dir.path(),id,200,"");assert(file.open(QIODevice::ReadOnly));assert(file.readAll()=="unchanged");
}
