// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <array>
#include <vector>

class MinesweeperGame final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rows READ rows NOTIFY boardReset)
    Q_PROPERTY(int columns READ columns NOTIFY boardReset)
    Q_PROPERTY(int mineCount READ mineCount NOTIFY boardReset)
    Q_PROPERTY(int flagsRemaining READ flagsRemaining NOTIFY cellsChanged)
    Q_PROPERTY(QVariantList cells READ cells NOTIFY cellsChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY statusChanged)
    Q_PROPERTY(bool won READ won NOTIFY statusChanged)
    Q_PROPERTY(int difficulty READ difficulty WRITE setDifficulty NOTIFY boardReset)
    Q_PROPERTY(int elapsedSeconds READ elapsedSeconds NOTIFY timeChanged)
    Q_PROPERTY(int bestSeconds READ bestSeconds NOTIFY statsChanged)
    Q_PROPERTY(int wins READ wins NOTIFY statsChanged)

public:
    explicit MinesweeperGame(QObject *parent = nullptr);

    int rows() const;
    int columns() const;
    int mineCount() const;
    int flagsRemaining() const;
    QVariantList cells() const;
    QString status() const;
    bool gameOver() const;
    bool won() const;
    int difficulty() const;
    int elapsedSeconds() const;
    int bestSeconds() const;
    int wins() const;

    Q_INVOKABLE void reset();
    Q_INVOKABLE bool openCell(int index);
    Q_INVOKABLE bool toggleFlag(int index);
    Q_INVOKABLE void setDifficulty(int level);

signals:
    void cellsChanged();
    void statusChanged();
    void boardReset();
    void timeChanged();
    void statsChanged();

private:
    struct Cell {
        bool mine = false;
        bool revealed = false;
        bool flagged = false;
        int adjacent = 0;
    };

    bool validIndex(int index) const;
    std::vector<int> neighbors(int index) const;
    void placeMines(int firstIndex);
    void revealFlood(int start);
    void revealMines();
    void checkWin();
    void finishTimer();
    void loadStats();
    void saveStats() const;

    int m_rows = 9;
    int m_columns = 9;
    int m_mineCount = 10;
    int m_difficulty = 0;
    bool m_started = false;
    bool m_gameOver = false;
    bool m_won = false;
    QString m_status = QStringLiteral("ready");
    std::vector<Cell> m_cells;

    QTimer m_timer;
    int m_elapsedSeconds = 0;
    std::array<int, 3> m_bestSeconds{0, 0, 0};
    std::array<int, 3> m_wins{0, 0, 0};
};
