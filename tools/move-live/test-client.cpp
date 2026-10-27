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
    QImage image(834,1584,QImage::Format_RGB32);image.fill(Qt::white);
    // Exact multimodal contract, no server tools or state, bounded model/output.
    const auto body=moveLive::payload(image);
    assert(body["model"]==model && body["store"]==false && body["max_output_tokens"]==800);
    assert(!body.contains("tools") && !body.contains("previous_response_id"));
    const auto format=body["text"].toObject()["format"].toObject();
    assert(format["strict"]==true && format["schema"].toObject()["additionalProperties"]==false);
    auto input=body["input"].toArray().first().toObject()["content"].toArray();
    assert(input.size()==2 && input[1].toObject()["type"]=="input_image");
    const auto uri=input[1].toObject()["image_url"].toString();
    assert(QImage::fromData(QByteArray::fromBase64(uri.section(',',1).toLatin1())).size()==image.size());
    for(const auto scenario:{"success","immediate","invalid-before","invalid-after","timeout","http","oversize","refusal","incomplete","schema","cancel","destroy"}) {
        LiveMockManager manager;bool valid=true,called=false;QString text,error;
        auto *client=new Client(&app);
        if(QString(scenario)=="immediate")manager.delay=-2;
        if(QString(scenario)=="timeout" || QString(scenario)=="cancel" || QString(scenario)=="destroy")manager.delay=-1;
        if(QString(scenario)=="invalid-before")valid=false;
        if(QString(scenario)=="invalid-after")manager.onDispatch=[&]{valid=false;};
        if(QString(scenario)=="http")manager.code=302;
        if(QString(scenario)=="oversize")manager.response=QByteArray(1024*1024+1,'x');
        if(QString(scenario)=="schema")manager.response=mockAnswer("unsupported","image");
        if(QString(scenario)=="incomplete") {auto o=QJsonDocument::fromJson(manager.response).object();o["status"]="incomplete";manager.response=QJsonDocument(o).toJson();}
        if(QString(scenario)=="refusal")manager.response=R"({"status":"completed","output":[{"type":"message","role":"assistant","status":"completed","content":[{"type":"refusal","refusal":"No"}]}]})";
        client->start(image,"synthetic-not-a-key",[&]{return valid;},[&](QString t,QString e){assert(!called);called=true;text=t;error=e;},&manager,50);
        if(QString(scenario)=="cancel" || QString(scenario)=="destroy") {
            if(QString(scenario)=="cancel")client->cancel();else {delete client;client=nullptr;}
            QTest::qWait(70);assert(!called && manager.aborts==1);
        } else {
            assert(QTest::qWaitFor([&]{return called;},1000));
            const bool success=QString(scenario)=="success" || QString(scenario)=="immediate";
            assert(success ? (!text.isEmpty() && error.isEmpty()) : (text.isEmpty() && !error.isEmpty()));
            assert(manager.requests==(QString(scenario)=="invalid-before"?0:1));
            bool rejected=false;
            client->start(image,"synthetic-not-a-key",[]{return true;},[&](QString,QString e){rejected=!e.isEmpty();},&manager);
            assert(rejected && manager.requests==(QString(scenario)=="invalid-before"?0:1));
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
    file.write("{\"api_key\":\"synthetic-not-a-key\",\"model\":\"gpt-4.1-mini\"}");file.close();
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);assert(loadKey(path)=="synthetic-not-a-key");
    assert(QFile::link(path,temp.path()+"/link"));assert(rejects(temp.path()+"/link"));
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadOther);assert(rejects(path));
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    assert(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write("{\"api_key\":\"synthetic\",\"model\":\"another-model\"}");file.close();assert(rejects(path));
    std::cout<<"PASS private config: missing, valid 0600, symlink, public permissions, incompatible model\n";
}
