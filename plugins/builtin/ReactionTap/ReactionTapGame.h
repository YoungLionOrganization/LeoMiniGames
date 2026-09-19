// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

class ReactionTapGame final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int lastMs READ lastMs NOTIFY statsChanged)
    Q_PROPERTY(int bestMs READ bestMs NOTIFY statsChanged)
    Q_PROPERTY(int rounds READ rounds NOTIFY statsChanged)
    Q_PROPERTY(int averageMs READ averageMs NOTIFY statsChanged)

public:
    explicit ReactionTapGame(QObject *parent = nullptr);

    QString state() const;
    int lastMs() const;
    int bestMs() const;
    int rounds() const;
    int averageMs() const;

    Q_INVOKABLE void startRound();
    Q_INVOKABLE void tap();

signals:
    void stateChanged();
    void statsChanged();

private:
    QTimer m_waitTimer;
    QElapsedTimer m_elapsed;
    QString m_state = QStringLiteral("idle");
    int m_lastMs = 0;
    int m_bestMs = 0;
    int m_rounds = 0;
    qint64 m_totalMs = 0;
};
