// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QVariantList>
#include <QTimer>
#include <vector>

class MemoryMatchGame final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList cards READ cards NOTIFY cardsChanged)
    Q_PROPERTY(int moves READ moves NOTIFY statsChanged)
    Q_PROPERTY(int matches READ matches NOTIFY statsChanged)
    Q_PROPERTY(int bestMoves READ bestMoves NOTIFY statsChanged)
    Q_PROPERTY(int elapsedSeconds READ elapsedSeconds NOTIFY timeChanged)
    Q_PROPERTY(int bestSeconds READ bestSeconds NOTIFY statsChanged)
    Q_PROPERTY(int wins READ wins NOTIFY statsChanged)
    Q_PROPERTY(bool locked READ locked NOTIFY lockedChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)

public:
    explicit MemoryMatchGame(QObject *parent = nullptr);

    QVariantList cards() const;
    int moves() const;
    int matches() const;
    int bestMoves() const;
    int elapsedSeconds() const;
    int bestSeconds() const;
    int wins() const;
    bool locked() const;
    bool gameOver() const;
    QString status() const;

    Q_INVOKABLE void reset();
    Q_INVOKABLE bool flip(int index);

signals:
    void cardsChanged();
    void statsChanged();
    void lockedChanged();
    void stateChanged();
    void pairResolved(bool matched);
    void timeChanged();

private:
    struct Card {
        QString symbol;
        bool faceUp = false;
        bool matched = false;
    };

    void resolvePair(int secondIndex);

    std::vector<Card> m_cards;
    int m_firstIndex = -1;
    int m_moves = 0;
    int m_matches = 0;
    int m_bestMoves = 0;
    QTimer m_timer;
    int m_elapsedSeconds = 0;
    int m_bestSeconds = 0;
    int m_wins = 0;
    bool m_locked = false;
    bool m_gameOver = false;
    QString m_status = QStringLiteral("playing");
};
