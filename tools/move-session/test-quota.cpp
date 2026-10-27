#include "quota.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <cassert>
#include <sys/wait.h>
#include <iostream>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);QTemporaryDir temp;assert(temp.isValid());
    const auto root=temp.path();const auto path=QFile::encodeName(root);
    assert(::chmod(path.constData(),0700)==0);
    assert(moveSession::remaining(root)==3);
    // Competing claimants cannot obtain more than three identities.
    for(int i=0;i<12;++i) {
        const pid_t child=::fork();assert(child>=0);
        if(!child) ::_exit(moveSession::reserve(root).isEmpty()?0:10);
    }
    int claims=0;
    for(int i=0;i<12;++i) {int status;assert(::wait(&status)>0 && WIFEXITED(status));assert(WEXITSTATUS(status)==0 || WEXITSTATUS(status)==10);claims+=WEXITSTATUS(status)==10;}
    assert(claims==3 && moveSession::remaining(root)==0 && moveSession::reserve(root).isEmpty());
    QTemporaryDir unsafe;assert(unsafe.isValid());
    auto u=QFile::encodeName(unsafe.path());assert(::chmod(u.constData(),0755)==0);
    assert(moveSession::remaining(unsafe.path())==0 && moveSession::reserve(unsafe.path()).isEmpty());
    assert(::chmod(u.constData(),0700)==0);
    assert(::symlink(path.constData(),QFile::encodeName(unsafe.path()+"/1").constData())==0);
    assert(moveSession::remaining(unsafe.path())==2);
    assert(moveSession::reserve(unsafe.path())==unsafe.path()+"/2");
    assert(moveSession::reserve(unsafe.path())==unsafe.path()+"/3");
    assert(moveSession::reserve(unsafe.path()).isEmpty());
    const auto link=root+"-alias";
    assert(::symlink(path.constData(),QFile::encodeName(link).constData())==0);
    assert(moveSession::remaining(link)==0 && moveSession::reserve(link).isEmpty());
    assert(QFile::remove(link));
    std::cout<<"PASS quota: concurrent claims, exhaustion/reopen, private permissions, symlink refusal and occupied-slot preservation\n";
}
