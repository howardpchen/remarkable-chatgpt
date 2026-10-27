#pragma once
#include <QQmlEngine>
#include <QJSValue>

namespace diaryMetadata {
// Source-grounded in Chiappa 3.28.0.172 DocumentView.qml. The page index and
// template filename stay inside this function; only fixed status values escape.
inline QString templateStatus(QQmlEngine *engine, QObject *root) {
    if(!engine || !root)return QStringLiteral("unavailable");
    auto fn=engine->evaluate(QStringLiteral(R"JS((function(root) {
        try {
            if(!root.documentLoaded || !root.document ||
               typeof root.currentPage !== 'number' ||
               !isFinite(root.currentPage) || root.currentPage < 0 ||
               Math.floor(root.currentPage) !== root.currentPage) return 'unavailable';
            const name=root.document.templateForPage(root.currentPage);
            if(typeof name !== 'string' || !name.length)return 'unavailable';
            return name === 'Blank' ? 'blank' : 'nonblank';
        } catch(e) {return 'unavailable';}
    }))JS"),QStringLiteral("diagnostic:readonly-template"));
    if(!fn.isCallable())return QStringLiteral("unavailable");
    const auto owner=QQmlEngine::objectOwnership(root);
    const auto wrapped=engine->newQObject(root);
    QQmlEngine::setObjectOwnership(root,owner);
    auto result=fn.call({wrapped});
    if(result.isError())return QStringLiteral("unavailable");
    const auto status=result.toString();
    return status=="blank" || status=="nonblank" ? status : QStringLiteral("unavailable");
}
}
