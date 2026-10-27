#include "admission.h"
#include <QTemporaryDir>
#include <cassert>
#include <iostream>
struct FakeNative final:moveDemo::NativePoseSource {
    QString error;
    int calls=0;
    QString verifyLivePose(QQuickWindow *,const QRect &crop, bool) override {
        ++calls;assert(crop==QRect(120,112,834,1584));return error;
    }
};
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    QQuickWindow window;window.resize(954,1696);window.show();
    QQuickItem document(window.contentItem());document.setProperty("documentLoaded",true);
    document.setProperty("currentPageId","fixture-page");
    QQuickItem panel(window.contentItem());panel.setObjectName("moveUiProbe");
    panel.setPosition({578,1504});panel.setSize({360,176});
    panel.setProperty("diaryCaptureOwner","ink-diary-owned-control-v1");
    panel.setProperty("diaryCaptureSuppressed",false);
    assert(moveDemo::inspectOwnControl(&window,false).isEmpty());
    panel.setWidth(96);assert(!moveDemo::inspectOwnControl(&window,false).isEmpty());panel.setWidth(360);
    panel.setX(577);assert(!moveDemo::inspectOwnControl(&window,false).isEmpty());panel.setX(578);
    panel.setVisible(false);assert(!moveDemo::inspectOwnControl(&window,false).isEmpty());panel.setVisible(true);
    {
        QQuickItem unknown(window.contentItem());unknown.setProperty("diaryCaptureOwner","other");
        assert(!moveDemo::inspectOwnControl(&window,false).isEmpty());
    }
    const QJsonObject viewport{{"templateName","Blank"},{"width",954},{"height",1696},
        {"windowWidth",954},{"windowHeight",1696},{"devicePixelRatio",1},
        {"points",QJsonArray{QJsonArray{0,0},QJsonArray{100,0},QJsonArray{0,100}}}};
    QTemporaryDir dir;assert(dir.isValid());
    assert(QFile::setPermissions(dir.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));
    moveDemo::Admission missing(&window,&document,viewport,nullptr);
    assert(missing.startInDirectory(viewport,dir.path())=="live-native-pose-source-required");
    assert(!QFile::exists(dir.path()+"/START_CONSUMED"));
    FakeNative native; native.error="unknown-native-overlay";
    moveDemo::Admission blocked(&window,&document,viewport,&native);
    assert(blocked.startInDirectory(viewport,dir.path())==native.error);
    assert(!QFile::exists(dir.path()+"/START_CONSUMED"));
    native.error.clear();moveDemo::Admission demo(&window,&document,viewport,&native);
    assert(demo.startInDirectory(viewport,dir.path()).isEmpty());
    assert(QFile::exists(dir.path()+"/START_CONSUMED"));
    assert(!demo.captureBoundary(viewport).isEmpty()); // still visible: terminal
    panel.setProperty("diaryCaptureSuppressed",true);panel.setVisible(false);
    assert(!demo.captureBoundary(viewport).isEmpty()); // cannot revive failed attempt
    panel.setProperty("diaryCaptureSuppressed",false);panel.setVisible(true);
    moveDemo::Admission afterReload(&window,&document,viewport,&native);
    assert(!afterReload.startInDirectory(viewport,dir.path()).isEmpty());
    QTemporaryDir second;
    moveDemo::Admission good(&window,&document,viewport,&native);
    assert(good.startInDirectory(viewport,second.path()).isEmpty());
    panel.setProperty("diaryCaptureSuppressed",true);panel.setVisible(false);
    assert(good.captureBoundary(viewport).isEmpty());assert(good.captureBoundary(viewport).isEmpty());
    native.error="native-toolbar-pose-changed";
    assert(good.captureBoundary(viewport)==native.error);
    panel.setProperty("diaryCaptureSuppressed",false);panel.setVisible(true);
    QTemporaryDir third;native.error.clear();
    moveDemo::Admission cancelled(&window,&document,viewport,&native);
    assert(cancelled.startInDirectory(viewport,third.path()).isEmpty());cancelled.cancel();
    assert(cancelled.phase()==moveDemo::Admission::Phase::Cancelled);
    assert(!cancelled.startInDirectory(viewport,third.path()).isEmpty());
    assert(!cancelled.captureBoundary(viewport).isEmpty());
    QTemporaryDir pageChange;moveDemo::Admission changed(&window,&document,viewport,&native);
    document.setProperty("currentPageId","different-page");
    assert(changed.startInDirectory(viewport,pageChange.path())=="document-or-page-changed");
    document.setProperty("currentPageId","fixture-page");
    QTemporaryDir fourth;assert(QFile::setPermissions(fourth.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup));
    assert(moveDemo::claimAttempt(fourth.path())=="attempt-directory-not-private");
    std::cout<<"demo admission: exact panel, absent/native failures, persistent claim, capture boundaries, cancellation passed\n";
}
