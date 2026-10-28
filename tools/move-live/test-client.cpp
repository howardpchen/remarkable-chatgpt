#define INK_MOVE_LIVE_TEST
#include "client.h"
#include "mock-transport.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTest>
#include <cassert>
#include <iostream>
using namespace moveLive;
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    QImage image(954,1696,QImage::Format_RGB32);image.fill(Qt::white);
    assert(diaryMoveBoundedCaptureArea()==QRect(0,0,954,1696));
    auto pose=QJsonObject{{"ok",true},{"items",QJsonArray{QJsonObject{{"rect",QJsonArray{0,0,954,1696}}}}}};
    assert(diaryMoveCaptureUiClear(pose));
    auto items=pose["items"].toArray();
    items.append(QJsonObject{{"rect",QJsonArray{842,0,112,112}}});
    pose["items"]=items; assert(!diaryMoveCaptureUiClear(pose));
    assert(!diaryMoveCaptureUiClear({}));
    items=QJsonArray{QJsonObject{{"rect",QJsonArray{0,0,112,1696}}}};
    pose["items"]=items; assert(!diaryMoveCaptureUiClear(pose));
    bool oldCropRejected=false;
    try {moveLive::payload(QImage(834,1584,QImage::Format_RGB32));}
    catch(const std::runtime_error &) {oldCropRejected=true;}
    assert(oldCropRejected);
    image.setPixelColor(0,0,Qt::black);
    image.setPixelColor(200,1695,Qt::black);
    // Exact multimodal contract, no server tools or state, bounded model/output.
    const auto body=moveLive::payload(image);
    assert(body["model"]==model && body["store"]==false && body["max_output_tokens"]==800);
    assert(body["service_tier"]=="default");
    assert(body["reasoning"].toObject()["effort"]=="none");
    assert(!body.contains("tools") && !body.contains("previous_response_id"));
    const auto format=body["text"].toObject()["format"].toObject();
    assert(format["strict"]==true && format["schema"].toObject()["additionalProperties"]==false);
    auto input=body["input"].toArray().first().toObject()["content"].toArray();
    assert(input.size()==2 && input[1].toObject()["type"]=="input_image");
    const auto uri=input[1].toObject()["image_url"].toString();
    assert(QImage::fromData(QByteArray::fromBase64(uri.section(',',1).toLatin1())).size()==image.size());
    const auto decoded=QImage::fromData(QByteArray::fromBase64(uri.section(',',1).toLatin1()));
    assert(decoded.pixelColor(0,0)==Qt::black && decoded.pixelColor(200,1695)==Qt::black);
    for(const auto scenario:{"success","immediate","dispatch-blocked","invalid-before","invalid-after","timeout","http","unauthorized","rate-limit","server-error","dns","tls","closed","oversize","refusal","incomplete","schema","needs-web","clarification","unsupported","cancel","destroy"}) {
        LiveMockManager manager;bool valid=true,called=false;QString text,error;
        auto *client=new Client(&app);
        int usageCalls=0,providerCalls=0,observedHttp=-1;QString observedCode;
        client->providerObserver=[&](int http,const QString &code){++providerCalls;observedHttp=http;observedCode=code;};
        if(QString(scenario)=="dispatch-blocked")client->dispatchGuard=[]{return false;};
        client->usageObserver=[&](const QJsonObject &){++usageCalls;};
        if(QString(scenario)=="immediate")manager.delay=-2;
        if(QString(scenario)=="timeout" || QString(scenario)=="cancel" || QString(scenario)=="destroy")manager.delay=-1;
        if(QString(scenario)=="invalid-before")valid=false;
        if(QString(scenario)=="invalid-after")manager.onDispatch=[&]{valid=false;};
        if(QString(scenario)=="http")manager.code=302;
        if(QString(scenario)=="unauthorized") {manager.code=401;manager.failure=QNetworkReply::AuthenticationRequiredError;}
        if(QString(scenario)=="rate-limit") {manager.code=429;manager.response=R"({"error":{"code":"insufficient_quota","message":"synthetic-secret"}})";}
        if(QString(scenario)=="server-error") {manager.code=503;manager.response=R"({"error":{"code":"synthetic-secret","message":"synthetic-secret"}})";}
        if(QString(scenario)=="dns") {manager.code=0;manager.failure=QNetworkReply::HostNotFoundError;}
        if(QString(scenario)=="tls") {manager.code=0;manager.failure=QNetworkReply::SslHandshakeFailedError;}
        if(QString(scenario)=="closed") {manager.code=0;manager.failure=QNetworkReply::RemoteHostClosedError;}
        if(QString(scenario)=="oversize")manager.response=QByteArray(1024*1024+1,'x');
        if(QString(scenario)=="needs-web")manager.response=mockAnswer("synthetic-secret","needs_web");
        if(QString(scenario)=="clarification")manager.response=mockAnswer("synthetic-secret","clarification");
        if(QString(scenario)=="unsupported")manager.response=mockAnswer("synthetic-secret","unsupported");
        if(QString(scenario)=="schema")manager.response=mockAnswer("unsupported","image");
        if(QString(scenario)=="incomplete") {auto o=QJsonDocument::fromJson(manager.response).object();o["status"]="incomplete";manager.response=QJsonDocument(o).toJson();}
        if(QString(scenario)=="refusal")manager.response=R"({"status":"completed","output":[{"type":"message","role":"assistant","status":"completed","content":[{"type":"refusal","refusal":"No"}]}]})";
        client->start(image,"synthetic-not-a-key",[&]{return valid;},[&](QString t,QString e){assert(!called);called=true;text=t;error=e;},&manager,50);
        if(QString(scenario)=="cancel" || QString(scenario)=="destroy") {
            if(QString(scenario)=="cancel")client->cancel();else {delete client;client=nullptr;}
            QTest::qWait(70);assert(!called && manager.aborts==1);
        } else {
            assert(QTest::qWaitFor([&]{return called;},1000));
            const bool usageExpected=QString(scenario)=="invalid-after" || QString(scenario)=="success" || QString(scenario)=="immediate" || QString(scenario)=="refusal" || QString(scenario)=="incomplete" || QString(scenario)=="schema" || QString(scenario)=="needs-web" || QString(scenario)=="clarification" || QString(scenario)=="unsupported";
            assert(usageCalls==(usageExpected?1:0));
            const bool providerExpected=usageExpected || QString(scenario)=="http" || QString(scenario)=="unauthorized" || QString(scenario)=="rate-limit" || QString(scenario)=="server-error" || QString(scenario)=="dns" || QString(scenario)=="tls" || QString(scenario)=="closed";
            assert(providerCalls==(providerExpected?1:0));
            if(providerExpected)assert(observedHttp==manager.code);
            if(QString(scenario)=="rate-limit")assert(observedCode=="insufficient_quota");
            const bool success=QString(scenario)=="success" || QString(scenario)=="immediate";
            assert(success ? (!text.isEmpty() && error.isEmpty()) : (text.isEmpty() && !error.isEmpty()));
            if(QString(scenario)=="needs-web")assert(error=="Needs web access; search is off.");
            if(QString(scenario)=="clarification")assert(error=="Cannot read question. Please rewrite.");
            if(QString(scenario)=="unsupported")assert(error=="Text answers only; try another question.");
            assert(!error.contains("synthetic-secret") && !error.contains("synthetic-not-a-key"));
            if(manager.code!=200 || manager.failure!=QNetworkReply::NoError) {
                assert(error==transportFailure(manager.code,manager.failure,manager.response));
                assert(error.contains(QString("Qt network error %1").arg(int(manager.failure))));
                assert(error.contains(manager.code ? QString("HTTP %1").arg(manager.code) : "no HTTP response"));
                if(QString(scenario)=="rate-limit")assert(error.contains("insufficient_quota"));
            }
            assert(manager.requests==((QString(scenario)=="invalid-before" || QString(scenario)=="dispatch-blocked")?0:1));
            bool rejected=false;
            client->start(image,"synthetic-not-a-key",[]{return true;},[&](QString,QString e){rejected=!e.isEmpty();},&manager);
            assert(rejected && manager.requests==((QString(scenario)=="invalid-before" || QString(scenario)=="dispatch-blocked")?0:1));
        }
        if(manager.requests) {
            assert(manager.request.url()==QUrl("https://api.openai.com/v1/responses"));
            assert(manager.request.attribute(QNetworkRequest::RedirectPolicyAttribute)==QNetworkRequest::ManualRedirectPolicy);
            assert(manager.request.rawHeader("Authorization")=="Bearer synthetic-not-a-key");
        }
        delete client;QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        assert(manager.findChildren<QNetworkReply *>().isEmpty());
        std::cout<<"PASS synthetic transport: "<<scenario<<'\n';
    }
    QTemporaryDir temp;assert(temp.isValid());const auto path=temp.path()+"/config.json";
    auto rejects=[&](const QString &p){try{loadKey(p);return false;}catch(const std::exception &){return true;}};
    assert(rejects(path));
    QFile file(path);assert(file.open(QIODevice::WriteOnly));
    file.write("{\"api_key\":\"synthetic-not-a-key\",\"model\":\"gpt-6-luna\"}");file.close();
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);assert(loadKey(path)=="synthetic-not-a-key");
    assert(QFile::link(path,temp.path()+"/link"));assert(rejects(temp.path()+"/link"));
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadOther);assert(rejects(path));
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    assert(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write("{\"api_key\":\"synthetic\",\"model\":\"another-model\"}");file.close();assert(rejects(path));
    assert(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write("{\"api_key\":\"synthetic\",\"model\":\"gpt-4.1-mini\"}");file.close();assert(rejects(path));
    std::cout<<"PASS private config: missing, valid 0600, symlink, public permissions, incompatible model\n";
}
