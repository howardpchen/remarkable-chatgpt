#define INK_MOVE_TOKEN_BUDGET
#include "quota.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <cassert>
#include <sys/wait.h>
#include <iostream>
void policy(const QString &root,int first=1) {
    QFile p(root+"/token-budget-policy.json");assert(p.open(QIODevice::WriteOnly));
    p.write(QJsonDocument(QJsonObject{{"schema",2},{"model","gpt-6-luna"},{"totalCredits",double(moveSession::budgetTotal/2)},{"firstRequestId",first}}).toJson());
    p.close();assert(p.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
}
QJsonObject usage(int in,int out) {
    return {{"model","gpt-6-luna"},{"service_tier","default"},{"usage",QJsonObject{{"input_tokens",in},{"output_tokens",out},{"total_tokens",in+out}}}};
}
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);QTemporaryDir temp;assert(temp.isValid());const auto root=temp.path();
    assert(moveSession::reserve(root).isEmpty());policy(root);
    assert(moveSession::budgetStatus(root).available==moveSession::budgetTotal);
    auto first=moveSession::reserve(root);assert(first==root+"/1");
    assert(moveSession::budgetStatus(root).available==moveSession::budgetTotal-moveSession::budgetReservation);
    moveSession::budgetSettle(root,first,usage(1000,100));
    assert(moveSession::tokenCharge(1000,100)==3000); // 1000 + 5*100 credits
    assert(moveSession::tokenCharge(272000,800)==552000); // threshold inclusive
    assert(moveSession::tokenCharge(272001,800)==1100004); // 2x input / 1.5x output
    assert(moveSession::budgetStatus(root).inputTokens==1000 && moveSession::budgetStatus(root).outputTokens==100);
    auto available=moveSession::budgetTotal-moveSession::tokenCharge(1000,100);
    assert(moveSession::budgetStatus(root).available==available);
    moveSession::budgetSettle(root,first,usage(1,0)); // No repeated refund/overwrite.
    assert(moveSession::budgetStatus(root).available==available);
    const auto second=moveSession::reserve(root);auto invalid=usage(1000,100);
    invalid["model"]="unexpected-model";moveSession::budgetSettle(root,second,invalid);
    assert(moveSession::budgetStatus(root).available==available-moveSession::budgetReservation);
    invalid=usage(1000,100);invalid["usage"]=QJsonObject{{"input_tokens",0.5},{"output_tokens",1},{"total_tokens",1.5}};
    moveSession::budgetSettle(root,second,invalid);assert(!QFile::exists(second+"/TOKEN_USAGE.json"));
    moveSession::budgetSettle(root,second,usage(922001,800));assert(!QFile::exists(second+"/TOKEN_USAGE.json"));
    invalid=usage(1000,100);invalid["service_tier"]="priority";moveSession::budgetSettle(root,second,invalid);assert(!QFile::exists(second+"/TOKEN_USAGE.json"));
    // Repeated low-usage requests continue well beyond three with no count cap.
    for(int i=3;i<=50;++i) {
        const auto path=moveSession::reserve(root);assert(path==root+"/"+QString::number(i));
        moveSession::budgetSettle(root,path,usage(1000,100));
    }
    // Concurrent reservations must never overdraw. Failures/cancellations leave
    // directories charged at full worst case, even without a POST/usage file.
    const int capacity=moveSession::remaining(root);
    for(int i=0;i<32;++i) {
        pid_t pid=::fork();assert(pid>=0);if(!pid)::_exit(moveSession::reserve(root).isEmpty()?0:10);
    }
    int claims=0;
    for(int i=0;i<32;++i) {int status;assert(::wait(&status)>0 && WIFEXITED(status));assert(WEXITSTATUS(status)==0 || WEXITSTATUS(status)==10);claims+=WEXITSTATUS(status)==10;}
    assert(claims==capacity && moveSession::remaining(root)==0 && moveSession::reserve(root).isEmpty());
    assert(moveSession::budgetStatus(root).available<moveSession::budgetReservation);
    assert(QFile::exists(first+"/TOKEN_USAGE.json"));
    QTemporaryDir historic;assert(QDir(historic.path()).mkdir("1") && QDir(historic.path()).mkdir("2"));
    policy(historic.path(),3);assert(moveSession::reserve(historic.path())==historic.path()+"/3");
    assert(moveSession::budgetStatus(historic.path()).available==moveSession::budgetTotal-moveSession::budgetReservation);
    const auto link=root+"-link";assert(::symlink(QFile::encodeName(root).constData(),QFile::encodeName(link).constData())==0);
    assert(moveSession::reserve(link).isEmpty());assert(QFile::remove(link));
    assert(::chmod(QFile::encodeName(historic.path()).constData(),0755)==0);assert(moveSession::reserve(historic.path()).isEmpty());
    assert(::chmod(QFile::encodeName(historic.path()).constData(),0700)==0);
    QFile halted(historic.path()+"/BUDGET_HALTED");assert(halted.open(QIODevice::WriteOnly));halted.close();assert(moveSession::reserve(historic.path()).isEmpty());
    // A symlink/non-directory request is not free capacity.
    QTemporaryDir unsafe;policy(unsafe.path());assert(::symlink("/tmp",QFile::encodeName(unsafe.path()+"/1").constData())==0);
    assert(moveSession::reserve(unsafe.path()).isEmpty());
    std::cout<<"PASS budget: exact costs, missing/invalid usage, no duplicate settlement, >3 requests, concurrent exhaustion, reopen, historical counters and unsafe-state refusal\n";
}
