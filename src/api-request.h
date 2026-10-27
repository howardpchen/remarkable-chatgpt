#pragma once
#include <QNetworkRequest>
// Shared endpoint/header policy. Dispatch authorization remains with each caller.
inline QNetworkRequest diaryOpenAiRequest(const QString &key,bool imageGeneration=false) {
    QNetworkRequest request(QUrl(imageGeneration ? "https://api.openai.com/v1/images/generations" : "https://api.openai.com/v1/responses"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setRawHeader("Authorization","Bearer "+key.toUtf8());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
    return request;
}
