// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/GameRuntime.h"
#include <QCoreApplication>
#include <QVariantMap>
#include <iostream>
int main(int argc,char **argv){
    QCoreApplication app(argc,argv); GameRuntime runtime;
    const QVariantMap legacy=runtime.checkCompatibility(QString(),QString(),QStringList());
    if(!legacy.value("ok").toBool()||!legacy.value("legacy").toBool()) return 1;
    const QVariantMap v07=runtime.checkCompatibility(QStringLiteral("0.7"),QStringLiteral("0.7"),{QStringLiteral("resources"),QStringLiteral("save.atomic")});
    if(!v07.value("ok").toBool()||v07.value("legacy").toBool()) return 2;
    if(runtime.checkCompatibility(QStringLiteral("0.8"),QStringLiteral("0.7"),QStringList()).value("ok").toBool()) return 3;
    if(runtime.checkCompatibility(QStringLiteral("0.7"),QStringLiteral("0.7"),{QStringLiteral("does.not.exist")}).value("ok").toBool()) return 4;
    std::cout << "GameRuntime compatibility PASS\n"; return 0;
}
