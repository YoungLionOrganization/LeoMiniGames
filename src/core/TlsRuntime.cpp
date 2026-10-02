// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "TlsRuntime.h"
#include "NetworkSafety.h"
#include <QCoreApplication>
#include <QSslSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

bool TlsRuntime::initialize() {
#ifdef Q_OS_ANDROID
    qputenv("ANDROID_OPENSSL_SUFFIX", "_3");
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    const QJniObject info = context.callObjectMethod("getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    const QString path = info.getObjectField<jstring>("nativeLibraryDir").toString();
    if (!path.isEmpty()) QCoreApplication::addLibraryPath(path);
    if (QSslSocket::availableBackends().contains(QStringLiteral("openssl"))) QSslSocket::setActiveBackend(QStringLiteral("openssl"));
#endif
    const bool ready = QSslSocket::supportsSsl();
    qInfo().noquote() << "LeoMiniGames TLS:" << diagnostic();
    return ready;
}
QString TlsRuntime::diagnostic() {
    return QStringLiteral("backend=%1; available=%2; OpenSSL=%3; pluginPaths=%4")
        .arg(QSslSocket::activeBackend(), QSslSocket::availableBackends().join(','), QSslSocket::sslLibraryVersionString(), QCoreApplication::libraryPaths().join(';'));
}
int TlsRuntime::probe() {
    if (!QSslSocket::supportsSsl()) { qCritical().noquote() << "FAIL: TLS backend unavailable:" << diagnostic(); return 10; }
    QNetworkAccessManager network;
    QEventLoop loop;
    QNetworkRequest request(QUrl(QStringLiteral("https://leominigames.younglion.xyz/api/v1/mods?limit=1")));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
    QNetworkReply *reply = network.get(request);
    NetworkSafety::boundJsonReply(reply, 2*1024*1024, 30000);
    bool encrypted = false;
    QObject::connect(reply, &QNetworkReply::encrypted, &loop, [&] { encrypted = true; });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (!encrypted || reply->error()!=QNetworkReply::NoError || status!=200) {
        qCritical().noquote() << "FAIL: HTTPS catalog probe:" << reply->errorString() << status << diagnostic(); return 11;
    }
    qInfo().noquote() << "PASS: HTTPS catalog probe; certificate validated;" << diagnostic(); return 0;
}
