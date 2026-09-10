// SPDX-License-Identifier: GPL-3.0-or-later
#include "TwentyFortyEightPlugin.h"
#include "TwentyFortyEightGame.h"

GamePluginInfo TwentyFortyEightPlugin::info() const
{
    return {
        QStringLiteral("2048"),
        QStringLiteral("2048"),
        QStringLiteral("Slide and merge equal tiles to build the 2048 tile."),
        QStringLiteral("Puzzle"),
        QStringLiteral("1.0.0"),
        QStringLiteral("YoungLion"),
        QStringLiteral("2048"),
        QUrl(QStringLiteral("qrc:/qt/qml/LeoMiniGames/TwentyFortyEightPage.qml")),
        1
    };
}

QObject *TwentyFortyEightPlugin::createGame(QObject *parent)
{
    return new TwentyFortyEightGame(parent);
}
