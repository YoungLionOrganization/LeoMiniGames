// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QVariantMap>
#include <QElapsedTimer>

class AppPaths;

class GameStats final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY changed)
public:
    explicit GameStats(AppPaths *paths, QObject *parent = nullptr);
    QString gameId() const;
    Q_INVOKABLE qint64 highScore() const;
    Q_INVOKABLE qint64 highScore(const QString &name) const;
    Q_INVOKABLE bool submitHighScore(qint64 score);
    Q_INVOKABLE bool submitHighScore(qint64 score, const QString &name);
    Q_INVOKABLE qint64 counter(const QString &name) const;
    Q_INVOKABLE void setCounter(const QString &name, qint64 value);
    Q_INVOKABLE qint64 increment(const QString &name);
    Q_INVOKABLE qint64 increment(const QString &name, qint64 amount);
    Q_INVOKABLE qint64 gamesPlayed() const;
    Q_INVOKABLE qint64 totalTimeMs() const;
    void activate(const QString &gameId);
    void startSession();
    void endSession();
signals: void changed();
private:
    QString path() const; void load(); void persist() const;
    AppPaths *m_paths = nullptr; QString m_gameId; QVariantMap m_data; QElapsedTimer m_session;
};
