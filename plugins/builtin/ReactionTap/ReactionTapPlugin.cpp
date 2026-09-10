// SPDX-License-Identifier: GPL-3.0-or-later
#include "ReactionTapPlugin.h"
#include "ReactionTapGame.h"

GamePluginInfo ReactionTapPlugin::info() const
{
    return {
        QStringLiteral("reaction_tap"),
        QStringLiteral("Reaction Tap"),
        QStringLiteral("Wait for the signal, then tap as quickly as you can."),
        QStringLiteral("Arcade"),
        QStringLiteral("1.0.0"),
        QStringLiteral("YoungLion"),
        QStringLiteral("RT"),
        QUrl(QStringLiteral("qrc:/qt/qml/LeoMiniGames/ReactionTapPage.qml")),
        1
    };
}

QObject *ReactionTapPlugin::createGame(QObject *parent)
{
    return new ReactionTapGame(parent);
}
