// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QVariantMap>
class AppPaths;
class Achievements final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY changed)
public:
    explicit Achievements(AppPaths *paths, QObject *parent=nullptr);
    QString gameId() const;
    Q_INVOKABLE bool unlock(const QString &id);
    Q_INVOKABLE bool isUnlocked(const QString &id) const;
    Q_INVOKABLE qreal progress(const QString &id) const;
    Q_INVOKABLE void setProgress(const QString &id, qreal progress);
    void activate(const QString &gameId);
signals: void changed(); void unlocked(const QString &id);
private:
    QString path() const; void load(); void persist() const;
    AppPaths *m_paths=nullptr; QString m_gameId; QVariantMap m_data;
};
