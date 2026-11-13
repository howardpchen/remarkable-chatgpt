#pragma once
#include "native-content-bounds.h"
#include <QPointF>
#include <limits>
#include <stdexcept>

namespace diaryNativePlacement {
// Project every envelope corner through the frozen reading->scene inverse.
// Bounds are deliberately not clipped: offscreen lower content must prevent
// insertion above it. Degenerate lines remain obstacles. No template pixels.
template<class Axes>
inline int lastRow(const QJsonObject &content,const Axes &axes,int height) {
    if(!content.value("ok").toBool() || height<=0 || !std::isfinite(axes.scale) || axes.scale<=0)
        throw std::runtime_error("Invalid native content snapshot");
    const auto rectangles=content.value("rectangles").toObject();
    const auto items=rectangles.value("itemsBoundingRect").toObject();
    const auto text=rectangles.value("rootDocumentBoundingRect").toObject();
    // An empty item bound plus a zero-height root is not verified writing.
    if(items.value("width").toDouble()==0 && items.value("height").toDouble()==0 &&
       text.value("height").toDouble()==0)return -1;
    if(!content.contains("sceneEnvelope"))return -1;
    const auto r=content.value("sceneEnvelope").toObject();
    const double x=r.value("x").toDouble(),y=r.value("y").toDouble();
    const double w=r.value("width").toDouble(),h=r.value("height").toDouble();
    double bottom=-std::numeric_limits<double>::infinity();
    for(const QPointF point:{QPointF(x,y),QPointF(x+w,y),QPointF(x,y+h),QPointF(x+w,y+h)}) {
        const auto delta=point-axes.zero;
        const double row=QPointF::dotProduct(delta,axes.y)/(axes.scale*axes.scale);
        if(!std::isfinite(row))throw std::runtime_error("Invalid native projection");
        bottom=std::max(bottom,row);
    }
    if(bottom<0)return -1;
    // Returning the last row is enough to trigger the existing fit refusal,
    // avoiding integer overflow for arbitrarily large offscreen bounds.
    return bottom>=height-1 ? height-1 : int(std::ceil(bottom));
}
}
