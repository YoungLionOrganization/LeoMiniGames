// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QVariantList>
#include <array>

class TwentyFortyEightGame final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList tiles READ tiles NOTIFY boardChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int bestScore READ bestScore NOTIFY scoreChanged)
    Q_PROPERTY(int moves READ moves NOTIFY scoreChanged)
    Q_PROPERTY(int highestTile READ highestTile NOTIFY boardChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY stateChanged)
    Q_PROPERTY(bool won READ won NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)

public:
    explicit TwentyFortyEightGame(QObject *parent = nullptr);

    QVariantList tiles() const;
    int score() const;
    int bestScore() const;
    int moves() const;
    int highestTile() const;
    bool gameOver() const;
    bool won() const;
    QString status() const;

    Q_INVOKABLE void reset();
    Q_INVOKABLE bool move(const QString &direction);

signals:
    void boardChanged();
    void scoreChanged();
    void stateChanged();

private:
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();
    bool collapseLine(std::array<int, 4> &line);
    void addRandomTile();
    bool hasMoves() const;
    void updateState();

    std::array<int, 16> m_tiles{};
    int m_score = 0;
    int m_bestScore = 0;
    int m_moves = 0;
    bool m_gameOver = false;
    bool m_won = false;
    QString m_status = QStringLiteral("playing");
};
