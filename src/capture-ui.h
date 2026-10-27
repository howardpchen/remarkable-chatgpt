#pragma once

#include <QGuiApplication>
#include <QMetaProperty>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

// Own controls only. Never writes `visible`: doing that destroys QML bindings.
// All public operations, construction and destruction MUST run on the GUI
// thread. A worker may wait for the callback, but the GUI thread must stay free.
// Ready acknowledges a Qt Quick render, NOT framebuffer-spy/e-ink freshness.
class DiaryCaptureUiSession final : public QObject {
public:
    enum class Event { RenderReady, Failed };
    using Notify = std::function<void(Event, const QString &)>;

    explicit DiaryCaptureUiSession(QQuickWindow *window, Notify notify,
                                   QObject *parent = nullptr)
        : QObject(parent), window_(window), notify_(std::move(notify)) {
        assertGui();
        timeout_.setSingleShot(true);
        connect(&timeout_, &QTimer::timeout, this, [this] {
            fail(QStringLiteral("Own control suppression render timed out"));
        });
    }
    ~DiaryCaptureUiSession() override { release(); }

    // Single use. Failure restores every property this session acquired.
    void begin(int renderTimeoutMs = 3000) {
        assertGui();
        QPointer<DiaryCaptureUiSession> alive(this);
        if (started_) { fail(QStringLiteral("Capture UI session already started")); return; }
        started_ = true;
        if (!window_ || !window_->isVisible() || renderTimeoutMs <= 0) {
            fail(QStringLiteral("Capture window unavailable")); return;
        }
        bool valid = true;
        std::vector<QQuickItem *> scene;
        const auto controls = discover(valid, &scene);
        if (!valid || controls.empty()) {
            fail(QStringLiteral("Own capture controls missing or incompatible")); return;
        }
        // Validate the complete set before modifying any control. Already-held
        // suppression fails closed; a second session cannot steal restoration.
        for (auto *item : controls) {
            if (item->property("diaryCaptureSuppressed").toBool()) {
                fail(QStringLiteral("Own capture control already suppressed")); return;
            }
        }
        // Watch the existing scene, not just root names. A transient new overlay
        // could otherwise appear in pixels and disappear before the post-check.
        // Conservative invalidation of ANY visual tree mutation is intentional.
        for (auto *item : scene)
            connections_.push_back(connect(item, &QQuickItem::childrenChanged, this, [this] {
                fail(QStringLiteral("Capture scene item tree changed"));
            }));
        for (auto *item : controls) {
            controls_.push_back(item);
            QPointer<QQuickItem> liveItem(item);
            const bool set = item->setProperty("diaryCaptureSuppressed", true);
            if (!alive || released_) return; // synchronous QML changes may invalidate us
            if (!liveItem) { fail(QStringLiteral("Own control vanished during suppression")); return; }
            if (!set) {
                fail(QStringLiteral("Cannot suppress own capture control")); return;
            }
            connections_.push_back(connect(item, &QObject::destroyed, this, [this] {
                fail(QStringLiteral("Own capture control destroyed"));
            }));
        }
        connections_.push_back(connect(window_, &QObject::destroyed, this, [this] {
            fail(QStringLiteral("Capture window destroyed"));
        }));
        auto phase = phase_ = std::make_shared<RenderPhase>();
        QPointer<DiaryCaptureUiSession> self(this);
        // Synchronization blocks the GUI thread. Connecting AFTER suppression
        // ensures this signal belongs to a synchronization with hidden controls,
        // not the afterRendering tail of an earlier frame.
        connections_.push_back(connect(window_, &QQuickWindow::afterSynchronizing,
            window_, [phase] { phase->synced.store(true); }, Qt::DirectConnection));
        connections_.push_back(connect(window_, &QQuickWindow::afterRendering,
            window_, [phase, self] {
                if (!phase->active.load() || !phase->synced.load() || phase->posted.exchange(true)) return;
                // Render-thread callback touches no QML/QObject state. qGuiApp
                // outlives the session; QPointer is checked only on the GUI.
                QMetaObject::invokeMethod(qGuiApp, [phase, self] {
                    if (phase->active.load() && self) self->rendered();
                }, Qt::QueuedConnection);
            }, Qt::DirectConnection));
        timeout_.start(renderTimeoutMs);
        window_->update();
    }

    // Call on GUI immediately before AND after obtaining screenshot pixels.
    // A new/replaced overlay invalidates the capture instead of leaking UI.
    bool readyForCapture() const {
        assertGui();
        if (!ready_ || released_ || !window_ || !window_->isVisible()) return false;
        bool valid = true;
        const auto live = discover(valid);
        if (!valid || live.size() != controls_.size()) return false;
        for (const auto &item : controls_) {
            if (!item || item->window() != window_ || item->isVisible() ||
                !item->property("diaryCaptureSuppressed").toBool()) return false;
            if (std::find(live.begin(), live.end(), item.data()) == live.end()) return false;
        }
        return true;
    }

    // Success, cancellation, exception and teardown all take this same path.
    // Idempotent; no callback on explicit release/destruction.
    void release() {
        assertGui();
        if (released_) return;
        released_ = true;
        ready_ = false;
        timeout_.stop();
        if (phase_) phase_->active.store(false);
        for (const auto &connection : connections_) disconnect(connection);
        connections_.clear();
        for (const auto &item : controls_)
            if (item) item->setProperty("diaryCaptureSuppressed", false);
        controls_.clear();
        if (window_) window_->update();
    }

private:
    struct RenderPhase {
        std::atomic<bool> synced{false}, posted{false}, active{true};
    };
    static void assertGui() {
        Q_ASSERT(qGuiApp && QThread::currentThread() == qGuiApp->thread());
    }
    std::vector<QQuickItem *> discover(bool &valid,
                                       std::vector<QQuickItem *> *scene=nullptr) const {
        std::vector<QQuickItem *> found;
        if (!window_ || !window_->contentItem()) { valid = false; return found; }
        std::vector<QQuickItem *> pending{window_->contentItem()};
        size_t inspected = 0;
        while (!pending.empty()) {
            if (++inspected > 20000) { valid = false; return {}; }
            auto *item = pending.back(); pending.pop_back();
            if (scene) scene->push_back(item);
            const auto owner = item->property("diaryCaptureOwner").toString();
            const bool knownName = item->objectName() == QLatin1String("inkDiaryControl") ||
                item->objectName() == QLatin1String("moveUiProbe");
            if (knownName || !owner.isEmpty()) {
                const int index = item->metaObject()->indexOfProperty("diaryCaptureSuppressed");
                if (!knownName || owner != QLatin1String("ink-diary-owned-control-v1") ||
                    index < 0 || !item->metaObject()->property(index).isWritable() ||
                    item->metaObject()->property(index).metaType().id() != QMetaType::Bool) {
                    valid = false; return {};
                }
                found.push_back(item);
            }
            for (auto *child : item->childItems()) pending.push_back(child);
        }
        return found;
    }
    void rendered() {
        timeout_.stop();
        ready_ = true;
        if (!readyForCapture()) { fail(QStringLiteral("Own capture controls changed before render")); return; }
        // Copy: a callback is allowed to delete/release this session.
        auto callback = notify_;
        if (callback) callback(Event::RenderReady, {});
    }
    void fail(const QString &reason) {
        if (released_) return;
        auto callback = notify_;
        release();
        if (callback) callback(Event::Failed, reason);
    }
    QPointer<QQuickWindow> window_;
    Notify notify_;
    QTimer timeout_;
    std::vector<QPointer<QQuickItem>> controls_;
    std::vector<QMetaObject::Connection> connections_;
    std::shared_ptr<RenderPhase> phase_;
    bool started_ = false, ready_ = false, released_ = false;
};
