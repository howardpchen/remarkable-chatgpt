#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QTimer>
#include <functional>
#include <memory>
#include <stdexcept>
// Manager injection keeps cancellation tests offline and lets them assert that no
// request is dispatched after its page/context has become invalid.
inline QJsonObject diaryApiRequestWithManager(QNetworkAccessManager &manager,
                                  const QJsonObject &payload,const QString &key,
                                  const std::function<bool()> &valid, bool imageGeneration=false) {
    QNetworkRequest request(QUrl(imageGeneration ? "https://api.openai.com/v1/images/generations" : "https://api.openai.com/v1/responses"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setRawHeader("Authorization", "Bearer " + key.toUtf8());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
    const auto body=QJsonDocument(payload).toJson(QJsonDocument::Compact);
    // Serialization can be nontrivial for a screenshot: validate at the actual
    // dispatch boundary, not only before preparing the request.
    if(!valid())throw std::runtime_error("Cancelled or page changed");
    // Own the reply within this call even when an injected manager outlives it.
    // This also disconnects callbacks that capture the local response buffer.
    std::unique_ptr<QNetworkReply> ownedReply(manager.post(request,body));
    auto *reply=ownedReply.get();
    QEventLoop loop; QTimer timeout,cancelPoll;
    timeout.setSingleShot(true);
    QObject::connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    QObject::connect(&timeout,&QTimer::timeout,&loop,[&]{reply->abort();loop.quit();});
    QObject::connect(&cancelPoll,&QTimer::timeout,reply,[&]{if(!valid())reply->abort();});
    const qint64 limit=imageGeneration ? 32*1024*1024 : 1024*1024;
    QByteArray bytes;bool oversized=false;
    auto drain=[&] { auto chunk=reply->readAll();if(bytes.size()+chunk.size()>limit){oversized=true;reply->abort();}else bytes+=chunk; };
    reply->setReadBufferSize(65536);
    QObject::connect(reply,&QNetworkReply::readyRead,reply,drain);
    timeout.start(imageGeneration ? 300000 : 120000);cancelPoll.start(300);
    // Some backends can finish inside post(), before the signal is connected.
    if(!reply->isFinished())loop.exec();
    timeout.stop();cancelPoll.stop();
    if(!oversized)drain();
    if(oversized)throw std::runtime_error("Response too large");
    if(!valid())throw std::runtime_error("Cancelled or page changed");
    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(code!=200||reply->error()!=QNetworkReply::NoError)
        throw std::runtime_error(QString("OpenAI request failed (HTTP %1); check Wi-Fi, key and model/tool support").arg(code).toStdString());
    auto doc=QJsonDocument::fromJson(bytes);
    if(!doc.isObject())throw std::runtime_error("Invalid API response");
    return doc.object();
}

inline QJsonObject diaryApiRequest(const QJsonObject &payload,const QString &key,
                                  const std::function<bool()> &valid, bool imageGeneration=false) {
    QNetworkAccessManager manager;
    return diaryApiRequestWithManager(manager,payload,key,valid,imageGeneration);
}
