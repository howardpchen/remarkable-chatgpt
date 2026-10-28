#include "../../src/gui-dispatch.h"
#include <QTimer>
#include <cassert>
#include <thread>
#include <iostream>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    assert(diaryDispatch::gate()->installed.load());
    if(argc>1) {
        bool observed=false;
        QObject::connect(&app,&QCoreApplication::aboutToQuit,[&]{
            assert(!QCoreApplication::closingDown());
            assert(diaryDispatch::gate()->closed.load());
            auto late=diaryDispatch::run([]{assert(false);return QJsonObject{};});
            assert(late.value("ok")==false);observed=true;
        });
        QTimer::singleShot(0,&app,&QCoreApplication::quit);
        assert(app.exec()==0 && observed);return 0;
    }
    int calls=0;
    auto direct=diaryDispatch::run([&]{++calls;return QJsonObject{{"value",7}};});
    assert(direct.value("value").toInt()==7 && calls==1);
    QJsonObject timed;
    std::thread timeout([&]{timed=diaryDispatch::run([&]{++calls;return QJsonObject{};},20);});
    timeout.join();
    assert(timed.value("error")=="gui-dispatch-timeout-cancelled");
    QCoreApplication::sendPostedEvents();assert(calls==1);
    QJsonObject normal;std::atomic<bool> complete{false};
    std::thread success([&]{normal=diaryDispatch::run([]{return QJsonObject{{"value",9}};});complete=true;});
    while(!complete.load()) {QCoreApplication::processEvents();std::this_thread::yield();}
    success.join();assert(normal.value("value").toInt()==9);
    // Prove timeout while work runs does not leave references to a dead caller.
    std::atomic<bool> began{false},returned{false};QJsonObject flight;
    std::thread running([&]{flight=diaryDispatch::run([&]{
        began=true;
        while(!returned.load())std::this_thread::yield();
        return QJsonObject{{"finished",true}};
    },30);returned=true;});
    while(!began.load())QCoreApplication::processEvents();
    running.join();assert(flight.value("error")=="gui-dispatch-timeout-in-flight-do-not-retry");
    // Queue a command, then emit aboutToQuit before its GUI callback can execute.
    QJsonObject stopped;std::atomic<bool> queued{false};
    QTimer::singleShot(0,&app,[&]{
        std::thread pending([&]{queued=true;stopped=diaryDispatch::run([&]{++calls;return QJsonObject{};});});
        bool registered=false;
        while(!registered) {
            auto state=diaryDispatch::gate();
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                for(auto &weak:state->pending) if(auto job=weak.lock()) {
                    std::lock_guard<std::mutex> jl(job->mutex);
                    if(!job->started && !job->done)registered=true;
                }
            }
            std::this_thread::yield();
        }
        // close() is the exact aboutToQuit handler; notify pending waiters now.
        diaryDispatch::gate()->close();
        pending.join();
        app.quit();
    });
    assert(app.exec()==0);
    assert(calls==1);
    assert(stopped.value("error").toString().startsWith("application-stopping"));
    auto late=diaryDispatch::run([&]{++calls;return QJsonObject{};});
    assert(late.value("ok")==false && calls==1);
    std::cout<<"Dispatch checks passed: direct, queued, pending timeout, in-flight timeout, shutdown cancellation and late rejection\n";
}
