// SPDX-License-Identifier: GPL-3.0-or-later
#include "MinesweeperPlugin.h"
#include "MinesweeperGame.h"

GamePluginInfo MinesweeperPlugin::info() const
{
    return {
        QStringLiteral("minesweeper"),
        QStringLiteral("Minesweeper"),
        QStringLiteral("Open safe cells, flag mines, and clear the board."),
        QStringLiteral("Puzzle"),
        QStringLiteral("1.0.0"),
        QStringLiteral("YoungLion"),
        QStringLiteral("*"),
        QUrl(QStringLiteral("qrc:/qt/qml/LeoMiniGames/MinesweeperPage.qml")),
        1
    };
}

QObject *MinesweeperPlugin::createGame(QObject *parent)
{
    return new MinesweeperGame(parent);
}
