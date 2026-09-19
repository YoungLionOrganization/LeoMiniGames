// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "XoxPlugin.h"
#include "XoxGame.h"

GamePluginInfo XoxPlugin::info() const
{
    return {
        QStringLiteral("xox"),
        QStringLiteral("XOX"),
        QStringLiteral("Classic 3×3 local two-player game."),
        QStringLiteral("Board"),
        QStringLiteral("1.0.0"),
        QStringLiteral("YoungLion"),
        QStringLiteral("XO"),
        QUrl(QStringLiteral("qrc:/qt/qml/LeoMiniGames/XoxPage.qml")),
        1
    };
}

QObject *XoxPlugin::createGame(QObject *parent)
{
    return new XoxGame(parent);
}
