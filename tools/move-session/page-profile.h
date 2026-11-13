#pragma once
#include "../../src/capture-layout.h"
#include <QFile>
#include <QJsonDocument>
#include <QSet>
#include <QStringList>
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
// On the exact supported Move firmware, opposite landscape is a physical
// half-turn. Derive the expected pose from the REVIEWED pose, never from live
// telemetry. Preserve every state/property and the full item sequence. Only the
// three native window roots change local rotation; descendant local rotations
// remain unchanged while their physical rectangles rotate with their parents.
inline QJsonObject oppositeLandscapePose(QJsonObject pose,const DiaryMoveFrame &frame) {
    if(!pose.value("ok").toBool() || !pose.value("items").isArray())
        throw std::runtime_error("Reviewed landscape native pose required");
    QJsonArray items;
    QSet<QString> roots;
    const QStringList windowRoots={"LegacyMainWindow","GesturesWindow","ShortcutsWindow"};
    for(const auto &value:pose.value("items").toArray()) {
        auto item=value.toObject();const auto rect=item.value("rect").toArray();
        if(rect.size()!=4)throw std::runtime_error("Invalid reviewed landscape rectangle");
        for(const auto &n:rect)if(!n.isDouble() || !std::isfinite(n.toDouble()))
            throw std::runtime_error("Invalid reviewed landscape rectangle");
        const double x=rect[0].toDouble(),y=rect[1].toDouble(),w=rect[2].toDouble(),h=rect[3].toDouble();
        if(w<0 || h<0)throw std::runtime_error("Invalid reviewed landscape rectangle");
        item.insert("rect",QJsonArray{frame.physical.width()-x-w,frame.physical.height()-y-h,w,h});
        const auto type=item.value("type").toString();
        if(windowRoots.contains(type)) {
            if(roots.contains(type) || item.value("rotation")!=QJsonValue(frame.degrees) ||
               rect!=QJsonArray{0,0,frame.physical.width(),frame.physical.height()})
                throw std::runtime_error("Unsupported reviewed landscape window root");
            roots.insert(type);item.insert("rotation",(frame.degrees+180)%360);
        } else if(item.value("rotation")!=QJsonValue(0)) {
            throw std::runtime_error("Unsupported reviewed landscape local rotation");
        }
        items.append(item);
    }
    if(roots.size()!=windowRoots.size())throw std::runtime_error("Reviewed landscape window roots required");
    pose.insert("items",items);return pose;
}
inline QJsonObject oppositeLandscapeReference(QJsonObject selected,const QJsonObject &current) {
    auto normal=selected.value("viewport").toObject();
    const auto base=diaryMoveFrame(normal),live=diaryMoveFrame(current);
    // Do not infer arbitrary rotations or portrait support. Template artwork
    // is irrelevant to this native-bounds placement calibration.
    // Both exact four-corner maps and the composed reading axes must agree.
    if(base.reading!=QSize(1696,954) || base.physical!=QSize(954,1696) ||
       live.reading!=base.reading || live.physical!=base.physical ||
       (base.degrees!=90 && base.degrees!=270) || (live.degrees+180)%360!=base.degrees ||
       diaryMoveReadingAxes(normal).degrees!=0 || diaryMoveReadingAxes(current).degrees!=0)
        throw std::runtime_error("Calibrated landscape half-turn required");
    const auto raw=diaryMoveAxes(normal.value("points").toArray());
    QJsonArray points;
    for(const QPointF p:{QPointF(),QPointF(100,0),QPointF(0,100)}) {
        const auto q=raw.map(QPointF(base.physical.width()-p.x(),base.physical.height()-p.y()));
        points.append(QJsonArray{q.x(),q.y()});
    }
    normal.insert("points",points);
    normal.insert("windowCorners",current.value("windowCorners"));
    selected.insert("viewport",normal);
    for(const auto &field:{"nativePose","hiddenNativePose"})
        selected.insert(field,oppositeLandscapePose(selected.value(field).toObject(),base));
    return selected;
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
    if(schema==4 && size==QSize(1696,954) &&
       diaryMoveFrame(selected.value("viewport").toObject()).degrees!=diaryMoveFrame(current).degrees)
        selected=oppositeLandscapeReference(selected,current);
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
