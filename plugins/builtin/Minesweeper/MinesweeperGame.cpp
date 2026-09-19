// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "MinesweeperGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"
#include <QSet>
#include <QtGlobal>
#include <queue>

MinesweeperGame::MinesweeperGame(QObject *parent) : QObject(parent)
{
    loadStats();
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        ++m_elapsedSeconds;
        emit timeChanged();
    });
    reset();
}

int MinesweeperGame::rows() const { return m_rows; }
int MinesweeperGame::columns() const { return m_columns; }
int MinesweeperGame::mineCount() const { return m_mineCount; }
int MinesweeperGame::difficulty() const { return m_difficulty; }
QString MinesweeperGame::status() const { return m_status; }
bool MinesweeperGame::gameOver() const { return m_gameOver; }
bool MinesweeperGame::won() const { return m_won; }
int MinesweeperGame::elapsedSeconds() const { return m_elapsedSeconds; }
int MinesweeperGame::bestSeconds() const { return m_bestSeconds[static_cast<std::size_t>(m_difficulty)]; }
int MinesweeperGame::wins() const { return m_wins[static_cast<std::size_t>(m_difficulty)]; }

int MinesweeperGame::flagsRemaining() const
{
    int flags = 0;
    for (const auto &cell : m_cells)
        if (cell.flagged) ++flags;
    return m_mineCount - flags;
}

QVariantList MinesweeperGame::cells() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_cells.size()));
    for (const auto &cell : m_cells) {
        QVariantMap map;
        map.insert(QStringLiteral("revealed"), cell.revealed);
        map.insert(QStringLiteral("flagged"), cell.flagged);
        map.insert(QStringLiteral("mine"), cell.mine && (m_gameOver || cell.revealed));
        map.insert(QStringLiteral("adjacent"), cell.adjacent);
        result.append(map);
    }
    return result;
}

bool MinesweeperGame::validIndex(int index) const
{
    return index >= 0 && index < static_cast<int>(m_cells.size());
}

std::vector<int> MinesweeperGame::neighbors(int index) const
{
    std::vector<int> out;
    const int row = index / m_columns;
    const int col = index % m_columns;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            const int r = row + dr;
            const int c = col + dc;
            if (r >= 0 && r < m_rows && c >= 0 && c < m_columns)
                out.push_back(r * m_columns + c);
        }
    }
    return out;
}

void MinesweeperGame::reset()
{
    m_timer.stop();
    m_elapsedSeconds = 0;
    m_cells.assign(static_cast<std::size_t>(m_rows * m_columns), Cell{});
    m_started = false;
    m_gameOver = false;
    m_won = false;
    m_status = QStringLiteral("ready");
    emit boardReset();
    emit cellsChanged();
    emit statusChanged();
    emit timeChanged();
    emit statsChanged();
}

void MinesweeperGame::setDifficulty(int level)
{
    level = qBound(0, level, 2);
    if (m_difficulty == level && !m_cells.empty())
        return;

    m_difficulty = level;
    if (level == 0) {
        m_rows = 9; m_columns = 9; m_mineCount = 10;
    } else if (level == 1) {
        m_rows = 12; m_columns = 12; m_mineCount = 22;
    } else {
        m_rows = 16; m_columns = 16; m_mineCount = 40;
    }
    reset();
}

void MinesweeperGame::placeMines(int firstIndex)
{
    QSet<int> forbidden;
    forbidden.insert(firstIndex);
    for (int n : neighbors(firstIndex))
        forbidden.insert(n);

    int placed = 0;
    while (placed < m_mineCount) {
        const int index = QRandomGenerator::global()->bounded(static_cast<int>(m_cells.size()));
        if (forbidden.contains(index) || m_cells[static_cast<std::size_t>(index)].mine)
            continue;
        m_cells[static_cast<std::size_t>(index)].mine = true;
        ++placed;
    }

    for (int i = 0; i < static_cast<int>(m_cells.size()); ++i) {
        if (m_cells[static_cast<std::size_t>(i)].mine)
            continue;
        int count = 0;
        for (int n : neighbors(i))
            if (m_cells[static_cast<std::size_t>(n)].mine) ++count;
        m_cells[static_cast<std::size_t>(i)].adjacent = count;
    }
}

void MinesweeperGame::revealFlood(int start)
{
    std::queue<int> queue;
    queue.push(start);

    while (!queue.empty()) {
        const int index = queue.front();
        queue.pop();

        Cell &cell = m_cells[static_cast<std::size_t>(index)];
        if (cell.revealed || cell.flagged || cell.mine)
            continue;

        cell.revealed = true;
        if (cell.adjacent == 0) {
            for (int n : neighbors(index)) {
                const Cell &neighbor = m_cells[static_cast<std::size_t>(n)];
                if (!neighbor.revealed && !neighbor.flagged && !neighbor.mine)
                    queue.push(n);
            }
        }
    }
}

bool MinesweeperGame::openCell(int index)
{
    if (m_gameOver || !validIndex(index))
        return false;

    Cell &cell = m_cells[static_cast<std::size_t>(index)];
    if (cell.flagged || cell.revealed)
        return false;

    if (!m_started) {
        placeMines(index);
        m_started = true;
        m_status = QStringLiteral("playing");
        m_timer.start();
    }

    if (cell.mine) {
        cell.revealed = true;
        m_gameOver = true;
        m_won = false;
        m_status = QStringLiteral("mine_hit");
        revealMines();
        finishTimer();
    } else {
        revealFlood(index);
        checkWin();
    }

    emit cellsChanged();
    emit statusChanged();
    return true;
}

bool MinesweeperGame::toggleFlag(int index)
{
    if (m_gameOver || !validIndex(index))
        return false;

    Cell &cell = m_cells[static_cast<std::size_t>(index)];
    if (cell.revealed)
        return false;
    if (!cell.flagged && flagsRemaining() <= 0)
        return false;

    cell.flagged = !cell.flagged;
    emit cellsChanged();
    return true;
}

void MinesweeperGame::revealMines()
{
    for (auto &cell : m_cells)
        if (cell.mine) cell.revealed = true;
}

void MinesweeperGame::checkWin()
{
    int hiddenSafe = 0;
    for (const auto &cell : m_cells)
        if (!cell.mine && !cell.revealed) ++hiddenSafe;

    if (hiddenSafe != 0)
        return;

    m_gameOver = true;
    m_won = true;
    m_status = QStringLiteral("cleared");
    for (auto &cell : m_cells)
        if (cell.mine) cell.flagged = true;

    finishTimer();

    const std::size_t difficulty = static_cast<std::size_t>(m_difficulty);
    ++m_wins[difficulty];
    if (m_bestSeconds[difficulty] == 0 || m_elapsedSeconds < m_bestSeconds[difficulty])
        m_bestSeconds[difficulty] = m_elapsedSeconds;

    saveStats();
    emit statsChanged();
}

void MinesweeperGame::finishTimer()
{
    if (m_timer.isActive())
        m_timer.stop();
}

void MinesweeperGame::loadStats()
{
    QHash<QString, QString> migration;
    for (int i = 0; i < 3; ++i) {
        migration.insert(QStringLiteral("games/minesweeper/%1/bestSeconds").arg(i),
                         QStringLiteral("custom/%1/bestSeconds").arg(i));
        migration.insert(QStringLiteral("games/minesweeper/%1/wins").arg(i),
                         QStringLiteral("counter/%1/wins").arg(i));
    }
    GameLocalStats::migrateLegacy(QStringLiteral("minesweeper"), migration);
    for (int i = 0; i < 3; ++i) {
        m_bestSeconds[static_cast<std::size_t>(i)] =
            GameLocalStats::value(QStringLiteral("minesweeper"), QStringLiteral("custom/%1/bestSeconds").arg(i), 0).toInt();
        m_wins[static_cast<std::size_t>(i)] =
            GameLocalStats::value(QStringLiteral("minesweeper"), QStringLiteral("counter/%1/wins").arg(i), 0).toInt();
    }
}

void MinesweeperGame::saveStats() const
{
    QVariantMap values;
    for (int i = 0; i < 3; ++i) {
        values.insert(QStringLiteral("custom/%1/bestSeconds").arg(i),
                      m_bestSeconds[static_cast<std::size_t>(i)]);
        values.insert(QStringLiteral("counter/%1/wins").arg(i),
                      m_wins[static_cast<std::size_t>(i)]);
    }
    GameLocalStats::setValues(QStringLiteral("minesweeper"), values);
}
