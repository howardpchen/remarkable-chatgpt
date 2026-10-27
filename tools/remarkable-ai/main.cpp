// Local-only administration. Deliberately links QtCore, never QtNetwork.
#define INK_CONFIGURE_LIBRARY
#include "../move-live/configure.cpp"
#define INK_MOVE_DOLLAR_BUDGET
#include "../move-session/quota.h"
#include "provider-status.h"
#include <QTextStream>
#include <QJsonArray>

namespace {
QJsonObject status(const QString &configPath,const QString &root) {
    QJsonObject out{{"schema",1},{"networkCheck",false},{"providerAccountBalance","unknown"}};
    QJsonObject key{{"state","missing"}};QString fingerprint;
    if(QFileInfo::exists(configPath) || QFileInfo(configPath).isSymLink()) {
        try {
            requirePrivateParent(configPath);
            const auto config=moveLive::readPrivateConfig(configPath);
            if(config.contains("api_key")) {
                const auto raw=moveLive::loadKey(configPath);fingerprint=remarkableAi::keyId(raw);
                key={{"state","configured"},{"identifier","sha256:"+fingerprint},{"verified",false}};
            }
        } catch(...) {key={{"state","invalid-or-unsafe"}};}
    }
    out.insert("key",key);
    QJsonObject allowance{{"scope","application-local conservative allowance; not OpenAI account credit"},
        {"state","unavailable-or-unprovisioned"},{"canAffordNextRequest",false},{"requestCountCap",QJsonValue::Null}};
    const bool safeRoot=QDir(root).canonicalPath()==QDir(root).absolutePath();
    if(safeRoot) {
        const auto budget=moveSession::budgetStatus(root);
        if(budget.ok) {
            allowance.insert("state",budget.available>=moveSession::budgetReservation?"ready":"exhausted");
            allowance.insert("totalUsd",double(moveSession::budgetTotal)/1e9);
            allowance.insert("availableUsd",double(budget.available)/1e9);
            allowance.insert("chargedIncludingReservationsUsd",double(budget.charged)/1e9);
            allowance.insert("unsettledReservedUsd",double(budget.unsettledReserved)/1e9);
            allowance.insert("unsettledAttempts",budget.unsettledAttempts);
            allowance.insert("attempts",budget.attempts);
            allowance.insert("nextRequestReservationUsd",double(moveSession::budgetReservation)/1e9);
            allowance.insert("canAffordNextRequest",budget.available>=moveSession::budgetReservation);
        }
    }
    out.insert("allowance",allowance);
    QJsonObject provider{{"state","not-observed"},{"freshVerification",false}};
    if(safeRoot && QFileInfo::exists(root+"/provider-status.json")) {
        try {
            auto observed=moveLive::readPrivateConfig(root+"/provider-status.json");
            const auto stamp=QDateTime::fromString(observed.value("observedAt").toString(),Qt::ISODate);
            const auto http=observed.value("httpStatus").toInt(-1);
            const auto code=observed.value("code").toString();
            if(observed.value("schema").toInt()!=1 || !stamp.isValid() || http<0 || http>599 ||
               (!code.isEmpty() && remarkableAi::allowedCode(code)!=code))throw std::runtime_error("invalid metadata");
            if(fingerprint.isEmpty() || observed.value("keyId").toString()!=fingerprint)
                provider.insert("state","observation-for-different-key");
            else {
                provider.insert("state","last-observed");provider.insert("httpStatus",http);
                provider.insert("code",code);provider.insert("observedAt",stamp.toUTC().toString(Qt::ISODate));
            }
        } catch(...) {provider.insert("state","observation-unavailable");}
    }
    out.insert("provider",provider);
    QStringList actions;
    if(key.value("state")!="configured")actions<<"Run remarkable-ai key set to configure a key.";
    if(allowance.value("state")=="exhausted")actions<<"Local allowance exhausted; replacing the key does not refill it.";
    if(allowance.value("state")=="unavailable-or-unprovisioned")actions<<"Local allowance unavailable: complete reviewed device/budget setup; do not reset usage records.";
    if(provider.value("state")=="last-observed") {
        const auto code=provider.value("code").toString();const int http=provider.value("httpStatus").toInt();
        if(code=="insufficient_quota")actions<<"Last observed provider quota exhausted: check OpenAI project billing/limits; replacing the key alone may not help.";
        else if(http==401 || code=="invalid_api_key")actions<<"Last observed authentication failure: review or replace the OpenAI key.";
        else if(http==429 || code=="rate_limit_exceeded")actions<<"Last observed rate limit: wait or check provider limits; no automatic retry.";
        else if(http==403)actions<<"Last observed access denial: check OpenAI project/model permissions.";
        else if(http!=200)actions<<"Last provider request failed; this is historical metadata, not a current key verification.";
    }
    if(actions.isEmpty())actions<<"No local key/budget issue detected; OpenAI account balance and current validity are not checked.";
    out.insert("guidance",QJsonArray::fromStringList(actions));return out;
}
void print(const QJsonObject &value,bool json) {
    if(json) {std::cout<<QJsonDocument(value).toJson(QJsonDocument::Indented).constData();return;}
    QTextStream stream(stdout);const auto key=value.value("key").toObject();
    stream<<"Key: "<<key.value("state").toString()<<" "<<key.value("identifier").toString()<<"\n";
    const auto budget=value.value("allowance").toObject();stream<<"Local allowance: "<<budget.value("state").toString()<<"\n";
    if(budget.contains("availableUsd")) {
        stream.setRealNumberNotation(QTextStream::FixedNotation);stream.setRealNumberPrecision(6);
        stream<<"  Available $"<<budget.value("availableUsd").toDouble()<<" / $"<<budget.value("totalUsd").toDouble()
          <<"; charged incl. reservations $"<<budget.value("chargedIncludingReservationsUsd").toDouble()
          <<"\n  Unsettled/reserved $"<<budget.value("unsettledReservedUsd").toDouble()
          <<"; next attempt reserves $"<<budget.value("nextRequestReservationUsd").toDouble()
          <<"\n  Next request affordable: "<<(budget.value("canAffordNextRequest").toBool()?"yes":"no")<<"; no separate request-count cap\n";
    }
    const auto provider=value.value("provider").toObject();stream<<"Provider: "<<provider.value("state").toString();
    if(provider.contains("httpStatus"))stream<<" at "<<provider.value("observedAt").toString()<<"; HTTP "<<provider.value("httpStatus").toInt()<<" "<<provider.value("code").toString();
    stream<<"\nOpenAI account balance: unknown. No API call made.\n";
    for(const auto action:value.value("guidance").toArray())stream<<"- "<<action.toString()<<"\n";
}
}
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);auto args=app.arguments();
    rlimit noCore{0,0};::setrlimit(RLIMIT_CORE,&noCore);::umask(077);
    if(args.size()==2 && (args[1]=="--help" || args[1]=="help")) {
        std::cout<<"remarkable-ai key status [--json] [--config PATH] [--state-dir PATH]\nremarkable-ai key set|remove [--config PATH]\nStatus is local-only; key changes never refill the allowance.\n";return 0;
    }
    if(args.size()<3 || args[1]!="key" || !QStringList{"status","set","remove"}.contains(args[2]))return 2;
    QString config="/home/root/.config/ink-diary/config.json",root="/home/root/xovi/move-session-small-templates-v4";bool json=false;
    for(int i=3;i<args.size();++i) {
        if(args[i]=="--json" && args[2]=="status")json=true;
        else if(args[i]=="--config" && i+1<args.size())config=args[++i];
        else if(args[i]=="--state-dir" && args[2]=="status" && i+1<args.size())root=args[++i];
        else return 2;
    }
    if(!QDir::isAbsolutePath(config) || !QDir::isAbsolutePath(root))return 2;
    if(args[2]=="status") {print(status(config,root),json);return 0;}
    // Create only an owned private config directory, never create a budget or runs.
    const auto parent=QFileInfo(config).absolutePath();
    if(!QFileInfo::exists(parent) && args[2]=="set") {
        const auto ancestor=QFileInfo(parent).absolutePath();
        if(QDir(ancestor).canonicalPath()!=ancestor || ::mkdir(QFile::encodeName(parent).constData(),0700)!=0) {
            std::cerr<<"Create an owned private configuration parent first.\n";return 1;
        }
    }
    return runConfigure({args[0],args[2]=="set"?"--configure":"--remove","--config",config});
}
