#pragma once
#ifndef INK_PAPER_PRO_LIVE_TEST
#error "This client belongs only to the separate bounded live-test artifact"
#endif
#include "config.h"
#include "../../../src/response.h"
#include "capture-layout.h"
#include <QBuffer>
#include <QFile>
#include <QJsonDocument>
#include "../../../src/api-request.h"
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QTimer>
#include <QRegularExpression>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <functional>

namespace paperProLive {
inline constexpr int outputTokens=800;
inline QJsonObject payload(const QImage &image) {
    if(!diaryPaperProReadingSizeSupported(image.size())) throw std::runtime_error("Unexpected live question dimensions");
    QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);
    if(!image.save(&buffer,"PNG") || png.size()>8*1024*1024) throw std::runtime_error("Question encoding failed");
    QJsonObject fields{{"kind",QJsonObject{{"type","string"},{"enum",QJsonArray{"text","needs_web","clarification","unsupported"}}}},
        {"text",QJsonObject{{"type","string"}}}};
    QJsonObject format{{"type","json_schema"},{"name","paper_pro_text_answer"},{"strict",true},
        {"schema",QJsonObject{{"type","object"},{"properties",fields},
            {"required",QJsonArray{"kind","text"}},{"additionalProperties",false}}}};
    return {{"model",model},{"store",false},{"max_output_tokens",outputTokens},
        {"service_tier","default"},
        {"reasoning",QJsonObject{{"effort","none"}}},
        {"instructions","Read the latest unanswered handwritten question. Earlier writing may contain prior answers. "
            "Give a concise ordinary text answer in the question's language, usually under 100 words. "
            "If the latest question is missing, unreadable or ambiguous, return kind=clarification; do not guess or answer an earlier question. "
            "For answerable questions use kind=text and simple Markdown, no HTML or external image URLs. "
            "This test has no tools. Requests needing current or latest facts, online verification, source lookup or search "
            "MUST return kind=needs_web, not an unverified answer. Requests for generated images, plots, diagrams "
            "or templates MUST return kind=unsupported. For every non-text kind set text to an empty string; "
            "the app displays its own explanation. Never claim to have searched or generated an image."},
        {"input",QJsonArray{QJsonObject{{"role","user"},{"content",QJsonArray{
            QJsonObject{{"type","input_text"},{"text","Answer the latest unanswered question on this page."}},
            QJsonObject{{"type","input_image"},{"detail","high"},
                {"image_url","data:image/png;base64,"+QString::fromLatin1(png.toBase64())}}
        }}}}}, {"text",QJsonObject{{"format",format}}}};
}
struct ParsedAnswer { QString text; QString notice; };
inline ParsedAnswer parse(const QJsonObject &root) {
    // Reject refusals/tool activity, rather than rendering their content as an answer.
    int messages=0,parts=0;
    for(const auto &value:root.value("output").toArray()) {
        auto item=value.toObject();
        if(item.value("type")!="message" || item.value("role")!="assistant" ||
           item.value("status")!="completed") throw std::runtime_error("Unexpected API output item");
        ++messages;
        for(const auto &part:item.value("content").toArray()) {
            if(part.toObject().value("type")!="output_text") throw std::runtime_error("API refusal or unsupported output");
            ++parts;
        }
    }
    if(messages!=1 || parts!=1) throw std::runtime_error("Expected one text answer");
    QJsonParseError error;
    auto doc=QJsonDocument::fromJson(responseText(root).toUtf8(),&error);
    auto answer=doc.object();
    if(error.error!=QJsonParseError::NoError || !doc.isObject() || answer.size()!=2 ||
       !answer.value("kind").isString() || !answer.value("text").isString())
        throw std::runtime_error("Malformed text answer");
    const auto kind=answer.value("kind").toString();
    // Fixed local notices: never log provider question transcriptions or insert
    // unsupported/uncertain results into the notebook. No retries or extra calls.
    if(kind=="needs_web")return {{},"Needs web access; search is off."};
    if(kind=="clarification")return {{},"Cannot read question. Please rewrite."};
    if(kind=="unsupported")return {{},"Text answers only; try another question."};
    if(kind!="text")throw std::runtime_error("Unknown answer kind");
    auto text=answer.value("text").toString().trimmed();
    if(text.isEmpty() || text.size()>4000) throw std::runtime_error("Empty or oversized text answer");
    return {text,{}};
}
// Only numeric transport metadata and allowlisted provider codes may reach UI/logs.
// Never copy errorString(), response messages, headers, URLs or response bodies.
inline QString transportFailure(int status,QNetworkReply::NetworkError networkError,const QByteArray &body) {
    QString result=status>0 ? QString("OpenAI HTTP %1").arg(status) : QString("OpenAI connection failure (no HTTP response)");
    result+=QString("; Qt network error %1").arg(int(networkError));
    const auto code=QJsonDocument::fromJson(body).object().value("error").toObject().value("code").toString();
    for(const auto allowed:{"rate_limit_exceeded","insufficient_quota","invalid_api_key","model_not_found",
                           "unsupported_parameter","invalid_value","server_error"}) {
        if(code==QLatin1String(allowed)) {result+="; "+code;break;}
    }
    return result+"; no retry";
}
// GUI-thread asynchronous transport: no nested event loop, detached worker,
// retries, redirect following, insecure TLS fallback, or notebook persistence.
class Client final : public QObject {
public:
    std::function<bool()> dispatchGuard;
    std::function<void(const QJsonObject &)> usageObserver;
    std::function<void(int,const QString &)> providerObserver;
    using Done=std::function<void(QString,QString)>;
    explicit Client(QObject *parent):QObject(parent) {
        timeout_.setSingleShot(true);
        connect(&timeout_,&QTimer::timeout,this,[this]{finish({},"OpenAI request timed out; no retry");});
    }
    ~Client() override {cancel();}
    void cancel() {
        timeout_.stop();done_={};valid_={};
        if(reply_) {auto *r=reply_.data();reply_.clear();disconnect(r,nullptr,this,nullptr);r->abort();r->deleteLater();
#ifdef INK_PAPER_PRO_PER_REQUEST
            qInfo("ink-paper-pro lifecycle reply-abort-complete");
#endif
        }
        bytes_.clear();inferenceBody_.clear();inferenceRequest_=QNetworkRequest();
    }
    void start(const QImage &image,const QString &key,std::function<bool()> valid,Done done,
               QNetworkAccessManager *injected=nullptr,int timeoutMs=120000) {
        if(used_) {done({},"Live API attempt already consumed");return;}
        used_=true;valid_=std::move(valid);done_=std::move(done);
        QByteArray body;
        try {body=QJsonDocument(payload(image)).toJson(QJsonDocument::Compact);}
        catch(const std::exception &e){finish({},QString::fromUtf8(e.what()));return;}
        if(!valid_()) {finish({},"Cancelled or page changed before API dispatch");return;}
        auto request=diaryOpenAiRequest(key);
        auto *manager=injected;
        if(!manager) manager=new QNetworkAccessManager(this);
        // Persist uncertainty BEFORE post: a crash can retain a hold, never
        // refund a request that might have reached the provider.
        if(dispatchGuard && !dispatchGuard()) {finish({},"Accounting dispatch guard unavailable; no request sent");return;}
        // Count this exact frozen payload before the ONE inference. Counting is
        // a distinct preflight, not an inference retry. Hold stays consumed on
        // uncertainty/cancel; do not release it after any provider dispatch.
        inferenceRequest_=request;inferenceBody_=body;manager_=manager;
        countPhase_=true;timeoutMs_=timeoutMs;
        QJsonObject count;
        const auto frozen=QJsonDocument::fromJson(body).object();
        for(const auto *field:{"model","input","instructions","text","reasoning"})
            if(frozen.contains(field))count.insert(field,frozen.value(field));
        auto countRequest=request;
        countRequest.setUrl(QUrl("https://api.openai.com/v1/responses/input_tokens"));
        post(countRequest,QJsonDocument(count).toJson(QJsonDocument::Compact));
    }
private:
    void post(const QNetworkRequest &request,const QByteArray &body) {
        reply_=manager_->post(request,body);
        reply_->setReadBufferSize(65536);
        connect(reply_,&QNetworkReply::readyRead,this,[this]{drain();});
        connect(reply_,&QNetworkReply::finished,this,[this]{complete();});
        timeout_.start(timeoutMs_);
        if(reply_->isFinished())QTimer::singleShot(0,this,[this]{if(reply_)complete();});
    }

    bool drain() {
        if(!reply_)return false;
        while(reply_->bytesAvailable()>0) {
            auto chunk=reply_->read(65536);
            if(bytes_.size()+chunk.size()>1024*1024) {finish({},"OpenAI response too large");return false;}
            if(chunk.isEmpty())break;
            bytes_+=chunk;
        }
        return true;
    }
    void complete() {
#ifdef INK_PAPER_PRO_PER_REQUEST
        qInfo("ink-paper-pro lifecycle reply-finished");
#endif
        if(!drain())return;
        const int httpStatus=reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(providerObserver) providerObserver(httpStatus,
            QJsonDocument::fromJson(bytes_).object().value("error").toObject().value("code").toString());
        if(httpStatus!=200 || reply_->error()!=QNetworkReply::NoError) {
            const auto failure=transportFailure(httpStatus,reply_->error(),bytes_);
#ifdef INK_PAPER_PRO_PER_REQUEST
            qInfo("ink-paper-pro transport %s",qPrintable(failure));
#endif
            finish({},failure);return;
        }
        try {
            QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes_,&error);
            if(error.error!=QJsonParseError::NoError || !doc.isObject())throw std::runtime_error("Invalid API JSON");
            if(countPhase_) {
                const auto count=doc.object().value("input_tokens");
                const double n=count.toDouble(-1);
                // Same documented worst-case cache-write, long-context and
                // regional rates as Move; no cached-token discount is assumed.
                // Include 1024 tokens of extra headroom beyond provider count.
                if(doc.object().value("object")!="response.input_tokens" || !count.isDouble() ||
                   !std::isfinite(n) || n<1 || n>922000 || std::floor(n)!=n ||
                   (qint64(n)+1024)*275+800LL*825>100000000LL)
                    throw std::runtime_error("Input token bound exceeds the authorized $0.10 test cap");
                if(!valid_ || !valid_())throw std::runtime_error("Page changed during token preflight");
                auto *old=reply_.data();reply_.clear();disconnect(old,nullptr,this,nullptr);old->deleteLater();
                bytes_.clear();timeout_.stop();countPhase_=false;
                post(inferenceRequest_,inferenceBody_);
                return;
            }
            if(usageObserver)usageObserver(doc.object());
            if(!valid_ || !valid_()) {finish({},"Cancelled or page changed during API request");return;}
            const auto answer=parse(doc.object());
            finish(answer.text,answer.notice);
        } catch(const std::exception &e) {finish({},QString::fromUtf8(e.what()));}
    }
    void finish(QString text,QString error) {
        auto done=std::move(done_);cancel();if(done)done(std::move(text),std::move(error));
    }
    QPointer<QNetworkReply> reply_;QTimer timeout_;QByteArray bytes_;
    QPointer<QNetworkAccessManager> manager_;
    QNetworkRequest inferenceRequest_;QByteArray inferenceBody_;
    bool countPhase_=false;int timeoutMs_=120000;
    bool used_=false;Done done_;std::function<bool()> valid_;
};
}
