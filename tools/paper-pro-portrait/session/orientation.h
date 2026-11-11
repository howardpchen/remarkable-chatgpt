#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QSize>
#include <QPointF>
#include <QImage>
#include <QTransform>
#include <cmath>
#include <stdexcept>

// Screen coordinates are logical window coordinates. Never infer a rotation
// from the framebuffer dimensions or reuse a portrait calibration in landscape.
inline bool diaryPaperProWindowSupported(const QSize &size) {
    return size==QSize(1620,2160);
}
inline bool diaryPaperProReadingSizeSupported(const QSize &size) {
    return size==QSize(1620,2160)
#ifdef INK_PAPER_PRO_PER_REQUEST
        || size==QSize(2160,1620)
#endif
        ;
}
struct DiaryPaperProFrame {
    QSize physical, reading;
    QTransform toPhysical;
    int degrees;
};
inline DiaryPaperProFrame diaryPaperProFrame(const QJsonObject &v) {
    const QSize physical(v.value("windowWidth").toInt(),v.value("windowHeight").toInt());
    const QSize reading(v.value("width").toInt(),v.value("height").toInt());
    if(!diaryPaperProWindowSupported(physical) || !diaryPaperProReadingSizeSupported(reading) ||
       v.value("width").toDouble()!=reading.width() || v.value("height").toDouble()!=reading.height() ||
       v.value("windowWidth").toDouble()!=physical.width() || v.value("windowHeight").toDouble()!=physical.height() ||
       v.value("devicePixelRatio").toDouble()!=1)
        throw std::runtime_error("Supported Move view/window at DPR 1 required");
    const auto corners=v.value("windowCorners").toArray();
    // Historical portrait references omit corners; only identity may do so.
    if(corners.isEmpty() && !v.contains("windowCorners") && physical==reading)
        return {physical,reading,QTransform(),0};
    const int turns[]={0,90,180,270};
    for(int degrees:turns) {
#ifndef INK_PAPER_PRO_PER_REQUEST
        if(degrees!=0)continue;
#else
        if(degrees==180)continue;
#endif
        QTransform rotation;rotation.rotate(degrees);
        const auto bounds=rotation.mapRect(QRectF(QPointF(),QSizeF(reading)));
        if(bounds.size()!=QSizeF(physical))continue;
        QTransform map(rotation.m11(),rotation.m12(),rotation.m21(),rotation.m22(),-bounds.x(),-bounds.y());
        QJsonArray expected;
        for(const QPointF point:{QPointF(0,0),QPointF(reading.width(),0),QPointF(0,reading.height()),QPointF(reading.width(),reading.height())}) {
            const auto q=map.map(point);expected.append(QJsonArray{q.x(),q.y()});
        }
        if(corners==expected)return {physical,reading,map,degrees};
    }
    throw std::runtime_error("Measured quarter-turn view/window corners required");
}
inline QSize diaryPaperProViewportSize(const QJsonObject &v) {return diaryPaperProFrame(v).reading;}
struct DiaryPaperProAxes {
    QPointF zero, x, y;
    qreal scale;
    int degrees;
    QPointF map(const QPointF &p) const {return zero+x*p.x()+y*p.y();}
};
inline DiaryPaperProAxes diaryPaperProAxes(const QJsonArray &points) {
    if(points.size()<3)throw std::runtime_error("Three Move mapping points required");
    QPointF p[3];
    for(int i=0;i<3;++i) {
        const auto a=points[i].toArray();
        if(a.size()!=2 || !a[0].isDouble() || !a[1].isDouble() ||
           !std::isfinite(a[0].toDouble()) || !std::isfinite(a[1].toDouble()))
            throw std::runtime_error("Invalid Move mapping point");
        p[i]={a[0].toDouble(),a[1].toDouble()};
    }
    const auto x=(p[1]-p[0])/100, y=(p[2]-p[0])/100;
    const qreal scale=std::hypot(x.x(),x.y());
    if(!std::isfinite(scale) || scale<=0 || scale>4)
        throw std::runtime_error("Unsupported Move scale");
    // Only orientation-preserving, uniform quarter turns. No shear/reflection.
    const QPointF unit[]={{1,0},{0,1},{-1,0},{0,-1}};
    for(int i=0;i<4;++i) {
#ifndef INK_PAPER_PRO_PER_REQUEST
        if(i!=0)continue;
#endif
        const auto expectedX=unit[i]*scale, expectedY=unit[(i+1)%4]*scale;
        if(std::hypot(x.x()-expectedX.x(),x.y()-expectedX.y())<=0.00001 &&
           std::hypot(y.x()-expectedY.x(),y.y()-expectedY.y())<=0.00001)
            return {p[0],x,y,scale,i*90};
    }
    throw std::runtime_error("Unsupported Move transform");
}

// Compose only the calibrated view->physical and physical->scene maps. Reading
// pixels are never treated as physical-window pixels in a split layout.
inline DiaryPaperProAxes diaryPaperProReadingAxes(const QJsonObject &v) {
    const auto frame=diaryPaperProFrame(v);
    const auto raw=diaryPaperProAxes(v.value("points").toArray());
    QJsonArray points;
    for(const QPointF point:{QPointF(0,0),QPointF(100,0),QPointF(0,100)}) {
        const auto q=raw.map(frame.toPhysical.map(point));points.append(QJsonArray{q.x(),q.y()});
    }
    return diaryPaperProAxes(points);
}
inline QImage diaryPaperProReadingImage(const QImage &physical,const QJsonObject &v) {
    const auto frame=diaryPaperProFrame(v);
    if(physical.isNull() || physical.size()!=frame.physical)
        throw std::runtime_error("Framebuffer dimensions changed");
    const auto image=frame.degrees ? physical.transformed(QTransform().rotate(-frame.degrees),Qt::FastTransformation) : physical;
    if(image.size()!=frame.reading)throw std::runtime_error("Reading image dimensions changed");
    return image;
}
