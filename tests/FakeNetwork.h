#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <cstring>
class FakeReply final : public QNetworkReply {
public:
    QByteArray body; bool aborted=false;
    FakeReply(const QNetworkRequest &request, QObject *parent):QNetworkReply(parent) { setRequest(request);setUrl(request.url());open(QIODevice::ReadOnly); }
    void abort() override { if (aborted) return; aborted=true;setError(OperationCanceledError,"cancelled");setFinished(true);emit finished(); }
    void complete(const QByteArray &data, int status=200) {body=data;setAttribute(QNetworkRequest::HttpStatusCodeAttribute,status);emit readyRead();if(!aborted){setFinished(true);emit finished();}}
    void lateComplete(const QByteArray &data) {body=data;setError(NoError,QString{});setAttribute(QNetworkRequest::HttpStatusCodeAttribute,200);emit finished();}
    qint64 bytesAvailable() const override {return body.size()+QNetworkReply::bytesAvailable();}
protected:
    qint64 readData(char *buffer,qint64 max) override {const auto size=qMin(max,qint64(body.size()));memcpy(buffer,body.constData(),size);body.remove(0,size);return size;}
};
class FakeNetwork final : public QNetworkAccessManager {
public: FakeReply *last=nullptr; int requests=0;
protected:
    QNetworkReply *createRequest(Operation,const QNetworkRequest &request,QIODevice *) override {++requests;last=new FakeReply(request,this);return last;}
};
