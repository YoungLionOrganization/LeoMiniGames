// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QObject>
#include <QStringList>
#include <vector>

class BlackjackGame final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList playerCards READ playerCards NOTIFY cardsChanged)
    Q_PROPERTY(QStringList dealerCards READ dealerCards NOTIFY cardsChanged)
    Q_PROPERTY(int playerValue READ playerValue NOTIFY cardsChanged)
    Q_PROPERTY(int dealerValue READ dealerValue NOTIFY cardsChanged)
    Q_PROPERTY(QString message READ message NOTIFY stateChanged)
    Q_PROPERTY(bool roundOver READ roundOver NOTIFY stateChanged)
    Q_PROPERTY(bool canHit READ canHit NOTIFY stateChanged)
    Q_PROPERTY(bool canStand READ canStand NOTIFY stateChanged)
    Q_PROPERTY(int wins READ wins NOTIFY scoreChanged)
    Q_PROPERTY(int losses READ losses NOTIFY scoreChanged)
    Q_PROPERTY(int pushes READ pushes NOTIFY scoreChanged)

public:
    explicit BlackjackGame(QObject *parent = nullptr);

    QStringList playerCards() const;
    QStringList dealerCards() const;
    int playerValue() const;
    int dealerValue() const;
    QString message() const;
    bool roundOver() const;
    bool canHit() const;
    bool canStand() const;
    int wins() const;
    int losses() const;
    int pushes() const;

    Q_INVOKABLE void newRound();
    Q_INVOKABLE void hit();
    Q_INVOKABLE void stand();
    Q_INVOKABLE void resetScore();

signals:
    void cardsChanged();
    void stateChanged();
    void scoreChanged();
    void roundStarted();
    void cardDealt(const QString &hand);

private:
    struct Card { int rank = 1; int suit = 0; };
    void rebuildDeck();
    Card draw();
    int handValue(const std::vector<Card> &hand) const;
    QString label(const Card &card) const;
    QStringList labels(const std::vector<Card> &hand, bool hideHole) const;
    void finishRound();
    void resolveNaturals();
    void saveScore() const;

    std::vector<Card> m_deck;
    std::vector<Card> m_player;
    std::vector<Card> m_dealer;
    QString m_message;
    bool m_roundOver = false;
    int m_wins = 0;
    int m_losses = 0;
    int m_pushes = 0;
};
