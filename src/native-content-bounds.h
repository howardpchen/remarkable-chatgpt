#pragma once
#include <QObject>
#include <QJsonObject>
#include <QRectF>
#include <cmath>
#include <algorithm>

namespace diaryContentBounds {
// GUI-thread read-only geometry. No layer toggles, selection, clipboard or writes.
// These native names are source-grounded, but their completeness (hidden layers,
// erasures, offscreen ink and typed text) still requires target validation.
inline QJsonObject read(QObject *controller) {
    QJsonObject out{{"diagnosticOnly",true},{"nativeInsertionAuthorized",false},
                    {"coverageVerified",false},{"ok",false}};
    if (!controller) { out.insert("error","controller-unavailable"); return out; }
    QJsonObject rectangles;
    for (const char *name : {"itemsBoundingRect", "rootDocumentBoundingRect", "boundingRect"}) {
        const auto value=controller->property(name);
        if (value.metaType().id()!=QMetaType::QRectF) continue;
        const auto r=value.toRectF();
        if (!std::isfinite(r.x()) || !std::isfinite(r.y()) ||
            !std::isfinite(r.width()) || !std::isfinite(r.height()) ||
            !std::isfinite(r.right()) || !std::isfinite(r.bottom()) ||
            r.width()<0 || r.height()<0) continue;
        // Degenerate bounds are retained; a zero-height horizontal stroke must
        // not be silently classified as an empty scene.
        rectangles.insert(name,QJsonObject{{"x",r.x()},{"y",r.y()},
            {"width",r.width()},{"height",r.height()}});
    }
    out.insert("rectangles",rectangles);
    out.insert("ok",rectangles.contains("itemsBoundingRect"));
    if (!rectangles.contains("itemsBoundingRect"))out.insert("error","item-bounds-unavailable");
    return out;
}
// Experimental detection envelope in native scene coordinates only. This is not
// a placement authorization or an emptiness predicate. Include degenerate lines.
inline QJsonObject detect(QObject *controller) {
    auto out=read(controller);
    out.insert("method","native-content-envelope-v1");
    out.insert("placementReady",false);
    out.insert("reason","coverage-and-page-revision-unverified");
    const auto rectangles=out.value("rectangles").toObject();
    for (const char *name : {"itemsBoundingRect","rootDocumentBoundingRect","boundingRect"}) {
        if (!rectangles.contains(name)) {
            out.insert("ok",false); out.insert("error","incomplete-native-bounds");
            return out;
        }
    }
    bool have=false;
    double left=0,top=0,right=0,bottom=0;
    for (const auto &value : rectangles) {
        const auto r=value.toObject();
        const double x=r.value("x").toDouble(), y=r.value("y").toDouble();
        const double w=r.value("width").toDouble(), h=r.value("height").toDouble();
        if (w==0 && h==0) continue;
        if (!have) {left=x;top=y;right=x+w;bottom=y+h;have=true;}
        else {left=std::min(left,x);top=std::min(top,y);
              right=std::max(right,x+w);bottom=std::max(bottom,y+h);}
    }
    if (have && (!std::isfinite(right-left) || !std::isfinite(bottom-top))) {
        out.insert("ok",false);out.insert("error","envelope-overflow");return out;
    }
    if (have) out.insert("sceneEnvelope",QJsonObject{{"x",left},{"y",top},
        {"width",right-left},{"height",bottom-top}});
    out.insert("emptyVerified",false);
    return out;
}

}
