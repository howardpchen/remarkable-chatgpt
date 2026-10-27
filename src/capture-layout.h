#pragma once
#include <QRect>
#include <QSize>
#include <cmath>
#include <stdexcept>

// Geometry preparation is separate from product/firmware installation support.
// Generic experimental callers supply an explicit crop rather than scaling rM2
// margins. The separately guarded Move path below fixes the measured fixture crop.
enum class DiaryCaptureProfile { Rm2, ExperimentalMove };
struct DiaryCaptureLayout {
    QRect area;
    int answerHeight;
    int answerGap = 45;
};
inline DiaryCaptureLayout diaryCaptureLayout(const QSize &physicalPixels,
                                             const QSize &logicalWindow,
                                             qreal devicePixelRatio = 1,
                                             DiaryCaptureProfile profile = DiaryCaptureProfile::Rm2,
                                             const QRect &measuredContentArea = {}) {
    const QSize expected = profile == DiaryCaptureProfile::Rm2 ? QSize(1404,1872) : QSize(954,1696);
    if (!std::isfinite(devicePixelRatio) || devicePixelRatio != 1 ||
        physicalPixels != expected || logicalWindow != expected)
        throw std::runtime_error("Unsupported capture/window geometry; portrait DPR 1 required");
    const QRect area = profile == DiaryCaptureProfile::Rm2 ? QRect(150,0,1174,1752) : measuredContentArea;
    if (area.width() < 300 || area.height() <= 0 || !QRect(QPoint(0,0),expected).contains(area))
        throw std::runtime_error("Measured notebook capture area required");
    // Keep the original rM2 render budget. Move's conservative budget is the
    // supplied content height, not a claim about offscreen append placement.
    return {area, profile == DiaryCaptureProfile::Rm2 ? 1582 : area.height(), 45};
}

// Local preparation only: neither this policy nor a successful fixture promotes
// the runtime loader, manifest or installer. The opt-in is exact, not a model
// allowlist. Blank identity and normal calibration must come from verified
// read-only metadata; screenshot heuristics cannot supply either assertion.
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QImage>
#include <QPointF>
struct DiaryMoveMapping { qreal sx, sy; QPointF zero; };
inline bool diaryExperimentalMoveTuple(const QByteArray &machine, const QByteArray &osRelease,
                                      const QByteArray &qt, const QString &optIn) {
    if (optIn != QLatin1String("chiappa/3.28.0.172/qt6.10.3/blank-visible-v1") ||
        machine.trimmed() != "reMarkable Chiappa" || qt != "6.10.3") return false;
    int matches=0;
    for (auto line : osRelease.split('\n')) {
        if (!line.startsWith("IMG_VERSION=")) continue;
        auto value=line.mid(12).trimmed();
        if (value.startsWith('"') && value.endsWith('"')) value=value.mid(1,value.size()-2);
        if (value != "3.28.0.172") return false;
        ++matches;
    }
    return matches == 1;
}
inline DiaryMoveMapping diaryExperimentalMoveMapping(const QJsonObject &snapshot,
                                                      const QJsonObject &normalReference) {
    // Factory identity is read-only; historical one-shot builds stay Blank-only.
    // A separately provisioned exact normal reference is still mandatory.
    const auto name=snapshot.value("templateName").toString();
    const bool supported=name==QLatin1String("Blank")
#ifdef INK_MOVE_PER_REQUEST
        || name==QLatin1String("P Dots S") || name==QLatin1String("P Grid small")
#endif
        ;
    if (!supported ||
        normalReference.isEmpty() || snapshot != normalReference ||
        snapshot.value("width").toDouble() != 954 ||
        snapshot.value("height").toDouble() != 1696 ||
        snapshot.value("windowWidth").toDouble() != 954 ||
        snapshot.value("windowHeight").toDouble() != 1696 ||
        snapshot.value("devicePixelRatio").toDouble(0) != 1)
        throw std::runtime_error("Supported template and normal Move viewport required");
    const auto points=snapshot.value("points").toArray();
    if (points.size()!=3) throw std::runtime_error("Move mapping requires three calibration points");
    qreal p[3][2];
    for(int i=0;i<3;++i) {
        const auto point=points[i].toArray();
        if(point.size()!=2) throw std::runtime_error("Invalid Move mapping point");
        for(int j=0;j<2;++j) {
            if(!point[j].isDouble() || !std::isfinite(point[j].toDouble()))
                throw std::runtime_error("Non-finite Move mapping");
            p[i][j]=point[j].toDouble();
        }
    }
    const qreal sx=(p[1][0]-p[0][0])/100, sy=(p[2][1]-p[0][1])/100;
    if (sx<=0 || sy<=0 || sx>4 || sy>4 ||
        std::abs(p[1][1]-p[0][1])>0.001 || std::abs(p[2][0]-p[0][0])>0.001 ||
        std::abs(sx-sy)>0.00001)
        throw std::runtime_error("Unsupported Move transform");
    return {sx,sy,{p[0][0],p[0][1]}};
}
inline DiaryCaptureLayout diaryExperimentalMoveLayout(const QSize &pixels, const QSize &window,
                                                      qreal dpr) {
    return diaryCaptureLayout(pixels,window,dpr,DiaryCaptureProfile::ExperimentalMove,{120,112,834,1584});
}
// Only the separate live target includes the top strip. Historical offline
// fixtures keep their measured crop and consumed-run semantics unchanged.
inline QRect diaryMoveBoundedCaptureArea() {
#ifdef INK_MOVE_LIVE_TEST
    return {0,0,954,1696};
#else
    return {120,112,834,1584};
#endif
}
// Full-page structural containers are expected; any smaller visible UI item
// overlapping the full-screen capture must be absent, not silently cropped.
inline bool diaryMoveCaptureUiClear(const QJsonObject &pose) {
    if (!pose.value("ok").toBool() || !pose.value("items").isArray()) return false;
    const QRectF content(0,0,954,1696);
    for (const auto &entry:pose.value("items").toArray()) {
        const auto item=entry.toObject();
        const auto a=item.value("rect").toArray();
        if (a.size()!=4) return false;
        for (const auto &v:a) if (!v.isDouble() || !std::isfinite(v.toDouble())) return false;
        const QRectF rect(a[0].toDouble(),a[1].toDouble(),a[2].toDouble(),a[3].toDouble());
        if (rect.intersects(content) && !rect.contains(content)) return false;
    }
    return true;
}
struct DiaryMovePlacement {
    QImage nativeImage;
    QRectF screenBounds;
    QJsonObject viewport;
};
inline DiaryMovePlacement diaryExperimentalMovePlacement(const QImage &rendered, int top,
        const QJsonObject &snapshot, const QJsonObject &normalReference) {
    const auto mapping=diaryExperimentalMoveMapping(snapshot,normalReference);
    const QRectF area(120,112,834,1584);
    // Reject BEFORE resizing: floor rounding must not make an oversized answer
    // appear to fit. This path never extrapolates past the captured page.
    if(rendered.isNull() || rendered.width()!=834 || rendered.height()>1584 ||
        top<area.top() || qreal(top)+rendered.height()>area.y()+area.height())
        throw std::runtime_error("Move answer does not fit the visible content area");
    const qreal w=std::floor(rendered.width()*mapping.sx), h=std::floor(rendered.height()*mapping.sy);
    if(!std::isfinite(w) || !std::isfinite(h) || w<1 || h<1 || w>4096 || h>4096 || w*h>16000000)
        throw std::runtime_error("Move native image budget exceeded");
    QImage native=rendered.scaled(int(w),int(h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    if(native.isNull()) throw std::runtime_error("Move native image scaling failed");
    QRectF footprint(area.center().x()-native.width()/mapping.sx/2,top,
                     native.width()/mapping.sx,native.height()/mapping.sy);
    if(!area.contains(footprint)) throw std::runtime_error("Move native image escapes visible content area");
    return {native,footprint,snapshot};
}

// Until separately authorized live API validation, the experimental Move branch
// is compile-time constrained to offline fixtures; missing/false config rejects.
inline bool diaryExperimentalMoveDemoAllowed(bool move, bool demo) {
    return !move || demo;
}

// The measured crop predates the UI overlay. Even valid metadata/reference must
// not authorize integrated capture until UI exclusion is implemented/reviewed.
// Deliberately no config flag or JSON field can override this preparation gate.
inline void diaryRequireExperimentalMoveCaptureReady(const QJsonObject &snapshot,
                                                      const QJsonObject &reference) {
    diaryExperimentalMoveMapping(snapshot,reference);
    throw std::runtime_error("Move capture UI exclusion is not validated");
}
