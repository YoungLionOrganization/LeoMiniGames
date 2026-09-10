// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include "sdk/IGamePlugin.h"

class TwentyFortyEightPlugin final : public QObject, public IGamePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LeoMiniGamesPlugin_iid FILE "metadata.json")
    Q_INTERFACES(IGamePlugin)

public:
    GamePluginInfo info() const override;
    QObject *createGame(QObject *parent) override;
};
