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
    if(frame.reading==QSize(2160,1620)) {
        if((name!="Blank" && name!="P Dots S" && name!="P Grid small") ||
           (frame.degrees!=90 && frame.degrees!=270))
            throw std::runtime_error("Unsupported Ferrari landscape template/orientation");
        const auto key=frame.degrees==90?"landscape-cw":"landscape-ccw";
        const auto orientation=profile.value("orientations").toObject().value(key).toObject();
        // Never inherit Blank or portrait calibration/pitch for landscape patterns.
        const auto entry=name=="Blank"?orientation:orientation.value("templates").toObject().value(name).toObject();
        if(name!="Blank" &&
           (!entry.value("pixelPitch").isDouble() ||
            !std::isfinite(entry.value("pixelPitch").toDouble()) ||
            entry.value("pixelPitch").toDouble()<8 || entry.value("pixelPitch").toDouble()>256))
            throw std::runtime_error("Reviewed Ferrari landscape template pitch required");
        if(entry.value("captureArea")!=QJsonArray{0,0,1620,2160} ||
           !entry.value("nativePose").toObject().value("ok").toBool() ||
           !entry.value("hiddenNativePose").toObject().value("ok").toBool())
            throw std::runtime_error("Separate reviewed Ferrari landscape calibration required");
        if(entry.value("viewport").toObject().value("templateName")!=name)
            throw std::runtime_error("Separate reviewed Ferrari landscape template identity required");
        if(name!="Blank")selected.insert("pixelPitch",entry.value("pixelPitch"));
        for(const auto *field:{"viewport","nativePose","hiddenNativePose","captureArea"})
            selected.insert(field,entry.value(field));
    } else if(name!="Blank") {
        if(name!="P Dots S" && name!="P Grid small")
            throw std::runtime_error("Unsupported Ferrari template");
        // No Blank pose/viewport or Move pitch is inherited for a pattern.
        const auto entry=profile.value("templates").toObject().value(name).toObject();
        if(entry.value("viewport").toObject().value("templateName")!=name ||
           !entry.value("nativePose").toObject().value("ok").toBool() ||
           !entry.value("hiddenNativePose").toObject().value("ok").toBool() ||
           !entry.value("pixelPitch").isDouble() ||
           !std::isfinite(entry.value("pixelPitch").toDouble()) ||
           entry.value("pixelPitch").toDouble()<8 || entry.value("pixelPitch").toDouble()>256)
            throw std::runtime_error("Reviewed Ferrari template calibration required");
        for(const auto *key:{"viewport","nativePose","hiddenNativePose","pixelPitch"})
            selected.insert(key,entry.value(key));
    }
    const auto normal=selected.value("viewport").toObject();
    const auto calibrated=diaryExperimentalPaperProMapping(normal,normal);
    const auto live=diaryExperimentalPaperProMapping(current,current);
    auto invariant=normal;
    invariant.insert("points",current.value("points"));
    if(invariant!=current || diaryPaperProViewportSize(normal)!=frame.reading ||
       diaryPaperProFrame(normal).degrees!=frame.degrees ||
       diaryPaperProReadingAxes(normal).degrees!=0 || diaryPaperProReadingAxes(current).degrees!=0 ||
       diaryPaperProFrame(normal).physical!=frame.physical ||
       !std::isfinite(calibrated.sx/live.sx) ||
       calibrated.sx/live.sx<0.5-1e-9 || calibrated.sx/live.sx>4.0+1e-9)
        throw std::runtime_error("Ferrari requires calibrated orientation and 50% to 400% uniform zoom");
    // Preserve immutable normal calibration; freeze live mapping for this request.
    // Template pitch is measured in screen pixels at normal zoom.
    if(name!="Blank")selected.insert("pixelPitch",selected.value("pixelPitch").toDouble()*calibrated.sx/live.sx);
    selected.insert("viewport",current);
    return selected;
}
}
