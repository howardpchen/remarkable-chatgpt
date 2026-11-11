#define INK_PAPER_PRO_DOLLAR_BUDGET
#include "quota.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <cassert>
#include <iostream>
static void policy(const QString &root,qint64 total) {
 QFile f(root+"/budget-policy.json");assert(f.open(QIODevice::WriteOnly));
 f.write(QJsonDocument(QJsonObject{{"schema",1},{"model","gpt-6-luna"},{"firstRequestId",1},{"totalNanoUsd",double(total)}}).toJson());f.close();
 assert(f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner));
}
int main(int argc,char **argv) {
 QCoreApplication app(argc,argv);QTemporaryDir dir;assert(dir.isValid());const auto root=dir.path();
 policy(root,100000000);auto first=paperProSession::reserve(root);assert(first==root+"/1");
 paperProSession::budgetSettle(root,first,{{"model","gpt-6-luna"},{"service_tier","default"},{"usage",QJsonObject{{"input_tokens",3160},{"output_tokens",49},{"total_tokens",3209}}}});
 QFile original(first+"/BUDGET_COST");assert(original.open(QIODevice::ReadOnly));const auto bytes=original.readAll();original.close();assert(bytes=="909425\n");
 assert(paperProSession::budgetStatus(root).available==99090575);
 policy(root,5000000000LL);auto s=paperProSession::budgetStatus(root);
 assert(s.ok && s.attempts==1 && s.settled==909425 && s.available==4999090575LL);
 // No request-count/daily quota: more than the earlier three-attempt test.
 for(int i=2;i<=12;++i){const auto run=paperProSession::reserve(root);assert(run==root+"/"+QString::number(i));assert(paperProSession::budgetTransition(root,run,false));}
 s=paperProSession::budgetStatus(root);assert(s.attempts==12 && s.available==4999090575LL);
 assert(original.open(QIODevice::ReadOnly));assert(original.readAll()==bytes);original.close();
 policy(root,5000000001LL);assert(!paperProSession::budgetStatus(root).ok && paperProSession::reserve(root).isEmpty());
 policy(root,5000000000LL);assert(paperProSession::budgetStatus(root).available==4999090575LL);
 std::cout<<"PASS five-dollar policy, preserved prior settlement, no count quota, over-ceiling refusal\n";
}
