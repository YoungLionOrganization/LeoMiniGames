// SPDX-License-Identifier: MIT OR GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

struct GamePluginInfo
{
    QString id;
    QString name;
    QString description;
    QString category;
    QString version;
    QString author;
    QString icon;
    QUrl entryUrl;
    int apiVersion = 1;
};

class IGamePlugin
{
public:
    virtual ~IGamePlugin() = default;
    virtual GamePluginInfo info() const = 0;
    virtual QObject *createGame(QObject *parent) = 0;
};

#define LeoMiniGamesPlugin_iid "xyz.younglion.leominigames.IGamePlugin/1.0"
Q_DECLARE_INTERFACE(IGamePlugin, LeoMiniGamesPlugin_iid)
