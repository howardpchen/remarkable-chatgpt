#pragma once
// Local non-renewing spending guard, NOT a provider/account billing limit.
// Prices verified 2026-10-27: https://developers.openai.com/api/docs/models/gpt-6-luna
// Reserve full maximum input (922000) + 800 output. Rates conservatively include
// long-context, cache-write and regional premiums; cached tokens get no discount.
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QRegularExpression>
#include <sys/file.h>
#include <dirent.h>
#include <cmath>
namespace moveSession {
inline constexpr qint64 budgetTotal=5000000000LL; // integer nano-USD
inline constexpr qint64 inputRate=275, outputRate=825;
inline constexpr qint64 budgetReservation=922000LL*inputRate+800LL*outputRate;
struct BudgetState { bool ok=false; qint64 available=0; int next=0; };
inline bool budgetInteger(QJsonValue v,qint64 maximum,qint64 &out) {
    const double n=v.toDouble(-1);
    if(!v.isDouble() || !std::isfinite(n) || n<0 || n>double(maximum) || std::floor(n)!=n)return false;
    out=qint64(n);return true;
}
inline QByteArray budgetRead(int parent,const QByteArray &name) {
    const int fd=::openat(parent,name.constData(),O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    if(fd<0)return {};
    struct stat st{};QByteArray result;
    if(::fstat(fd,&st)==0 && S_ISREG(st.st_mode) && st.st_uid==::geteuid() &&
       (st.st_mode&0777)==0600 && st.st_nlink==1 && st.st_size>0 && st.st_size<=1024) {
        char data[1025];const auto n=::read(fd,data,sizeof(data));
        if(n==st.st_size)result=QByteArray(data,int(n));
    }
    ::close(fd);return result;
}
// Caller holds an exclusive flock on the private root for scan/reserve/settle.
inline BudgetState budgetScan(int fd) {
    BudgetState state;
    QJsonParseError error;
    const auto doc=QJsonDocument::fromJson(budgetRead(fd,"budget-policy.json"),&error);
    const auto policy=doc.object();qint64 first=0,total=0;
    if(error.error!=QJsonParseError::NoError || policy.value("schema").toInt()!=1 ||
       policy.value("model").toString()!="gpt-6-luna" ||
       !budgetInteger(policy.value("totalNanoUsd"),budgetTotal,total) || total!=budgetTotal ||
       !budgetInteger(policy.value("firstRequestId"),1000000,first) || first<1)return state;
    struct stat stopped{};
    if(::fstatat(fd,"BUDGET_HALTED",&stopped,AT_SYMLINK_NOFOLLOW)==0 || errno!=ENOENT)return state;
    const int scanFd=::openat(fd,".",O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    DIR *dir=scanFd<0?nullptr: ::fdopendir(scanFd);
    if(!dir) {if(scanFd>=0)::close(scanFd);return state;}
    qint64 spent=0;state.next=int(first);
    bool valid=true;int entries=0;
    for(;;) {
        errno=0;const auto *entry=::readdir(dir);
        if(!entry) {if(errno)valid=false;break;}
        if(++entries>1000000){valid=false;break;}
        const QByteArray name(entry->d_name);
        if(name=="." || name=="..")continue;
        if(!QRegularExpression("^[0-9]+$").match(QString::fromLatin1(name)).hasMatch())continue;
        bool parsed=false;const auto id=name.toInt(&parsed);
        if(!parsed || id<1 || id>=1000000 || name!=QByteArray::number(id)){valid=false;break;}
        if(id<first)continue; // Preserved historical test records, separately funded.
        state.next=qMax(state.next,id+1);
        // Missing/malformed/uncertain settlement stays charged at full reservation.
        qint64 charge=budgetReservation;
        const int request=::openat(fd,name.constData(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
        if(request<0){valid=false;break;}
        struct stat requestStat{};
        if(::fstat(request,&requestStat)!=0 || requestStat.st_uid!=::geteuid() || (requestStat.st_mode&0777)!=0700)valid=false;
        const auto raw=budgetRead(request,"BUDGET_COST");
        if(!raw.isEmpty()) {
            bool converted=false;const auto n=raw.trimmed().toLongLong(&converted);
            if(converted && n>=0 && n<=budgetReservation && raw==QByteArray::number(n)+'\n')charge=n;
            else valid=false;
        }
        ::close(request);
        spent+=charge;
        if(!valid || spent>total){valid=false;break;}
    }
    ::closedir(dir);
    if(valid) {state.ok=true;state.available=total-spent;}
    return state;
}
inline BudgetState budgetStatus(const QString &root) {
    int fd=openRoot(root);if(fd<0)return {};
    BudgetState state;
    if(::flock(fd,LOCK_EX)==0)state=budgetScan(fd);
    ::close(fd);return state;
}
inline QString budgetReserve(const QString &root) {
    int fd=openRoot(root);if(fd<0)return {};
    QString result;
    if(::flock(fd,LOCK_EX)==0) {
        const auto state=budgetScan(fd);
        if(state.ok && state.available>=budgetReservation && state.next<1000000) {
            const auto name=QByteArray::number(state.next);
            if(::mkdirat(fd,name.constData(),0700)==0 && ::fsync(fd)==0)result=root+"/"+QString::fromLatin1(name);
        }
    }
    ::close(fd);return result;
}
inline void budgetSettle(const QString &root,const QString &requestPath,const QJsonObject &response) {
    // No estimate-based refund: require bounded, integral provider usage and
    // expected model/tier. Missing/error/cancelled responses keep full reservation.
    if(response.value("model").toString()!="gpt-6-luna")return;
    const auto tier=response.value("service_tier").toString();
    if(!tier.isEmpty() && tier!="default")return;
    const auto usage=response.value("usage").toObject();qint64 input=0,output=0,total=0;
    if(!budgetInteger(usage.value("input_tokens"),922000,input) ||
       !budgetInteger(usage.value("output_tokens"),800,output) ||
       !budgetInteger(usage.value("total_tokens"),922800,total) || total!=input+output || input<1)return;
    if(!requestPath.startsWith(root+"/"))return;
    const auto name=requestPath.mid(root.size()+1).toLatin1();bool parsed=false;
    const int id=name.toInt(&parsed);
    if(!parsed || name!=QByteArray::number(id))return;
    int fd=openRoot(root);if(fd<0)return;
    if(::flock(fd,LOCK_EX)==0) {
        const auto state=budgetScan(fd);
        const auto policy=QJsonDocument::fromJson(budgetRead(fd,"budget-policy.json")).object();
        if(state.ok && id>=policy.value("firstRequestId").toInt() && id<state.next) {
            const int req=::openat(fd,name.constData(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
            if(req>=0) {
                const int cost=::openat(req,"BUDGET_COST",O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
                if(cost>=0) {
                    const auto data=QByteArray::number(input*inputRate+output*outputRate)+'\n';
                    // A partial write is never accepted by the scanner.
                    const bool saved=::write(cost,data.constData(),size_t(data.size()))==data.size() && ::fsync(cost)==0;
                    ::close(cost);
                    if(!saved)::unlinkat(req,"BUDGET_COST",0); // Restore conservative reservation.
                    ::fsync(req);
                }
                ::close(req);
            }
        }
    }
    ::close(fd);
}
}
