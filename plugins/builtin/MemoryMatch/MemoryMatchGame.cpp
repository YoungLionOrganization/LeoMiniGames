// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "MemoryMatchGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"
#include <QStringList>
#include <QTimer>
#include <algorithm>

MemoryMatchGame::MemoryMatchGame(QObject *parent) : QObject(parent)
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
        ++m_elapsedSeconds;
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
    m_timer.stop();
    m_elapsedSeconds = 0;

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
    m_timer.start();
}

bool MemoryMatchGame::flip(int index)
{
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
            m_timer.stop();
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

    QTimer::singleShot(650, this, [this, firstIndex, secondIndex] {
        if (firstIndex >= 0 && firstIndex < static_cast<int>(m_cards.size()))
            m_cards[static_cast<std::size_t>(firstIndex)].faceUp = false;
        if (secondIndex >= 0 && secondIndex < static_cast<int>(m_cards.size()))
            m_cards[static_cast<std::size_t>(secondIndex)].faceUp = false;

        m_locked = false;
        emit cardsChanged();
        emit lockedChanged();
    });
}
