// SPDX-License-Identifier: GPL-3.0-or-later
#include "ReactionTapGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"

ReactionTapGame::ReactionTapGame(QObject *parent) : QObject(parent)
{
    GameLocalStats::migrateLegacy(QStringLiteral("reaction_tap"), {
        {QStringLiteral("games/reaction/bestMs"), QStringLiteral("custom/bestMs")}
    });
    m_bestMs = GameLocalStats::value(QStringLiteral("reaction_tap"), QStringLiteral("custom/bestMs"), 0).toInt();
    m_waitTimer.setSingleShot(true);
    connect(&m_waitTimer, &QTimer::timeout, this, [this] {
        m_state = QStringLiteral("ready");
        m_elapsed.restart();
        emit stateChanged();
    });
}

QString ReactionTapGame::state() const { return m_state; }
int ReactionTapGame::lastMs() const { return m_lastMs; }
int ReactionTapGame::bestMs() const { return m_bestMs; }
int ReactionTapGame::rounds() const { return m_rounds; }
int ReactionTapGame::averageMs() const
{
    return m_rounds > 0 ? static_cast<int>(m_totalMs / m_rounds) : 0;
}

void ReactionTapGame::startRound()
{
    m_waitTimer.stop();
    m_state = QStringLiteral("waiting");
    const int delay = 1200 + QRandomGenerator::global()->bounded(2301);
    m_waitTimer.start(delay);
    emit stateChanged();
}

void ReactionTapGame::tap()
{
    if (m_state == QStringLiteral("waiting")) {
        m_waitTimer.stop();
        m_state = QStringLiteral("false_start");
        emit stateChanged();
        return;
    }

    if (m_state != QStringLiteral("ready"))
        return;

    m_lastMs = static_cast<int>(m_elapsed.elapsed());
    ++m_rounds;
    m_totalMs += m_lastMs;
    if (m_bestMs == 0 || m_lastMs < m_bestMs) {
        m_bestMs = m_lastMs;
        GameLocalStats::setValues(QStringLiteral("reaction_tap"), {
            {QStringLiteral("custom/bestMs"), m_bestMs}
        });
    }
    m_state = QStringLiteral("result");
    emit statsChanged();
    emit stateChanged();
}
