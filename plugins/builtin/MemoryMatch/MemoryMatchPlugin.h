// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QObject>
#include "sdk/IGamePlugin.h"

class MemoryMatchPlugin final : public QObject, public IGamePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LeoMiniGamesPlugin_iid FILE "metadata.json")
    Q_INTERFACES(IGamePlugin)

public:
    GamePluginInfo info() const override;
    QObject *createGame(QObject *parent) override;
};
