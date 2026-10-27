#pragma once
// Host-only synthetic transport; createRequest never opens a socket.
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cstring>
#include <functional>
inline QByteArray mockAnswer(QString text="A short synthetic answer.",QString kind="text") {
    auto inner=QString::fromUtf8(QJsonDocument(QJsonObject{{"kind",kind},{"text",text}}).toJson(QJsonDocument::Compact));
    return QJsonDocument(QJsonObject{{"status","completed"},{"output",QJsonArray{QJsonObject{
        {"type","message"},{"role","assistant"},{"status","completed"},
        {"content",QJsonArray{QJsonObject{{"type","output_text"},{"text",inner}}}}}}}}).toJson(QJsonDocument::Compact);
}
class LiveMockReply final:public QNetworkReply {
public:
    QByteArray bytes; qint64 pos=0;int &aborts;
    LiveMockReply(const QNetworkRequest &req,QByteArray body,int delay,int code,int &count,QObject *parent)
        :QNetworkReply(parent),bytes(std::move(body)),aborts(count) {
        setRequest(req);setUrl(req.url());setAttribute(QNetworkRequest::HttpStatusCodeAttribute,code);
        open(QIODevice::ReadOnly|QIODevice::Unbuffered);
        if(delay==-2)setFinished(true);
        else if(delay>=0) QTimer::singleShot(delay,this,[this]{if(isFinished())return;setFinished(true);emit readyRead();emit finished();});
    }
    void abort() override {if(isFinished())return;++aborts;setFinished(true);setError(OperationCanceledError,"cancelled");emit finished();}
    qint64 bytesAvailable() const override {return bytes.size()-pos+QNetworkReply::bytesAvailable();}
    qint64 readData(char *data,qint64 max) override {
        auto n=qMin(max,qint64(bytes.size())-pos);if(n<=0)return -1;
        std::memcpy(data,bytes.constData()+pos,size_t(n));pos+=n;return n;
    }
};
class LiveMockManager final:public QNetworkAccessManager {
public:
    int requests=0,aborts=0,delay=30,code=200;
    QByteArray response=mockAnswer(),sent;
    QNetworkRequest request;
    std::function<void()> onDispatch;
protected:
    QNetworkReply *createRequest(Operation op,const QNetworkRequest &req,QIODevice *data) override {
        Q_ASSERT(op==PostOperation);++requests;request=req;sent=data->readAll();
        if(onDispatch)onDispatch();
        return new LiveMockReply(req,response,delay,code,aborts,this);
    }
};
