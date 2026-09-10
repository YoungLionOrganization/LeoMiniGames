// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QUrl>

class GameResources final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY changed)
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
public:
    explicit GameResources(QObject *parent=nullptr);
    QString gameId() const { return m_gameId; }
    bool ready() const { return true; }
    QString version() const { return QStringLiteral("0.7"); }
    void activate(const QString &gameId);
    Q_INVOKABLE QUrl url(const QString &relativePath) const;
    Q_INVOKABLE bool exists(const QString &relativePath) const;
    Q_INVOKABLE QStringList capabilities() const { return {QStringLiteral("qrc_url"), QStringLiteral("exists")}; }
signals: void changed();
private: static bool safePath(const QString &path); QString m_gameId;
};
