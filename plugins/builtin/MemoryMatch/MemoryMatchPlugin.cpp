// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "MemoryMatchPlugin.h"
#include "MemoryMatchGame.h"

GamePluginInfo MemoryMatchPlugin::info() const
{
    return {
        QStringLiteral("memory_match"),
        QStringLiteral("Memory Match"),
        QStringLiteral("Flip cards, remember their positions, and find every pair."),
        QStringLiteral("Memory"),
        QStringLiteral("1.0.0"),
        QStringLiteral("YoungLion"),
        QStringLiteral("MM"),
        QUrl(QStringLiteral("qrc:/qt/qml/LeoMiniGames/MemoryMatchPage.qml")),
        1
    };
}

QObject *MemoryMatchPlugin::createGame(QObject *parent)
{
    return new MemoryMatchGame(parent);
}
