#include "readonly-template.h"
#include <QCoreApplication>
#include <cassert>
#include <iostream>
class TestDocument : public QObject {
    Q_OBJECT
public:
    QString name="Blank"; int calls=0;
    Q_INVOKABLE QString templateForPage(int page) {assert(page==0);++calls;return name;}
};
class TestRoot : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool documentLoaded MEMBER loaded)
    Q_PROPERTY(QObject* document MEMBER document)
    Q_PROPERTY(int currentPage MEMBER page)
public:
    bool loaded=true; QObject *document=nullptr; int page=0;
};
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv); QQmlEngine engine;
    TestDocument document;TestRoot root;root.document=&document;
    const auto ownership=QQmlEngine::objectOwnership(&root);
    assert(diagnostic::templateStatus(&engine,&root)=="blank");
    for(const auto &name:{"P Dots S","P Grid small"}) {
        document.name=name;
        assert(diaryMetadata::templateIdentity(&engine,&root)==name);
        assert(diagnostic::templateStatus(&engine,&root)=="nonblank");
    }
    document.name="Private custom template";
    assert(diagnostic::templateStatus(&engine,&root)=="nonblank");
    document.name="";assert(diagnostic::templateStatus(&engine,&root)=="unavailable");
    const int calls=document.calls;
    root.loaded=false;assert(diagnostic::templateStatus(&engine,&root)=="unavailable");
    root.loaded=true;root.page=-1;assert(diagnostic::templateStatus(&engine,&root)=="unavailable");
    root.page=0;root.document=nullptr;assert(diagnostic::templateStatus(&engine,&root)=="unavailable");
    assert(document.calls==calls);
    assert(diagnostic::templateStatus(nullptr,&root)=="unavailable");
    assert(QQmlEngine::objectOwnership(&root)==ownership);
    std::cout<<"Read-only template status checks passed\n";
}
#include "test-template.moc"
