// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QElapsedTimer>
#include <QObject>

class GameClock final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(double timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged)
public:
    explicit GameClock(QObject *parent = nullptr);
    bool paused() const;
    double timeScale() const;
    void setTimeScale(double scale);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE qint64 elapsedMs() const;
    Q_INVOKABLE double elapsedSeconds() const;
signals:
    void pausedChanged();
    void timeScaleChanged();
private:
    QElapsedTimer m_timer;
    qint64 m_accumulatedMs = 0;
    bool m_paused = false;
    double m_timeScale = 1.0;
};
