#include "../../src/capture-ui.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QTest>
#include <QDir>
#include <cassert>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlComponent rootComponent(&engine);
    rootComponent.setData("import QtQuick 2.15\nItem { property bool documentLoaded: true; width:954; height:1696; Rectangle { objectName: \"thirdPartyToolbar\"; width:60; height:400; color:\"black\" } }", QUrl());
    auto *root = qobject_cast<QQuickItem *>(rootComponent.create());
    assert(root);
    QQuickWindow window;
    window.setColor(Qt::white);
    window.resize(954,1696);
    root->setParentItem(window.contentItem());
    QQmlComponent specimen(&engine, QUrl::fromLocalFile(QDir::current().absoluteFilePath("tools/move-ui-probe/MoveUiProbe.qml")));
    auto create = [&]() {
        auto *item = qobject_cast<QQuickItem *>(specimen.beginCreate(engine.rootContext()));
        if (!item) std::cerr << specimen.errorString().toStdString();
        assert(item);
        item->setParentItem(root); specimen.completeCreate(); return item;
    };
    auto *overlay = create();
    auto *foreign = root->findChild<QQuickItem *>("thirdPartyToolbar");
    assert(foreign);
    window.show();
    assert(QTest::qWaitFor([&] { return window.isExposed(); }));
    QTest::qWait(50);
    const auto before = window.grabWindow();
    assert(!before.isNull());
    int ready = 0, failed = 0;
    auto notify = [&](DiaryCaptureUiSession::Event event, const QString &) {
        event == DiaryCaptureUiSession::Event::RenderReady ? ++ready : ++failed;
    };
    {
        DiaryCaptureUiSession capture(&window, notify);
        capture.begin();
        assert(!overlay->isVisible() && foreign->isVisible());
        assert(!capture.readyForCapture()); // hide alone is not a render acknowledgement
        assert(QTest::qWaitFor([&] { return ready == 1 || failed; }));
        assert(ready == 1 && failed == 0 && capture.readyForCapture());
        const auto hidden = window.grabWindow();
        assert(!hidden.isNull() && hidden != before);
        // The complete owned overlay area is white after actual scene rendering.
        for (int y=1504; y<1680; ++y) for (int x=578; x<938; ++x)
            assert(hidden.pixelColor(x,y) == QColor(Qt::white));
        assert(hidden.pixelColor(10,10) == QColor(Qt::black)); // third-party toolbar untouched
        // Concurrent acquisition cannot restore someone else's hidden state.
        int competingFailures=0;
        DiaryCaptureUiSession competing(&window,[&](auto event, const auto &) {
            assert(event == DiaryCaptureUiSession::Event::Failed); ++competingFailures;
        });
        competing.begin();
        assert(competingFailures == 1 && !overlay->isVisible() && capture.readyForCapture());
        // Application state changes while hidden must survive restoration.
        overlay->setProperty("simulatedBusy",true);
        root->setProperty("documentLoaded",false);
        capture.release(); capture.release();
        assert(!overlay->property("diaryCaptureSuppressed").toBool() && !overlay->isVisible());
        root->setProperty("documentLoaded",true);
        assert(overlay->isVisible() && overlay->property("simulatedBusy").toBool());
        root->setWidth(319); assert(!overlay->isVisible());
        root->setWidth(954); assert(overlay->isVisible());
    }
    // Cancellation before frame completion must neither emit stale ready nor
    // lose the original QML visibility expression.
    const int beforeReady = ready, beforeFailed = failed;
    {
        DiaryCaptureUiSession capture(&window,notify);
        capture.begin(); capture.release();
        assert(overlay->isVisible());
        QTest::qWait(50);
        assert(ready == beforeReady && failed == beforeFailed);
    }
    // RAII error unwind/teardown before the queued render acknowledgement.
    {
        auto capture = std::make_unique<DiaryCaptureUiSession>(&window,notify);
        capture->begin(); capture.reset();
        QTest::qWait(50);
        assert(overlay->isVisible() && ready == beforeReady && failed == beforeFailed);
    }
    // A newly-created known control invalidates a previous acknowledgement.
    try {
        DiaryCaptureUiSession capture(&window,notify);
        capture.begin(); assert(QTest::qWaitFor([&] { return capture.readyForCapture(); }));
        throw std::runtime_error("simulated screenshot failure");
    } catch (const std::runtime_error &) {
        assert(overlay->isVisible() && !overlay->property("diaryCaptureSuppressed").toBool());
    }
    // A newly-created known control invalidates a previous acknowledgement.
    {
        DiaryCaptureUiSession capture(&window,notify);
        capture.begin(); assert(QTest::qWaitFor([&] { return capture.readyForCapture(); }));
        const int prior = failed;
        auto *newOverlay = create();
        assert(failed == prior+1 && !capture.readyForCapture() && newOverlay->isVisible());
        capture.release(); assert(overlay->isVisible()); delete newOverlay;
        assert(!capture.readyForCapture()); // transient replacement cannot revive readiness
    }
    // Destroying a page/control after render causes Failed and releases peers.
    {
        DiaryCaptureUiSession capture(&window,notify);
        capture.begin(); assert(QTest::qWaitFor([&] { return capture.readyForCapture(); }));
        const int prior = failed;
        delete overlay; overlay = nullptr;
        assert(failed == prior+1 && !capture.readyForCapture());
    }
    overlay = create();
    // Completion callbacks may destroy their GUI-owned session immediately.
    {
        std::unique_ptr<DiaryCaptureUiSession> capture;
        capture = std::make_unique<DiaryCaptureUiSession>(&window,[&](auto event, const auto &) {
            assert(event == DiaryCaptureUiSession::Event::RenderReady);
            capture.reset();
        });
        capture->begin();
        assert(QTest::qWaitFor([&] { return !capture; }));
        assert(overlay->isVisible());
    }
    // Incompatible named overlays fail closed without partial suppression.
    {
        auto *unknown = new QQuickItem(root);
        unknown->setObjectName("inkDiaryControl");
        const int prior = failed;
        DiaryCaptureUiSession capture(&window,notify); capture.begin();
        assert(failed == prior+1 && overlay->isVisible());
        delete unknown;
    }
    // No render signal: a bounded async timeout restores the owned controls.
    {
        const int prior = failed;
        window.blockSignals(true);
        DiaryCaptureUiSession capture(&window,notify); capture.begin(25);
        assert(QTest::qWaitFor([&] { return failed == prior+1; }));
        assert(overlay->isVisible() && !capture.readyForCapture());
        window.blockSignals(false);
    }
    // Window destruction while waiting does not access dead items or deliver ready.
    {
        auto *other = new QQuickWindow;
        other->resize(954,1696); other->show();
        auto *item = create(); item->setParentItem(other->contentItem());
        const int prior = failed;
        DiaryCaptureUiSession capture(other,notify); capture.begin();
        delete other;
        assert(failed == prior+1 && !capture.readyForCapture());
    }
    delete root;
    app.processEvents();
    std::cout << "PASS: own-only suppression, real hidden render, binding-preserving restore, concurrent lease rejection, cancellation/error/teardown, new-control rejection, async timeout and lifetime safety\n";
}
