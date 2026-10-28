#pragma once
#include "../move-demo/admission.h"
#include <QMetaProperty>

namespace moveSession {
// Request-scoped native hiding, independent of the short screenshot lease.
// Only our exact admitted widget may own the reversible QML Bindings. Never
// write native visible/expanded properties imperatively or bless a live pose.
class ToolbarLease final {
public:
    ToolbarLease()=default;
    ToolbarLease(const ToolbarLease &)=delete;
    ToolbarLease &operator=(const ToolbarLease &)=delete;
    ~ToolbarLease() { release(); }
    QString acquire(QQuickWindow *window) {
        if (control_) return "request-toolbar-lease-already-held";
        const auto error=moveDemo::inspectOwnControl(window,false);
        if (!error.isEmpty()) return error;
        QList<QQuickItem *> pending{window->contentItem()};
        while (!pending.isEmpty()) {
            auto *item=pending.takeLast();
            if (item->objectName()=="moveUiProbe") { control_=item; break; }
            for (auto *child:item->childItems()) pending.append(child);
        }
        if (!control_) return "request-toolbar-control-unavailable";
        const int index=control_->metaObject()->indexOfProperty("diaryRequestToolbarSuppressed");
        if (index<0 || !control_->metaObject()->property(index).isWritable() ||
            control_->property("diaryRequestToolbarSuppressed").metaType().id()!=QMetaType::Bool ||
            control_->property("diaryRequestToolbarSuppressed").toBool()) {
            control_.clear(); // Do not release a lease owned by somebody else.
            return "request-toolbar-control-incompatible-or-held";
        }
        control_->setProperty("diaryRequestToolbarSuppressed",true);
        if (!control_ || control_->property("nativeRequestToolbarReady").metaType().id()!=QMetaType::Bool ||
            !control_->property("nativeRequestToolbarReady").toBool()) {
            release(); return "request-toolbar-hide-unavailable";
        }
        return {};
    }
    void release() {
        if (control_) control_->setProperty("diaryRequestToolbarSuppressed",false);
        control_.clear();
    }
private:
    QPointer<QQuickItem> control_;
};
}
