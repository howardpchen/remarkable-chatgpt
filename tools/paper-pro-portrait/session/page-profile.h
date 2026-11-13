#pragma once
#include "capture-layout.h"
#include <QFile>
#include <QJsonDocument>
#include <QSet>
#include <QStringList>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace paperProSession {
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
// Ferrari profile never accepts a Move schema, inferred native pose or firmware.
inline QJsonObject referenceForStart(const QJsonObject &profile,const QJsonObject &current) {
    if(profile.value("schema").toInt()!=1 || profile.value("product")!="ferrari" ||
       profile.value("release")!="3.28.0.172" || profile.value("qt")!="6.10.3" ||
       profile.value("captureArea").toArray()!=QJsonArray{0,0,1620,2160} ||
       !profile.value("nativePose").toObject().value("ok").toBool() ||
       !profile.value("hiddenNativePose").toObject().value("ok").toBool())
        throw std::runtime_error("Reviewed Ferrari portrait profile required");
    auto selected=profile;
    const auto name=current.value("templateName").toString();
    const auto frame=diaryPaperProFrame(current);
    QJsonObject entry;
    if(frame.reading==QSize(2160,1620)) {
        if(frame.degrees!=90 && frame.degrees!=270)
            throw std::runtime_error("Unsupported Ferrari landscape orientation");
        const auto key=frame.degrees==90?"landscape-cw":"landscape-ccw";
        const auto orientation=profile.value("orientations").toObject().value(key).toObject();
        // Each Ferrari turn still requires its own reviewed physical map/poses.
        // Existing template-specific poses are honored, but pixel pitch is no
        // longer needed and absence of a template record uses this SAME Ferrari
        // orientation anchor, never Move data or an inferred opposite turn.
        const auto templates=orientation.value("templates").toObject();
        if(templates.contains(name)) {
            entry=templates.value(name).toObject();
            if(entry.isEmpty())throw std::runtime_error("Invalid explicit Ferrari template pose record");
        } else entry=orientation;
        if(entry.value("captureArea")!=QJsonArray{0,0,1620,2160})
            throw std::runtime_error("Separate reviewed Ferrari landscape calibration required");
    } else {
        const auto templates=profile.value("templates").toObject();
        if(templates.contains(name)) {
            entry=templates.value(name).toObject();
            if(entry.isEmpty())throw std::runtime_error("Invalid explicit Ferrari template pose record");
        } else entry=profile;
    }
    if(entry.value("viewport").toObject().isEmpty() ||
       !entry.value("nativePose").toObject().value("ok").toBool() ||
       !entry.value("hiddenNativePose").toObject().value("ok").toBool())
        throw std::runtime_error("Reviewed Ferrari orientation viewport and poses required");
    for(const auto *field:{"viewport","nativePose","hiddenNativePose"})
        selected.insert(field,entry.value(field));
    const auto normal=selected.value("viewport").toObject();
    const auto calibrated=diaryExperimentalPaperProMapping(normal,normal);
    const auto live=diaryExperimentalPaperProMapping(current,current);
    auto invariant=normal;
    invariant.insert("points",current.value("points"));
    // Artwork identity is optional telemetry, not part of numeric calibration.
    invariant.remove("templateName");
    auto numericCurrent=current;numericCurrent.remove("templateName");
    if(invariant!=numericCurrent || diaryPaperProViewportSize(normal)!=frame.reading ||
       diaryPaperProFrame(normal).degrees!=frame.degrees ||
       diaryPaperProReadingAxes(normal).degrees!=0 || diaryPaperProReadingAxes(current).degrees!=0 ||
       diaryPaperProFrame(normal).physical!=frame.physical ||
       !std::isfinite(calibrated.sx/live.sx) ||
       calibrated.sx/live.sx<0.5-1e-9 || calibrated.sx/live.sx>4.0+1e-9)
        throw std::runtime_error("Ferrari requires calibrated orientation and 50% to 400% uniform zoom");
    // Preserve immutable Ferrari calibration; freeze this live mapping.
    selected.insert("viewport",current);
    return selected;
}
}
