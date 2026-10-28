#pragma once
#include <QQmlEngine>
#include <QJSValue>
#include <QObject>
#include <QString>
#include <QQuickItem>
#include <QQuickWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>
#include "move-orientation.h"

namespace diaryMetadata {
// Fixed numeric metadata only. Matches the runtime's three-point snapshot shape,
// but is NOT a normal-zoom assertion or a trusted runtime reference. A live
// owner-confirmed baseline and template/zoom discrimination are still required.
inline QJsonObject viewportSnapshot(QQmlEngine *engine, QQuickItem *view) {
    if (!engine || !view || !view->window() || !view->isVisible() ||
        view->width() <= 0 || view->height() <= 0 ||
        !std::isfinite(view->width()) || !std::isfinite(view->height())) return {};
    auto *window=view->window();
    if(window->width()<=0 || window->height()<=0 ||
       !std::isfinite(window->devicePixelRatio()) || window->devicePixelRatio()<=0) return {};
    auto fn=engine->evaluate(QStringLiteral(R"JS((function(view,quarterTurns) {
        try {
            const points=[[0,0],[100,0],[0,100]];
            let mapped=[];
            for(let i=0;i<points.length;++i) {
                const p=view.viewToScene(view.mapFromItem(null,points[i][0],points[i][1]));
                if(!p || typeof p.x!=='number' || typeof p.y!=='number' ||
                   !isFinite(p.x) || !isFinite(p.y))return '';
                mapped.push([p.x,p.y]);
            }
            if(!quarterTurns && (mapped[1][0]<=mapped[0][0] || mapped[2][1]<=mapped[0][1] ||
               Math.abs(mapped[1][1]-mapped[0][1])>0.001 ||
               Math.abs(mapped[2][0]-mapped[0][0])>0.001))return '';
            const det=(mapped[1][0]-mapped[0][0])*(mapped[2][1]-mapped[0][1])-
                      (mapped[1][1]-mapped[0][1])*(mapped[2][0]-mapped[0][0]);
            if(!isFinite(det) || Math.abs(det)<0.000001)return '';
            let result={width:view.width,height:view.height,points:mapped};
            if(quarterTurns) {
                let corners=[];
                for(const p of [[0,0],[view.width,0],[0,view.height],[view.width,view.height]]) {
                    const q=view.mapToItem(null,p[0],p[1]);
                    if(!isFinite(q.x)||!isFinite(q.y))return '';
                    corners.push([q.x,q.y]);
                }
                result.windowCorners=corners;
            }
            return JSON.stringify(result);
        }catch(e){return '';}
    }))JS"),QStringLiteral("diagnostic:readonly-viewport"));
    if(!fn.isCallable())return {};
    const auto ownership=QQmlEngine::objectOwnership(view);
    const auto wrapper=engine->newQObject(view);
    QQmlEngine::setObjectOwnership(view,ownership);
    const auto result=fn.call({wrapper,
#ifdef INK_MOVE_PER_REQUEST
        true
#else
        false
#endif
    });
    if(result.isError() || !result.isString())return {};
    auto snapshot=QJsonDocument::fromJson(result.toString().toUtf8()).object();
    if(snapshot.isEmpty())return {};
#ifdef INK_MOVE_PER_REQUEST
    try {diaryMoveAxes(snapshot.value("points").toArray());} catch(...) {return {};}
#endif
    snapshot.insert("windowWidth",window->width());
    snapshot.insert("windowHeight",window->height());
    snapshot.insert("devicePixelRatio",window->devicePixelRatio());
    return snapshot;
}

// Use the existing engine directly: no QML component or asynchronous imports.
// Only this fixed script runs; it reads coordinates and returns a fixed status.
inline QString mappingStatus(QQmlEngine *engine, QObject *view) {
    if (!engine || !view) return QStringLiteral("engine-unavailable");
    auto function = engine->evaluate(QStringLiteral(R"JS((function(view) {
        try {
            const points = [[0,0],[100,0],[0,100]];
            let out = [];
            for (let i = 0; i < points.length; ++i) {
                const p = view.viewToScene(view.mapFromItem(null, points[i][0], points[i][1]));
                if (!p || typeof p.x !== 'number' || typeof p.y !== 'number' ||
                    !isFinite(p.x) || !isFinite(p.y)) return 'non-finite';
                out.push([p.x, p.y]);
            }
            const determinant = (out[1][0]-out[0][0])*(out[2][1]-out[0][1]) -
                                (out[1][1]-out[0][1])*(out[2][0]-out[0][0]);
            return isFinite(determinant) && Math.abs(determinant) > 0.000001 ? 'valid' : 'degenerate';
        } catch (exception) { return 'invocation-failed'; }
    }))JS"), QStringLiteral("diagnostic:readonly"));
    if (!function.isCallable()) return QStringLiteral("script-unavailable");
    // newQObject can change ownership for a previously unwrapped C++ object.
    // Restore it before executing JS so the probe cannot transfer UI ownership.
    const auto ownership = QQmlEngine::objectOwnership(view);
    const auto wrapper = engine->newQObject(view);
    QQmlEngine::setObjectOwnership(view, ownership);
    const auto result = function.call({wrapper});
    if (result.isError()) return QStringLiteral("invocation-failed");
    const auto status = result.toString();
    for (const auto *known : {"valid", "non-finite", "degenerate", "invocation-failed"})
        if (status == QLatin1String(known)) return status;
    return QStringLiteral("unexpected-result");
}
}
