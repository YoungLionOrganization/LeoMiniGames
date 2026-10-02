// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "BlackjackGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"
#include "sdk/BuiltinState.h"
#include <algorithm>
#include <random>
#include <QSet>

BlackjackGame::BlackjackGame(QObject *parent) : BuiltinGame(parent)
{
    GameLocalStats::migrateLegacy(QStringLiteral("blackjack"), {
        {QStringLiteral("games/blackjack/wins"), QStringLiteral("counter/wins")},
        {QStringLiteral("games/blackjack/losses"), QStringLiteral("counter/losses")},
        {QStringLiteral("games/blackjack/pushes"), QStringLiteral("counter/pushes")}
    });
    m_wins = GameLocalStats::value(QStringLiteral("blackjack"), QStringLiteral("counter/wins"), 0).toInt();
    m_losses = GameLocalStats::value(QStringLiteral("blackjack"), QStringLiteral("counter/losses"), 0).toInt();
    m_pushes = GameLocalStats::value(QStringLiteral("blackjack"), QStringLiteral("counter/pushes"), 0).toInt();
    rebuildDeck();
}

QStringList BlackjackGame::playerCards() const { return labels(m_player, false); }
QStringList BlackjackGame::dealerCards() const { return labels(m_dealer, !m_roundOver); }
int BlackjackGame::playerValue() const { return handValue(m_player); }
int BlackjackGame::dealerValue() const { return m_roundOver ? handValue(m_dealer) : (m_dealer.empty() ? 0 : handValue(std::vector<Card>{m_dealer.front()})); }
QString BlackjackGame::message() const { return m_message; }
bool BlackjackGame::roundOver() const { return m_roundOver; }
bool BlackjackGame::canHit() const { return !suspended() && !m_roundOver; }
bool BlackjackGame::canStand() const { return !suspended() && !m_roundOver; }
int BlackjackGame::wins() const { return m_wins; }
int BlackjackGame::losses() const { return m_losses; }
int BlackjackGame::pushes() const { return m_pushes; }

void BlackjackGame::rebuildDeck()
{
    m_deck.clear();
    m_deck.reserve(52);
    for (int suit = 0; suit < 4; ++suit)
        for (int rank = 1; rank <= 13; ++rank)
            m_deck.push_back(Card{rank, suit});

    std::mt19937 rng(QRandomGenerator::global()->generate());
    std::shuffle(m_deck.begin(), m_deck.end(), rng);
}

BlackjackGame::Card BlackjackGame::draw()
{
    if (m_deck.empty())
        rebuildDeck();
    Card card = m_deck.back();
    m_deck.pop_back();
    return card;
}

int BlackjackGame::handValue(const std::vector<Card> &hand) const
{
    int total = 0;
    int aces = 0;
    for (const Card &card : hand) {
        if (card.rank == 1) { total += 11; ++aces; }
        else if (card.rank >= 10) total += 10;
        else total += card.rank;
    }
    while (total > 21 && aces > 0) {
        total -= 10;
        --aces;
    }
    return total;
}

QString BlackjackGame::label(const Card &card) const
{
    QString rank;
    switch (card.rank) {
    case 1: rank = QStringLiteral("A"); break;
    case 11: rank = QStringLiteral("J"); break;
    case 12: rank = QStringLiteral("Q"); break;
    case 13: rank = QStringLiteral("K"); break;
    default: rank = QString::number(card.rank); break;
    }
    static const QStringList suits{
        QStringLiteral("\u2660"), QStringLiteral("\u2665"),
        QStringLiteral("\u2666"), QStringLiteral("\u2663")
    };
    return rank + suits.at(card.suit);
}

QStringList BlackjackGame::labels(const std::vector<Card> &hand, bool hideHole) const
{
    QStringList result;
    for (std::size_t i = 0; i < hand.size(); ++i) {
        if (hideHole && i == 1) result.append(QStringLiteral("??"));
        else result.append(label(hand[i]));
    }
    return result;
}

void BlackjackGame::newRound()
{
    if (suspended()) return;
    if (m_deck.size() < 15)
        rebuildDeck();
    m_player.clear();
    m_dealer.clear();
    m_roundOver = false;
    m_message = QStringLiteral("your_turn");
    m_player.push_back(draw());
    m_dealer.push_back(draw());
    m_player.push_back(draw());
    m_dealer.push_back(draw());
    emit roundStarted();
    resolveNaturals();
    emit cardsChanged();
    emit stateChanged();
}

void BlackjackGame::resolveNaturals()
{
    const bool playerNatural = handValue(m_player) == 21 && m_player.size() == 2;
    const bool dealerNatural = handValue(m_dealer) == 21 && m_dealer.size() == 2;
    if (!playerNatural && !dealerNatural)
        return;

    m_roundOver = true;
    if (playerNatural && dealerNatural) {
        m_message = QStringLiteral("push_both_blackjack");
        ++m_pushes;
    } else if (playerNatural) {
        m_message = QStringLiteral("blackjack_win");
        ++m_wins;
    } else {
        m_message = QStringLiteral("dealer_blackjack");
        ++m_losses;
    }
    saveScore();
    emit scoreChanged();
}

void BlackjackGame::hit()
{
    if (suspended() || m_roundOver)
        return;
    m_player.push_back(draw());
    emit cardDealt(QStringLiteral("player"));
    const int value = handValue(m_player);
    if (value > 21) {
        m_roundOver = true;
        m_message = QStringLiteral("bust");
        ++m_losses;
        saveScore();
        emit scoreChanged();
    } else if (value == 21) {
        stand();
        return;
    }
    emit cardsChanged();
    emit stateChanged();
}

void BlackjackGame::stand()
{
    if (suspended() || m_roundOver)
        return;
    while (handValue(m_dealer) < 17) {
        m_dealer.push_back(draw());
        emit cardDealt(QStringLiteral("dealer"));
    }
    finishRound();
    emit cardsChanged();
    emit stateChanged();
}

void BlackjackGame::finishRound()
{
    m_roundOver = true;
    const int player = handValue(m_player);
    const int dealer = handValue(m_dealer);

    if (dealer > 21 || player > dealer) {
        m_message = QStringLiteral("you_win");
        ++m_wins;
    } else if (player < dealer) {
        m_message = QStringLiteral("dealer_win");
        ++m_losses;
    } else {
        m_message = QStringLiteral("push");
        ++m_pushes;
    }
    saveScore();
    emit scoreChanged();
}

void BlackjackGame::resetScore()
{
    if (suspended()) return;
    m_wins = m_losses = m_pushes = 0;
    saveScore();
    emit scoreChanged();
    newRound();
}

void BlackjackGame::saveScore() const
{
    GameLocalStats::setValues(QStringLiteral("blackjack"), {
        {QStringLiteral("counter/wins"), m_wins},
        {QStringLiteral("counter/losses"), m_losses},
        {QStringLiteral("counter/pushes"), m_pushes}
    });
}

void BlackjackGame::start() { if (m_player.empty()) newRound(); }
QVariantMap BlackjackGame::snapshot() const {
    const auto encode=[](const std::vector<Card> &cards) { QVariantList out; for (const auto &card:cards) out.append(card.suit*13+card.rank-1); return out; };
    return {{"deck",encode(m_deck)}, {"player",encode(m_player)}, {"dealer",encode(m_dealer)}, {"roundOver",m_roundOver}, {"message",m_message}};
}
bool BlackjackGame::restoreSnapshot(const QVariantMap &state) {
    std::vector<Card> deck,player,dealer; QSet<int> seen;
    const auto decode=[&seen](const QVariant &value,std::vector<Card> &out) {
        if (value.metaType()!=QMetaType::fromType<QVariantList>()) return false;
        const QVariantList cards=value.toList(); if (cards.size()>52) return false;
        for (const auto &item:cards) { int token; if (!BuiltinState::integer({{"value",item}},"value",0,51,token) || seen.contains(token)) return false; seen.insert(token); out.push_back({token%13+1,token/13}); }
        return true;
    };
    bool over;
    const QString message=state.value("message").toString();
    const QStringList messages{"your_turn","push_both_blackjack","blackjack_win","dealer_blackjack","bust","you_win","dealer_win","push"};
    if (!decode(state.value("deck"),deck) || !decode(state.value("player"),player) || !decode(state.value("dealer"),dealer) || player.size()<2 || dealer.size()<2 || !BuiltinState::boolean(state,"roundOver",over) || !messages.contains(message)) return false;
    const int playerValue=handValue(player), dealerValue=handValue(dealer);
    const bool playerNatural=playerValue==21 && player.size()==2, dealerNatural=dealerValue==21 && dealer.size()==2;
    if (!over) {
        if (message!="your_turn" || playerValue>=21 || dealerNatural || dealer.size()!=2 || deck.empty()) return false;
    } else {
        const QString expected=playerNatural && dealerNatural?"push_both_blackjack":playerNatural?"blackjack_win":dealerNatural?"dealer_blackjack":playerValue>21?"bust":dealerValue>21 || playerValue>dealerValue?"you_win":playerValue<dealerValue?"dealer_win":"push";
        if (message!=expected || (!playerNatural && !dealerNatural && playerValue<=21 && dealerValue<17)) return false;
    }
    m_deck=deck; m_player=player; m_dealer=dealer; m_roundOver=over; m_message=message;
    emit cardsChanged(); emit stateChanged(); return true;
}
