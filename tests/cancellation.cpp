// Offline transport regression: the production API dispatch path, no HTTP access.
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <cstring>
#include <iostream>
#include "../src/api.h"

class MockReply final : public QNetworkReply {
    QByteArray body=R"({"ok":true})";
    qint64 position=0;
public:
    int &abortCount;
    MockReply(const QNetworkRequest &request, bool complete, bool immediate, int &aborts, QObject *parent)
        :QNetworkReply(parent),abortCount(aborts) {
        setRequest(request);setUrl(request.url());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute,200);
        open(QIODevice::ReadOnly|QIODevice::Unbuffered);
        if(immediate)setFinished(true);
        else if(complete)QTimer::singleShot(0,this,[this] {setFinished(true);emit readyRead();emit finished();});
    }
    void abort() override {
        if(isFinished())return;
        ++abortCount;setError(QNetworkReply::OperationCanceledError,"Cancelled");
        setFinished(true);emit finished();
    }
    qint64 bytesAvailable() const override {return body.size()-position+QNetworkReply::bytesAvailable();}
protected:
    qint64 readData(char *data,qint64 limit) override {
        const qint64 count=qMin(limit,qint64(body.size())-position);
        if(count<=0)return -1;
        std::memcpy(data,body.constData()+position,size_t(count));position+=count;return count;
    }
};
class MockManager final : public QNetworkAccessManager {
public:
    int requests=0;
    bool complete=true, immediate=false;
    int aborts=0;
    QUrl dispatchedUrl;
protected:
    QNetworkReply *createRequest(Operation operation,const QNetworkRequest &request,QIODevice *) override {
        if(operation!=PostOperation)throw std::runtime_error("Expected POST");
        ++requests;dispatchedUrl=request.url();
        return new MockReply(request,complete,immediate,aborts,this);
    }
};
static void check(bool condition,const char *message) {if(!condition)throw std::runtime_error(message);}
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    try {
#ifdef INK_DIARY_EXPERIMENTAL_MOVE
        for(bool images:{false,true}) {
            MockManager manager;
            bool rejected=false,validCalled=false;
            try {diaryApiRequestWithManager(manager,{},"offline-placeholder",[&]{validCalled=true;return true;},images);}
            catch(const std::runtime_error &error) {rejected=QString::fromUtf8(error.what()).contains("offline demo-only");}
            check(rejected,"Experimental artifact allowed API entry");
            check(!validCalled && manager.requests==0,"Experimental artifact reached dispatch preparation");
        }
        std::cout<<"PASS: experimental Move artifact rejects Responses/Images before dispatch independently of config\n";
#else
        // Both Responses and Images use the same guarded dispatch boundary.
        for(bool images:{false,true}) {
            MockManager manager;
            bool rejected=false;
            try {diaryApiRequestWithManager(manager,{},"offline-placeholder",[]{return false;},images);}
            catch(const std::runtime_error &error) {rejected=QString::fromUtf8(error.what()).contains("Cancelled");}
            check(rejected,"Invalid request was not rejected");
            check(manager.requests==0,"Invalid request dispatched notebook data");
        }
        {
            MockManager manager;
            auto result=diaryApiRequestWithManager(manager,{},"offline-placeholder",[]{return true;});
            check(result.value("ok").toBool(),"Valid mock response failed");
            check(manager.requests==1,"Valid request was not dispatched exactly once");
            check(manager.dispatchedUrl.path()=="/v1/responses","Incorrect endpoint");
        }
        {
            // Finished before post() returns: there will be no later finished signal.
            MockManager manager;manager.immediate=true;
            auto result=diaryApiRequestWithManager(manager,{},"offline-placeholder",[]{return true;});
            check(result.value("ok").toBool(),"Immediately finished reply failed");
            check(manager.requests==1,"Immediate reply dispatched more than once");
            check(manager.findChildren<QNetworkReply *>().isEmpty(),"Reply outlived request scope");
        }
        {
            MockManager manager;manager.complete=false;
            bool valid=true,rejected=false;
            QTimer::singleShot(0,&app,[&]{valid=false;});
            try {diaryApiRequestWithManager(manager,{},"offline-placeholder",[&]{return valid;});}
            catch(const std::runtime_error &error) {rejected=QString::fromUtf8(error.what()).contains("Cancelled");}
            check(rejected,"In-flight cancellation not reported");
            check(manager.requests==1 && manager.aborts==1,"In-flight request not aborted");
        }
        std::cout<<"Offline API cancellation tests passed\n";
#endif
        return 0;
    } catch(const std::exception &error) {std::cerr<<error.what()<<'\n';return 1;}
}
