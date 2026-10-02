#include "FakeNetwork.h"
#include "core/CredentialStore.h"
#include "core/DeveloperManager.h"
#include <QCoreApplication>
#include <QDebug>

class FakeStore final : public CredentialStore {
public:
    QString stored; QString pendingValue; QString pendingKind; QStringList started;
    Callback pending;
    void read(Callback callback) override { begin("read",QString{},std::move(callback)); }
    void write(const QString &value,Callback callback) override { begin("write",value,std::move(callback)); }
    void erase(Callback callback) override { begin("delete",QString{},std::move(callback)); }
    void begin(const QString &kind,const QString &value,Callback callback) {
        if (pending) qFatal("overlapping credential operations");
        started.append(kind);pendingKind=kind;pendingValue=value;pending=std::move(callback);
    }
    void complete() {
        if (!pending) qFatal("no pending credential operation");
        const QString value=stored;
        if (pendingKind=="write") stored=pendingValue;
        if (pendingKind=="delete") stored.clear();
        auto callback=std::move(pending); pending=nullptr;callback({true,false,value});
    }
};
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);int count=0;auto check=[&](bool ok,const char *name){++count;if(!ok)qFatal("FAIL: %s",name);};
    const QString a="lmg_"+QString(12,'a')+"_"+QString(43,'b');
    const QString b="lmg_"+QString(12,'c')+"_"+QString(43,'d');
    const QByteArray good=R"({"data":{"authenticated":true,"key":{},"developer":{}}})";
    FakeNetwork network;FakeStore store;DeveloperManager manager(nullptr,nullptr,nullptr,nullptr,&network,&store);
    manager.verifyApiKey(a,true);network.last->complete(good);check(store.pendingKind=="write","first write pending");
    manager.logout();manager.verifyApiKey(b,true);network.last->complete(good);
    check(store.started==QStringList{"write"},"logout delete and new write wait for old write");
    store.complete();check(store.pendingKind=="delete","delete executes after old write");
    store.complete();check(store.pendingKind=="write","new write executes after delete");
    store.complete();check(store.stored==b&&manager.hasSavedKey()&&manager.authenticated(),"new key survives delayed old jobs");
    manager.logout();store.complete();check(store.stored.isEmpty()&&!manager.hasSavedKey(),"logout clears final stored key");
    store.stored=a;manager.restoreSavedKey();const int before=network.requests;manager.logout();
    store.complete();check(network.requests==before&&!manager.authenticated(),"late read cannot reauthenticate after logout");store.complete();
    check(store.stored.isEmpty(),"queued logout deletion still runs after stale read");
    qInfo()<<"PASS:"<<count<<"credential ordering assertions";
}
