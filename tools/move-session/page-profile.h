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
    const auto size=diaryMoveViewportSize(current);
    auto selected=profile;
    const int schema=profile.value("schema").toInt();
    if(schema==4) {
        const auto key=size.width()>size.height()?QStringLiteral("landscape"):QStringLiteral("portrait");
        const auto orientation=profile.value("orientations").toObject().value(key).toObject();
        if(orientation.isEmpty())throw std::runtime_error("Orientation calibration required");
        for(const auto &field:{"viewport","captureArea","nativePose","hiddenNativePose"})
            selected.insert(field,orientation.value(field));
    } else if(schema!=3 || size!=QSize(954,1696)) {
        throw std::runtime_error("Orientation-specific device setup profile required");
    }
    if(selected.value("captureArea").toArray()!=QJsonArray{0,0,diaryMoveFrame(current).physical.width(),diaryMoveFrame(current).physical.height()} ||
       selected.value("nativePose").toObject().isEmpty() ||
       selected.value("hiddenNativePose").toObject().isEmpty())
        throw std::runtime_error("Reviewed orientation capture and native poses required");
    const auto normal=selected.value("viewport").toObject();
    const auto calibrated=diaryExperimentalMoveMapping(normal,normal);
    const auto live=diaryExperimentalMoveMapping(current,current);
    // The normal remains an immutable orientation anchor. Uniform zoom and pan
    // may vary at Start; the entire live transform is frozen for this request.
    // viewToScene gives scene units per screen pixel, so zoom is inverse scale.
    const double zoom=calibrated.sx/live.sx;
    if(diaryMoveViewportSize(normal)!=size || diaryMoveFrame(normal).physical!=diaryMoveFrame(current).physical ||
       diaryMoveFrame(normal).degrees!=diaryMoveFrame(current).degrees ||
       diaryMoveAxes(normal.value("points").toArray()).degrees!=diaryMoveAxes(current.value("points").toArray()).degrees ||
       !std::isfinite(zoom) || zoom<0.5-1e-9 || zoom>4.0+1e-9)
        throw std::runtime_error("Supported zoom is 50% to 400%; calibrated orientation required");
    auto result=selected;
    result.insert("viewport",current); // Frozen for all admission boundaries.
    return result;
}
}
