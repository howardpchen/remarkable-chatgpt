#pragma once
#include <QString>
#include <QFile>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
namespace moveSession {
// Persistent finite quota: 3 explicit attempts / <=2400 output tokens per
// separately provisioned session. A failed/cancelled/uncertain claim stays spent.
// This limits traffic, NOT provider dollar billing. No automatic replenishment.
inline constexpr int maxAttempts=3;
inline int openRoot(const QString &root) {
    int fd=::open(QFile::encodeName(root).constData(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    struct stat st{};
    if(fd>=0 && (::fstat(fd,&st)!=0 || st.st_uid!=::geteuid() || (st.st_mode&0777)!=0700)) {::close(fd);fd=-1;}
    return fd;
}
}
#ifdef INK_MOVE_TOKEN_BUDGET
#include "budget.h"
#endif
namespace moveSession {
inline int remaining(const QString &root) {
#ifdef INK_MOVE_TOKEN_BUDGET
    const auto s=budgetStatus(root);return s.ok?int(s.available/budgetReservation):0;
#else
    int fd=openRoot(root);if(fd<0)return 0;
    int free=0;
    for(int i=1;i<=maxAttempts;++i) {
        struct stat st{};
        if(::fstatat(fd,QByteArray::number(i).constData(),&st,AT_SYMLINK_NOFOLLOW)!=0 && errno==ENOENT)++free;
    }
    ::close(fd);return free;
#endif
}
inline QString reserve(const QString &root) {
#ifdef INK_MOVE_TOKEN_BUDGET
    return budgetReserve(root);
#else
    int fd=openRoot(root);if(fd<0)return {};
    QString path;
    for(int i=1;i<=maxAttempts;++i) {
        auto name=QByteArray::number(i);
        if(::mkdirat(fd,name.constData(),0700)==0) {
            // Claim is durable before it can enable any request. On sync failure
            // leave the directory consumed and fail closed.
            if(::fsync(fd)==0)path=root+"/"+QString::number(i);
            break;
        }
        if(errno!=EEXIST)break;
    }
    ::close(fd);return path;
#endif
}
}
