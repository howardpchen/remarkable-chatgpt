#pragma once
#include "../../src/capture-layout.h"
#include <QFile>
#include <QJsonDocument>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace moveSession {
// Reusable device calibration, not a question/page approval or a spending grant.
inline QJsonObject loadProfile(const QString &path) {
    const int fd=::open(QFile::encodeName(path).constData(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK);
    if(fd<0)throw std::runtime_error("Device setup profile required");
    struct stat st{};
    if(::fstat(fd,&st)!=0 || !S_ISREG(st.st_mode) || st.st_uid!=::geteuid() ||
       (st.st_mode&0777)!=0600 || st.st_nlink!=1 || st.st_size<1 || st.st_size>1024*1024) {
        ::close(fd);throw std::runtime_error("Device setup profile must be a private regular file (0600)");
    }
    QFile file;
    if(!file.open(fd,QIODevice::ReadOnly,QFileDevice::AutoCloseHandle)) {
        ::close(fd);throw std::runtime_error("Cannot read device setup profile");
    }
    QJsonParseError error;
    const auto doc=QJsonDocument::fromJson(file.read(1024*1024+1),&error);
    if(error.error!=QJsonParseError::NoError || !doc.isObject())throw std::runtime_error("Invalid device setup profile");
    return doc.object();
}
inline QJsonObject referenceForStart(const QJsonObject &profile,const QJsonObject &current) {
    if(profile.value("schema").toInt()!=3 ||
       profile.value("captureArea").toArray()!=QJsonArray{0,0,954,1696} ||
       profile.value("nativePose").toObject().isEmpty() ||
       profile.value("hiddenNativePose").toObject().isEmpty())
        throw std::runtime_error("Device setup profile schema 3 required");
    const auto normal=profile.value("viewport").toObject();
    const auto calibrated=diaryExperimentalMoveMapping(normal,normal);
    const auto live=diaryExperimentalMoveMapping(current,current);
    // Keep the reviewed normal zoom. A page's scene origin may differ, but it
    // cannot redefine the normal scale. Rotation/shear/dimensions reject above.
    if(std::abs(calibrated.sx-live.sx)>1e-9 || std::abs(calibrated.sy-live.sy)>1e-9)
        throw std::runtime_error("Return to normal zoom before Start");
    auto result=profile;
    result.insert("viewport",current); // Frozen for all admission boundaries.
    return result;
}
}
