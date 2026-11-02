// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/RuntimeChecks.h"
#include <QGuiApplication>
#include <QQmlEngine>
#include <QDebug>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QString error;
    if (!RuntimeChecks::graphics(&engine, &error)) {
        qCritical().noquote() << error;
        return 1;
    }
    return 0;
}
