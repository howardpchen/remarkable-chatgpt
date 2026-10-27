#pragma once
// Local-only candidate mathematics. No product/firmware admission or insertion.
#include "../../src/layout.h"
#include "../../src/markdown.h"
#include <QTransform>
#include <QSizeF>

namespace paperProPortrait {
inline QSize candidatePixels() { return {1620,2160}; }
inline void require(bool condition,const char *message) {
    if(!condition) throw std::runtime_error(message);
}
inline bool finite(qreal value) { return std::isfinite(value); }
inline void axisAligned(const QTransform &t) {
    require(finite(t.m11()) && finite(t.m12()) && finite(t.m13()) &&
        finite(t.m21()) && finite(t.m22()) && finite(t.m23()) &&
        finite(t.m31()) && finite(t.m32()) && finite(t.m33()) &&
        t.m12()==0 && t.m21()==0 && t.m13()==0 && t.m23()==0 && t.m33()==1 &&
        t.m11()>0 && t.m22()>0 && std::abs(t.m11()-t.m22())<=1e-9*std::min(t.m11(),t.m22()),
        "Positive finite uniform axis-aligned calibration required");
}
inline void finiteRect(const QRectF &rect) {
    require(finite(rect.x()) && finite(rect.y()) && finite(rect.width()) && finite(rect.height()) &&
        finite(rect.right()) && finite(rect.bottom()) && rect.width()>0 && rect.height()>0 &&
        rect.right()>rect.left() && rect.bottom()>rect.top(),"Invalid mapped rectangle");
}
struct Viewport {
    QSizeF logicalSize;
    qreal dpr=0; // Observation, never a substitute for coordinate calibration.
    QTransform logicalToScene;
    QString pageId;
    QString templateName;
    bool operator==(const Viewport &other) const {
        return logicalSize==other.logicalSize && dpr==other.dpr &&
            logicalToScene==other.logicalToScene && pageId==other.pageId &&
            templateName==other.templateName;
    }
};
struct Placement {
    QImage physicalCrop;
    QImage rendered;
    QImage nativeCandidate;
    QRectF physicalBounds,logicalBounds,sceneBounds;
    int lastInk=-1;
};
class Candidate final {
    const Viewport reference_;
    const QTransform physicalToLogical_;
    const QRect crop_;
public:
    Candidate(Viewport reference,QTransform measuredPhysicalToLogical,QRect measuredCrop)
        :reference_(std::move(reference)),physicalToLogical_(measuredPhysicalToLogical),crop_(measuredCrop) {
        axisAligned(physicalToLogical_); axisAligned(reference_.logicalToScene);
        const auto size=reference_.logicalSize;
        require(finite(size.width()) && finite(size.height()) && size.width()>0 &&
            size.height()>size.width() && finite(reference_.dpr) && reference_.dpr>0 &&
            !reference_.pageId.isEmpty() && reference_.templateName=="Blank",
            "Explicit portrait Blank viewport required");
        // Require independently supplied mapping, including origin and extents.
        // DevicePixelRatio is deliberately absent from this calculation.
        const auto mapped=physicalToLogical_.mapRect(QRectF(QPointF(0,0),candidatePixels()));
        require(std::abs(mapped.x())<1e-9 && std::abs(mapped.y())<1e-9 &&
            std::abs(mapped.width()-size.width())<1e-9 &&
            std::abs(mapped.height()-size.height())<1e-9,
            "Physical-to-logical calibration does not match observed extents");
        finiteRect(reference_.logicalToScene.mapRect(QRectF(QPointF(0,0),size)));
        require(crop_.width()>=300 && crop_.width()<=1600 && crop_.height()>0 &&
            QRect(QPoint(0,0),candidatePixels()).contains(crop_),"Measured bounded crop required");
    }
    void check(const Viewport &current) const {
        require(current==reference_,"Frozen viewport/page/template changed");
    }
    Placement prepare(const QImage &capture,const Viewport &current,const QString &answer,int gap=45) const {
        check(current);
        require(!capture.isNull() && capture.size()==candidatePixels() &&
            capture.devicePixelRatio()==1,"Candidate physical capture required");
        require(gap>=0 && gap<=crop_.height(),"Invalid physical answer gap");
        Placement result;
        result.physicalCrop=capture.copy(crop_);
        result.lastInk=lastInkRow(result.physicalCrop);
        const int top=crop_.y()+result.lastInk+1+gap;
        result.rendered=renderDiaryMarkdown(answer,"DejaVu Sans",crop_.width()).image;
        require(!result.rendered.isNull() && top>=crop_.y() &&
            qreal(top)+result.rendered.height()<=qreal(crop_.y())+crop_.height(),
            "Answer does not fit physical crop");
        const qreal scale=physicalToLogical_.m11()*reference_.logicalToScene.m11();
        const qreal width=std::floor(result.rendered.width()*scale);
        const qreal height=std::floor(result.rendered.height()*scale);
        require(finite(width) && finite(height) && width>=1 && height>=1 &&
            width<=4096 && height<=4096 && width*height<=16000000,
            "Native candidate image budget exceeded");
        result.nativeCandidate=result.rendered.scaled(int(width),int(height),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        require(!result.nativeCandidate.isNull(),"Native candidate allocation failed");
        // Actual rounded raster footprint, centered in the measured crop.
        result.physicalBounds={crop_.x()+(crop_.width()-width/scale)/2, qreal(top),width/scale,height/scale};
        require(QRectF(crop_).contains(result.physicalBounds),"Rounded footprint escapes crop");
        result.logicalBounds=physicalToLogical_.mapRect(result.physicalBounds);
        result.sceneBounds=reference_.logicalToScene.mapRect(result.logicalBounds);
        finiteRect(result.logicalBounds);finiteRect(result.sceneBounds);
        return result;
    }
};
}
