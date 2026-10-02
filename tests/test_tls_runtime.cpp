// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/TlsRuntime.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QSslSocket>
#include <QDebug>

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    const QStringList original=QCoreApplication::libraryPaths(); QTemporaryDir empty;
    QCoreApplication::setLibraryPaths({empty.path()});
    if (TlsRuntime::initialize() || TlsRuntime::probe()!=10) qFatal("FAIL: missing backend must fail before HTTPS request");
    QCoreApplication::setLibraryPaths(original);
    if (!TlsRuntime::initialize() || QSslSocket::activeBackend().isEmpty()) qFatal("FAIL: restored plugin path must provide TLS backend");
    qInfo("PASS: actual Qt TLS plugin discovery rejects a missing backend and recovers with correct paths");
}
