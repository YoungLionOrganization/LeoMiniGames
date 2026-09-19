// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
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
