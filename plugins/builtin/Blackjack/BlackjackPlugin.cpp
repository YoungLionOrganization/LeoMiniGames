// SPDX-License-Identifier: GPL-3.0-or-later
#include "BlackjackPlugin.h"
#include "BlackjackGame.h"

GamePluginInfo BlackjackPlugin::info() const
{
    return {
        QStringLiteral("blackjack"),
        QStringLiteral("Blackjack"),
        QStringLiteral("Reach 21 without going over. Dealer stands on 17."),
        QStringLiteral("Cards"),
        QStringLiteral("1.0.0"),
        QStringLiteral("YoungLion"),
        QStringLiteral("21"),
        QUrl(QStringLiteral("qrc:/qt/qml/LeoMiniGames/BlackjackPage.qml")),
        1
    };
}

QObject *BlackjackPlugin::createGame(QObject *parent)
{
    return new BlackjackGame(parent);
}
