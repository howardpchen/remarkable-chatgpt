#pragma once
// Shared broker-to-GUI admission. Jobs own their captures across timeout/teardown.
#include <QCoreApplication>
#include <QJsonObject>
#include <QThread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <algorithm>

namespace diaryDispatch {
inline QJsonObject error(const char *reason) { return {{"ok",false},{"error",reason}}; }
struct Job {
    std::mutex mutex;
    std::condition_variable changed;
    bool started=false, done=false;
    QJsonObject result;
    std::function<QJsonObject()> work;
};
struct Gate {
    std::atomic<bool> installed{false}, closed{false};
    std::mutex mutex;
    std::vector<std::weak_ptr<Job>> pending;
    void close() {
        closed.store(true);
        std::lock_guard<std::mutex> lock(mutex);
        for(auto &weak:pending) if(auto job=weak.lock()) {
            std::lock_guard<std::mutex> jl(job->mutex);
            if(!job->started && !job->done) {
                job->done=true; job->result=error("application-stopping");
                job->work={}; job->changed.notify_all();
            }
        }
    }
};
inline std::shared_ptr<Gate> gate() {
    // Deliberately process-lifetime: worker callers can outlive app destruction.
    static const auto *state=new std::shared_ptr<Gate>(std::make_shared<Gate>());
    return *state;
}
inline void install() {
    auto *app=QCoreApplication::instance();
    auto state=gate();
    if(!app || QThread::currentThread()!=app->thread() || state->installed.load())return;
    QObject::connect(app,&QCoreApplication::aboutToQuit,app,[state]{state->close();},Qt::DirectConnection);
    QObject::connect(app,&QObject::destroyed,[state]{state->close();});
    state->installed.store(true);
}
Q_COREAPP_STARTUP_FUNCTION(install)

inline QJsonObject run(std::function<QJsonObject()> work,int timeoutMs=5000) {
    auto state=gate();
    if(!state->installed.load() || state->closed.load())return error("application-stopping-or-not-ready");
    auto *app=QCoreApplication::instance();
    if(!app || QCoreApplication::closingDown())return error("application-stopping");
    if(QThread::currentThread()==app->thread()) {
        if(state->closed.load())return error("application-stopping");
        return work();
    }
    auto job=std::make_shared<Job>(); job->work=std::move(work);
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        if(state->closed.load())return error("application-stopping");
        state->pending.erase(std::remove_if(state->pending.begin(),state->pending.end(),
            [](const std::weak_ptr<Job> &p){return p.expired();}),state->pending.end());
        state->pending.push_back(job);
        // Context destruction cancels queued delivery; close() wakes its caller.
        if(!QMetaObject::invokeMethod(app,[state,job]{
            std::function<QJsonObject()> fn;
            {
                std::lock_guard<std::mutex> lock(job->mutex);
                if(job->done)return;
                if(state->closed.load()) {
                    job->done=true; job->result=error("application-stopping");
                    job->work={}; job->changed.notify_all(); return;
                }
                job->started=true; fn=std::move(job->work);
            }
            QJsonObject result;
            try {result=fn();} catch(...) {result=error("gui-job-failed");}
            std::lock_guard<std::mutex> lock(job->mutex);
            job->result=std::move(result); job->done=true; job->changed.notify_all();
        },Qt::QueuedConnection))return error("gui-unavailable");
    }
    std::unique_lock<std::mutex> lock(job->mutex);
    if(!job->changed.wait_for(lock,std::chrono::milliseconds(timeoutMs),[&]{return job->done;})) {
        // Never abandon reference captures or replay a possibly executing write.
        if(job->started)return error("gui-dispatch-timeout-in-flight-do-not-retry");
        job->done=true; job->work={}; job->result=error("gui-dispatch-timeout-cancelled");
    }
    return job->result;
}
}
