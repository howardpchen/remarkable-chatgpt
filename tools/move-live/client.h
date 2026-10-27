#pragma once
#ifndef INK_MOVE_LIVE_TEST
#error "This client belongs only to the separate bounded live-test artifact"
#endif
#include "../../src/response.h"
#include <QBuffer>
#include <QFile>
#include <QJsonDocument>
#include "../../src/api-request.h"
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

namespace moveLive {
inline constexpr int outputTokens=800;
inline constexpr auto model="gpt-4.1-mini";
// Open only the expected private regular file, without following a symlink.
// No credential contents enter status, exceptions, evidence, or logs.
inline QString loadKey(const QString &path) {
    int fd=::open(QFile::encodeName(path).constData(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK);
    if(fd<0) throw std::runtime_error("Private API configuration missing");
    struct stat st{};
    if(::fstat(fd,&st)!=0 || !S_ISREG(st.st_mode) || st.st_uid!=::geteuid() ||
       (st.st_mode&0777)!=0600 || st.st_nlink!=1 || st.st_size<1 || st.st_size>16384) {
        ::close(fd); throw std::runtime_error("API configuration must be owner-only regular file (0600)");
    }
    QFile file;
    if(!file.open(fd,QIODevice::ReadOnly,QFileDevice::AutoCloseHandle)) {
        ::close(fd);throw std::runtime_error("Cannot read API configuration");
    }
    QJsonParseError error;
    auto doc=QJsonDocument::fromJson(file.read(16385),&error);
    auto config=doc.object();
    const auto key=config.value("api_key").toString();
    if(error.error!=QJsonParseError::NoError || !doc.isObject() ||
       !QRegularExpression("^[A-Za-z0-9_-]{1,512}$").match(key).hasMatch() ||
       config.value("model").toString()!=model || config.value("demo_mode").toBool(false))
        throw std::runtime_error("Live test requires API key, gpt-4.1-mini, and no demo mode");
    return key;
}
inline QJsonObject payload(const QImage &image) {
    if(image.size()!=QSize(834,1584)) throw std::runtime_error("Unexpected live question dimensions");
    QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);
    if(!image.save(&buffer,"PNG") || png.size()>8*1024*1024) throw std::runtime_error("Question encoding failed");
    QJsonObject fields{{"kind",QJsonObject{{"type","string"},{"enum",QJsonArray{"text","unsupported"}}}},
        {"text",QJsonObject{{"type","string"}}}};
    QJsonObject format{{"type","json_schema"},{"name","move_text_answer"},{"strict",true},
        {"schema",QJsonObject{{"type","object"},{"properties",fields},
            {"required",QJsonArray{"kind","text"}},{"additionalProperties",false}}}};
    return {{"model",model},{"store",false},{"max_output_tokens",outputTokens},
        {"instructions","Read the latest unanswered handwritten question. Earlier writing may contain prior answers. "
            "Give a concise ordinary text answer in the question's language, usually under 100 words. "
            "If handwriting is unclear, ask briefly for clarification; do not invent it. "
            "Use kind=text and simple Markdown, no HTML or external image URLs. "
            "This test has no tools. Requests for current facts, online verification, sources, search, generated "
            "images, plots, diagrams or templates MUST return kind=unsupported with a short explanation, "
            "not fabricated results. Never claim to have searched or generated an image."},
        {"input",QJsonArray{QJsonObject{{"role","user"},{"content",QJsonArray{
            QJsonObject{{"type","input_text"},{"text","Answer the latest unanswered question on this page."}},
            QJsonObject{{"type","input_image"},{"detail","high"},
                {"image_url","data:image/png;base64,"+QString::fromLatin1(png.toBase64())}}
        }}}}}, {"text",QJsonObject{{"format",format}}}};
}
inline QString parse(const QJsonObject &root) {
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
       !answer.value("kind").isString() || !answer.value("text").isString() ||
       answer.value("kind")!="text") throw std::runtime_error("Unsupported request or malformed text answer");
    auto text=answer.value("text").toString().trimmed();
    if(text.isEmpty() || text.size()>4000) throw std::runtime_error("Empty or oversized text answer");
    return text;
}
// GUI-thread asynchronous transport: no nested event loop, detached worker,
// retries, redirect following, insecure TLS fallback, or notebook persistence.
class Client final : public QObject {
public:
    using Done=std::function<void(QString,QString)>;
    explicit Client(QObject *parent):QObject(parent) {
        timeout_.setSingleShot(true);
        connect(&timeout_,&QTimer::timeout,this,[this]{finish({},"OpenAI request timed out; no retry");});
    }
    ~Client() override {cancel();}
    void cancel() {
        timeout_.stop();done_={};valid_={};
        if(reply_) {auto *r=reply_.data();reply_.clear();disconnect(r,nullptr,this,nullptr);r->abort();r->deleteLater();}
        bytes_.clear();
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
        reply_=manager->post(request,body);
        reply_->setReadBufferSize(65536);
        connect(reply_,&QNetworkReply::readyRead,this,[this]{drain();});
        connect(reply_,&QNetworkReply::finished,this,[this]{complete();});
        timeout_.start(timeoutMs);
        // Also handles transports completing synchronously inside post().
        if(reply_->isFinished()) QTimer::singleShot(0,this,[this]{if(reply_)complete();});
    }
private:
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
        if(!drain())return;
        if(!valid_ || !valid_()) {finish({},"Cancelled or page changed during API request");return;}
        if(reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()!=200 || reply_->error()!=QNetworkReply::NoError) {
            finish({},"OpenAI HTTP/network failure; no retry");return;
        }
        try {
            QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes_,&error);
            if(error.error!=QJsonParseError::NoError || !doc.isObject())throw std::runtime_error("Invalid API JSON");
            finish(parse(doc.object()),{});
        } catch(const std::exception &e) {finish({},QString::fromUtf8(e.what()));}
    }
    void finish(QString text,QString error) {
        auto done=std::move(done_);cancel();if(done)done(std::move(text),std::move(error));
    }
    QPointer<QNetworkReply> reply_;QTimer timeout_;QByteArray bytes_;
    bool used_=false;Done done_;std::function<bool()> valid_;
};
}
