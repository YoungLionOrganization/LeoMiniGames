#include "core/DeveloperManager.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <cstring>
class AuthReply : public QNetworkReply {
public:
 QByteArray body; bool aborted=false;
 AuthReply(const QNetworkRequest&r,QObject*p):QNetworkReply(p){setRequest(r);setUrl(r.url());open(QIODevice::ReadOnly);}
 void abort()override{aborted=true;setError(OperationCanceledError,"canceled");setFinished(true);emit finished();}
 void complete(QByteArray data,int status=200){body=data;setAttribute(QNetworkRequest::HttpStatusCodeAttribute,status);setError(NoError,QString{});emit readyRead();if(!aborted){setFinished(true);emit finished();}}
 void lateComplete(QByteArray data){body=data;setAttribute(QNetworkRequest::HttpStatusCodeAttribute,200);setError(NoError,QString{});emit finished();}
 void redirect(){setAttribute(QNetworkRequest::RedirectionTargetAttribute,QUrl("https://example.invalid/"));complete("{}",302);}
 qint64 bytesAvailable()const override{return body.size()+QNetworkReply::bytesAvailable();}
protected:qint64 readData(char*d,qint64 max)override{auto n=qMin(max,qint64(body.size()));memcpy(d,body.constData(),n);body.remove(0,n);return n;}
};
class AuthNetwork:public QNetworkAccessManager{
public:AuthReply*last=nullptr;int requests=0;
protected:QNetworkReply*createRequest(Operation,const QNetworkRequest&r,QIODevice*)override{++requests;last=new AuthReply(r,this);return last;}
};
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);int count=0;
 auto check=[&](bool ok,const char*n){++count;if(!ok)qFatal("FAIL: %s",n);};
 AuthNetwork network;DeveloperManager manager(nullptr,nullptr,nullptr,nullptr,&network);
 const QString key=QStringLiteral("lmg_")+QString(12,'a')+"_"+QString(43,'b');
 const QByteArray good=R"({"data":{"authenticated":true,"key":{},"developer":{}}})";
 manager.verifyApiKey("invalid");check(network.requests==0,"bad key rejected before request");
 manager.verifyApiKey(key);check(manager.verifying(),"pending verification");
 auto*old=network.last;manager.logout();check(old->aborted,"logout cancels request");old->lateComplete(good);check(!manager.authenticated(),"late success cannot log back in");
 manager.verifyApiKey(key);network.last->complete(good);check(manager.authenticated(),"contract accepted");
 manager.logout();check(!manager.authenticated(),"logout locks session");check(!manager.launchImported(),"cannot run local package while locked");
 manager.verifyApiKey(key);network.last->complete("{not json");check(!manager.authenticated()&&!manager.verifying(),"invalid JSON locks session");
 manager.verifyApiKey(key);network.last->complete("{\"data\":{\"authenticated\":true}}");check(!manager.authenticated(),"incomplete auth contract rejected");
 manager.verifyApiKey(key);network.last->redirect();check(!manager.authenticated(),"redirect rejected");
 manager.verifyApiKey(key);network.last->complete(QByteArray(524289,'x'));check(!manager.authenticated()&&!manager.verifying(),"oversized auth body aborts");
 manager.verifyApiKey(key);network.last->complete(good,403);check(!manager.authenticated(),"HTTP failure cannot authenticate");
 manager.verifyApiKey(key);auto before=network.requests;manager.verifyApiKey(key);check(network.requests==before,"duplicate verification suppressed");manager.logout();
 qInfo()<<"PASS:"<<count<<"Developer Lab auth assertions";
}
