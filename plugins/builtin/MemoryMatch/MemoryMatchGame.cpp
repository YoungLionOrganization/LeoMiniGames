// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "MemoryMatchGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"
#include "sdk/BuiltinState.h"
#include <QStringList>
#include <QTimer>
#include <algorithm>

MemoryMatchGame::MemoryMatchGame(QObject *parent) : BuiltinGame(parent)
{
    GameLocalStats::migrateLegacy(QStringLiteral("memory_match"), {
        {QStringLiteral("games/memory/bestMoves"), QStringLiteral("custom/bestMoves")},
        {QStringLiteral("games/memory/bestSeconds"), QStringLiteral("custom/bestSeconds")},
        {QStringLiteral("games/memory/wins"), QStringLiteral("counter/wins")}
    });
    m_bestMoves = GameLocalStats::value(QStringLiteral("memory_match"), QStringLiteral("custom/bestMoves"), 0).toInt();
    m_bestSeconds = GameLocalStats::value(QStringLiteral("memory_match"), QStringLiteral("custom/bestSeconds"), 0).toInt();
    m_wins = GameLocalStats::value(QStringLiteral("memory_match"), QStringLiteral("counter/wins"), 0).toInt();

    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        m_elapsedSeconds = static_cast<int>((m_activeBeforePause + m_playClock.elapsed()) / 1000);
        emit timeChanged();
    });
    reset();
}

QVariantList MemoryMatchGame::cards() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_cards.size()));
    for (const Card &card : m_cards) {
        QVariantMap map;
        map.insert(QStringLiteral("symbol"), card.symbol);
        map.insert(QStringLiteral("faceUp"), card.faceUp);
        map.insert(QStringLiteral("matched"), card.matched);
        result.append(map);
    }
    return result;
}

int MemoryMatchGame::moves() const { return m_moves; }
int MemoryMatchGame::matches() const { return m_matches; }
int MemoryMatchGame::bestMoves() const { return m_bestMoves; }
int MemoryMatchGame::elapsedSeconds() const { return m_elapsedSeconds; }
int MemoryMatchGame::bestSeconds() const { return m_bestSeconds; }
int MemoryMatchGame::wins() const { return m_wins; }
bool MemoryMatchGame::locked() const { return m_locked; }
bool MemoryMatchGame::gameOver() const { return m_gameOver; }
QString MemoryMatchGame::status() const { return m_status; }

void MemoryMatchGame::reset()
{
    if (m_paused) return;
    m_pairTimer.stop(); m_pairRemaining = -1;
    m_timer.stop();
    m_elapsedSeconds = 0; m_activeBeforePause = 0;

    static const QStringList symbols{
        QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D"),
        QStringLiteral("E"), QStringLiteral("F"), QStringLiteral("G"), QStringLiteral("H")
    };

    m_cards.clear();
    m_cards.reserve(16);
    for (const QString &symbol : symbols) {
        m_cards.push_back(Card{symbol, false, false});
        m_cards.push_back(Card{symbol, false, false});
    }

    for (int i = static_cast<int>(m_cards.size()) - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(m_cards[static_cast<std::size_t>(i)], m_cards[static_cast<std::size_t>(j)]);
    }

    m_firstIndex = -1;
    m_moves = 0;
    m_matches = 0;
    m_locked = false;
    m_gameOver = false;
    m_status = QStringLiteral("playing");

    emit cardsChanged();
    emit statsChanged();
    emit lockedChanged();
    emit stateChanged();
    emit timeChanged();
    m_playClock.restart();
    m_timer.start();
}

bool MemoryMatchGame::flip(int index)
{
    if (m_paused) return false;
    if (m_locked || m_gameOver || index < 0 || index >= static_cast<int>(m_cards.size()))
        return false;

    Card &card = m_cards[static_cast<std::size_t>(index)];
    if (card.faceUp || card.matched)
        return false;

    card.faceUp = true;
    emit cardsChanged();

    if (m_firstIndex < 0) {
        m_firstIndex = index;
        return true;
    }

    ++m_moves;
    emit statsChanged();

    resolvePair(index);
    return true;
}

void MemoryMatchGame::resolvePair(int secondIndex)
{
    Card &first = m_cards[static_cast<std::size_t>(m_firstIndex)];
    Card &second = m_cards[static_cast<std::size_t>(secondIndex)];

    if (first.symbol == second.symbol) {
        first.matched = true;
        second.matched = true;
        m_firstIndex = -1;
        ++m_matches;
        emit cardsChanged();
        emit statsChanged();
        emit pairResolved(true);

        if (m_matches == 8) {
            m_gameOver = true;
            m_status = QStringLiteral("cleared");
            m_activeBeforePause += m_playClock.elapsed();
            m_timer.stop();
            m_elapsedSeconds = static_cast<int>(m_activeBeforePause / 1000);
            ++m_wins;
            if (m_bestMoves == 0 || m_moves < m_bestMoves)
                m_bestMoves = m_moves;
            if (m_bestSeconds == 0 || m_elapsedSeconds < m_bestSeconds)
                m_bestSeconds = m_elapsedSeconds;

            GameLocalStats::setValues(QStringLiteral("memory_match"), {
                {QStringLiteral("custom/bestMoves"), m_bestMoves},
                {QStringLiteral("custom/bestSeconds"), m_bestSeconds},
                {QStringLiteral("counter/wins"), m_wins}
            });

            emit statsChanged();
            emit stateChanged();
        }
        return;
    }

    emit pairResolved(false);

    const int firstIndex = m_firstIndex;
    m_firstIndex = -1;
    m_locked = true;
    emit lockedChanged();

    m_pairTimer.disconnect(this);
    m_pairTimer.setSingleShot(true);
    connect(&m_pairTimer, &QTimer::timeout, this, [this, firstIndex, secondIndex] {
        if (firstIndex >= 0 && firstIndex < static_cast<int>(m_cards.size()))
            m_cards[static_cast<std::size_t>(firstIndex)].faceUp = false;
        if (secondIndex >= 0 && secondIndex < static_cast<int>(m_cards.size()))
            m_cards[static_cast<std::size_t>(secondIndex)].faceUp = false;

        m_locked = false;
        emit cardsChanged();
        emit lockedChanged();
    });
    m_pairTimer.start(650);
}

void MemoryMatchGame::pause() {
    BuiltinGame::pause();
    if (m_paused) return;
    m_paused = true; m_resumeTimer = m_timer.isActive();
    if (m_resumeTimer) m_activeBeforePause += m_playClock.elapsed();
    m_timer.stop();
    m_pairRemaining = m_pairTimer.isActive() ? m_pairTimer.remainingTime() : -1; m_pairTimer.stop();
}
void MemoryMatchGame::resume() {
    BuiltinGame::resume();
    if (!m_paused) return;
    m_paused = false;
    if (m_resumeTimer) { m_playClock.restart(); m_timer.start(); }
    if (m_pairRemaining >= 0) m_pairTimer.start(qMax(1, m_pairRemaining)); m_pairRemaining = -1;
}

QVariantMap MemoryMatchGame::snapshot() const {
    QVariantList stateCards = cards();
    // A delayed mismatch is settled on restore; a single selected card survives.
    if (m_locked) for (QVariant &value:stateCards) { auto card=value.toMap(); if (!card.value("matched").toBool()) card["faceUp"]=false; value=card; }
    const qint64 milliseconds=m_activeBeforePause + (m_timer.isActive() && m_playClock.isValid() ? m_playClock.elapsed() : 0);
    return {{"cards",stateCards},{"moves",m_moves},{"elapsedMs",milliseconds}};
}
bool MemoryMatchGame::restoreSnapshot(const QVariantMap &state) {
    const QVariantList values=state.value("cards").toList(); int moves, elapsed;
    if (values.size()!=16 || !BuiltinState::integer(state,"moves",0,BuiltinState::maxCounter(),moves) || !BuiltinState::integer(state,"elapsedMs",0,BuiltinState::maxCounter(),elapsed)) return false;
    std::vector<Card> cards; QHash<QString,int> counts, matched; int first=-1, matchedCount=0;
    for (int i=0;i<16;++i) {
        const auto value=values[i].toMap(); const QString symbol=value.value("symbol").toString(); bool up,match;
        if (symbol.size()!=1 || symbol<"A" || symbol>"H" || !BuiltinState::boolean(value,"faceUp",up) || !BuiltinState::boolean(value,"matched",match) || (match && !up)) return false;
        if (up && !match) { if (first>=0) return false; first=i; }
        ++counts[symbol]; if (match) { ++matched[symbol]; ++matchedCount; }
        cards.push_back({symbol,up,match});
    }
    if (counts.size()!=8 || moves<matchedCount/2) return false;
    for (auto it=counts.cbegin();it!=counts.cend();++it) if (it.value()!=2 || (matched.value(it.key())!=0 && matched.value(it.key())!=2)) return false;
    m_timer.stop(); m_pairTimer.stop(); m_pairRemaining=-1; m_cards=cards; m_firstIndex=first; m_moves=moves; m_matches=matchedCount/2; m_activeBeforePause=elapsed; m_elapsedSeconds=elapsed/1000;
    m_locked=false; m_gameOver=m_matches==8; m_status=m_gameOver?"cleared":"playing"; m_resumeTimer=!m_gameOver;
    m_playClock.restart(); if (!m_paused && !m_gameOver) m_timer.start();
    emit cardsChanged(); emit statsChanged(); emit lockedChanged(); emit stateChanged(); emit timeChanged(); return true;
}
