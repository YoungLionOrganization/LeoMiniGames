#include "core/NetworkSafety.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <cstring>
class Reply : public QNetworkReply {
public:
    QByteArray body;
    bool aborted=false;
    Reply(){open(QIODevice::ReadOnly);}
    void abort() override {if(aborted)return;aborted=true;setError(OperationCanceledError,"aborted");setFinished(true);emit finished();}
    qint64 bytesAvailable() const override{return body.size()+QNetworkReply::bytesAvailable();}
    void feed(QByteArray bytes){body+=bytes;emit readyRead();}
    void length(qint64 n){setHeader(QNetworkRequest::ContentLengthHeader,n);emit metaDataChanged();}
protected:
    qint64 readData(char *data,qint64 max) override{const auto n=qMin(max,qint64(body.size()));memcpy(data,body.constData(),n);body.remove(0,n);return n;}
};
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);int count=0;
 auto check=[&](bool ok,const char*n){++count;if(!ok)qFatal("FAIL: %s",n);};
 Reply exact;NetworkSafety::boundJsonReply(&exact,16);exact.feed(QByteArray(16,'x'));check(!exact.aborted,"exact limit allowed");
 exact.feed("x");check(exact.aborted,"chunked response bounded");check(exact.property("lmgResponseTooLarge").toBool(),"size error marked");
 Reply header;NetworkSafety::boundJsonReply(&header,16);header.length(100000);check(header.aborted,"large Content-Length rejected");
 Reply deadline;NetworkSafety::boundJsonReply(&deadline,16,10);QEventLoop loop;QTimer::singleShot(30,&loop,&QEventLoop::quit);loop.exec();check(deadline.aborted,"absolute deadline");
 check(deadline.property("lmgDeadlineExceeded").toBool(),"deadline marked");qInfo()<<"PASS:"<<count<<"network assertions";
}
