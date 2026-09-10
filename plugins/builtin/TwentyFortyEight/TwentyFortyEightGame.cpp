// SPDX-License-Identifier: GPL-3.0-or-later
#include "TwentyFortyEightGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"
#include <QString>
#include <vector>

TwentyFortyEightGame::TwentyFortyEightGame(QObject *parent) : QObject(parent)
{
    GameLocalStats::migrateLegacy(QStringLiteral("2048"), {
        {QStringLiteral("games/2048/bestScore"), QStringLiteral("high/default")}
    });
    m_bestScore = GameLocalStats::value(QStringLiteral("2048"), QStringLiteral("high/default"), 0).toInt();
    reset();
}

QVariantList TwentyFortyEightGame::tiles() const
{
    QVariantList result;
    result.reserve(16);
    for (int value : m_tiles)
        result.append(value);
    return result;
}

int TwentyFortyEightGame::score() const { return m_score; }
int TwentyFortyEightGame::bestScore() const { return m_bestScore; }
int TwentyFortyEightGame::moves() const { return m_moves; }

int TwentyFortyEightGame::highestTile() const
{
    int highest = 0;
    for (int value : m_tiles)
        if (value > highest) highest = value;
    return highest;
}
bool TwentyFortyEightGame::gameOver() const { return m_gameOver; }
bool TwentyFortyEightGame::won() const { return m_won; }
QString TwentyFortyEightGame::status() const { return m_status; }

void TwentyFortyEightGame::reset()
{
    m_tiles.fill(0);
    m_score = 0;
    m_moves = 0;
    m_gameOver = false;
    m_won = false;
    m_status = QStringLiteral("playing");
    addRandomTile();
    addRandomTile();
    emit boardChanged();
    emit scoreChanged();
    emit stateChanged();
}

bool TwentyFortyEightGame::collapseLine(std::array<int, 4> &line)
{
    const std::array<int, 4> original = line;
    std::vector<int> values;
    for (int value : line)
        if (value != 0) values.push_back(value);

    std::vector<int> merged;
    for (std::size_t i = 0; i < values.size();) {
        if (i + 1 < values.size() && values[i] == values[i + 1]) {
            const int value = values[i] * 2;
            merged.push_back(value);
            m_score += value;
            i += 2;
        } else {
            merged.push_back(values[i]);
            ++i;
        }
    }

    line.fill(0);
    for (std::size_t i = 0; i < merged.size() && i < 4; ++i)
        line[i] = merged[i];

    return line != original;
}

bool TwentyFortyEightGame::moveLeft()
{
    bool changed = false;
    for (int row = 0; row < 4; ++row) {
        std::array<int, 4> line{};
        for (int col = 0; col < 4; ++col)
            line[static_cast<std::size_t>(col)] = m_tiles[static_cast<std::size_t>(row * 4 + col)];
        if (collapseLine(line))
            changed = true;
        for (int col = 0; col < 4; ++col)
            m_tiles[static_cast<std::size_t>(row * 4 + col)] = line[static_cast<std::size_t>(col)];
    }
    return changed;
}

bool TwentyFortyEightGame::moveRight()
{
    bool changed = false;
    for (int row = 0; row < 4; ++row) {
        std::array<int, 4> line{};
        for (int col = 0; col < 4; ++col)
            line[static_cast<std::size_t>(col)] = m_tiles[static_cast<std::size_t>(row * 4 + (3 - col))];
        if (collapseLine(line))
            changed = true;
        for (int col = 0; col < 4; ++col)
            m_tiles[static_cast<std::size_t>(row * 4 + (3 - col))] = line[static_cast<std::size_t>(col)];
    }
    return changed;
}

bool TwentyFortyEightGame::moveUp()
{
    bool changed = false;
    for (int col = 0; col < 4; ++col) {
        std::array<int, 4> line{};
        for (int row = 0; row < 4; ++row)
            line[static_cast<std::size_t>(row)] = m_tiles[static_cast<std::size_t>(row * 4 + col)];
        if (collapseLine(line))
            changed = true;
        for (int row = 0; row < 4; ++row)
            m_tiles[static_cast<std::size_t>(row * 4 + col)] = line[static_cast<std::size_t>(row)];
    }
    return changed;
}

bool TwentyFortyEightGame::moveDown()
{
    bool changed = false;
    for (int col = 0; col < 4; ++col) {
        std::array<int, 4> line{};
        for (int row = 0; row < 4; ++row)
            line[static_cast<std::size_t>(row)] = m_tiles[static_cast<std::size_t>((3 - row) * 4 + col)];
        if (collapseLine(line))
            changed = true;
        for (int row = 0; row < 4; ++row)
            m_tiles[static_cast<std::size_t>((3 - row) * 4 + col)] = line[static_cast<std::size_t>(row)];
    }
    return changed;
}

bool TwentyFortyEightGame::move(const QString &direction)
{
    if (m_gameOver)
        return false;

    bool changed = false;
    if (direction == QStringLiteral("left")) changed = moveLeft();
    else if (direction == QStringLiteral("right")) changed = moveRight();
    else if (direction == QStringLiteral("up")) changed = moveUp();
    else if (direction == QStringLiteral("down")) changed = moveDown();

    if (!changed)
        return false;

    addRandomTile();
    ++m_moves;
    if (m_score > m_bestScore) {
        m_bestScore = m_score;
        GameLocalStats::setValues(QStringLiteral("2048"), {
            {QStringLiteral("high/default"), m_bestScore}
        });
    }

    updateState();
    emit boardChanged();
    emit scoreChanged();
    emit stateChanged();
    return true;
}

void TwentyFortyEightGame::addRandomTile()
{
    std::vector<int> empty;
    for (int i = 0; i < 16; ++i)
        if (m_tiles[static_cast<std::size_t>(i)] == 0) empty.push_back(i);

    if (empty.empty())
        return;

    const int pick = QRandomGenerator::global()->bounded(static_cast<int>(empty.size()));
    const int index = empty[static_cast<std::size_t>(pick)];
    m_tiles[static_cast<std::size_t>(index)] =
        QRandomGenerator::global()->bounded(10) == 0 ? 4 : 2;
}

bool TwentyFortyEightGame::hasMoves() const
{
    for (int value : m_tiles)
        if (value == 0) return true;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            const int value = m_tiles[static_cast<std::size_t>(row * 4 + col)];
            if (col < 3 && value == m_tiles[static_cast<std::size_t>(row * 4 + col + 1)])
                return true;
            if (row < 3 && value == m_tiles[static_cast<std::size_t>((row + 1) * 4 + col)])
                return true;
        }
    }
    return false;
}

void TwentyFortyEightGame::updateState()
{
    bool reached2048 = false;
    for (int value : m_tiles)
        if (value >= 2048) reached2048 = true;

    const bool justWon = reached2048 && !m_won;
    if (justWon)
        m_won = true;

    if (!hasMoves()) {
        m_gameOver = true;
        m_status = QStringLiteral("game_over");
    } else if (justWon) {
        m_status = QStringLiteral("won");
    } else {
        m_status = QStringLiteral("playing");
    }
}
