// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "XoxGame.h"

#include <array>
#include "sdk/GameLocalStats.h"

XoxGame::XoxGame(QObject *parent) : QObject(parent)
{
    GameLocalStats::migrateLegacy(QStringLiteral("xox"), {
        {QStringLiteral("games/xox/xWins"), QStringLiteral("counter/xWins")},
        {QStringLiteral("games/xox/oWins"), QStringLiteral("counter/oWins")},
        {QStringLiteral("games/xox/draws"), QStringLiteral("counter/draws")}
    });
    m_xWins = GameLocalStats::value(QStringLiteral("xox"), QStringLiteral("counter/xWins"), 0).toInt();
    m_oWins = GameLocalStats::value(QStringLiteral("xox"), QStringLiteral("counter/oWins"), 0).toInt();
    m_draws = GameLocalStats::value(QStringLiteral("xox"), QStringLiteral("counter/draws"), 0).toInt();
    m_board.fill(QString(), 9);
}

QStringList XoxGame::board() const { return m_board; }
QString XoxGame::currentPlayer() const { return m_currentPlayer; }
QString XoxGame::status() const { return m_status; }
bool XoxGame::gameOver() const { return m_gameOver; }
int XoxGame::xWins() const { return m_xWins; }
int XoxGame::oWins() const { return m_oWins; }
int XoxGame::draws() const { return m_draws; }

bool XoxGame::hasWinner(const QString &symbol) const
{
    static constexpr std::array<std::array<int, 3>, 8> lines{{
        {{0,1,2}}, {{3,4,5}}, {{6,7,8}},
        {{0,3,6}}, {{1,4,7}}, {{2,5,8}},
        {{0,4,8}}, {{2,4,6}}
    }};
    for (const auto &line : lines) {
        if (m_board[line[0]] == symbol && m_board[line[1]] == symbol && m_board[line[2]] == symbol)
            return true;
    }
    return false;
}

bool XoxGame::isFull() const
{
    for (const auto &cell : m_board)
        if (cell.isEmpty()) return false;
    return true;
}

bool XoxGame::play(int index)
{
    if (m_gameOver || index < 0 || index >= m_board.size() || !m_board[index].isEmpty())
        return false;

    const QString played = m_currentPlayer;
    m_board[index] = played;
    emit boardChanged();

    if (hasWinner(played)) {
        m_gameOver = true;
        m_status = QStringLiteral("win");
        if (played == QStringLiteral("X")) ++m_xWins; else ++m_oWins;
        saveScore();
        emit scoreChanged();
        emit gameOverChanged();
        emit statusChanged();
        return true;
    }

    if (isFull()) {
        m_gameOver = true;
        m_status = QStringLiteral("draw");
        ++m_draws;
        saveScore();
        emit scoreChanged();
        emit gameOverChanged();
        emit statusChanged();
        return true;
    }

    m_currentPlayer = (m_currentPlayer == QStringLiteral("X")) ? QStringLiteral("O") : QStringLiteral("X");
    m_status = QStringLiteral("turn");
    emit currentPlayerChanged();
    emit statusChanged();
    return true;
}

void XoxGame::newRound()
{
    m_board.fill(QString(), 9);
    m_currentPlayer = QStringLiteral("X");
    m_status = QStringLiteral("turn");
    m_gameOver = false;
    emit boardChanged();
    emit currentPlayerChanged();
    emit statusChanged();
    emit gameOverChanged();
}

void XoxGame::resetScore()
{
    m_xWins = m_oWins = m_draws = 0;
    saveScore();
    emit scoreChanged();
    newRound();
}

void XoxGame::saveScore() const
{
    GameLocalStats::setValues(QStringLiteral("xox"), {
        {QStringLiteral("counter/xWins"), m_xWins},
        {QStringLiteral("counter/oWins"), m_oWins},
        {QStringLiteral("counter/draws"), m_draws}
    });
}
