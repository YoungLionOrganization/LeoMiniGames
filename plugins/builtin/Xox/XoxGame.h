// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QObject>
#include <QStringList>

class XoxGame final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList board READ board NOTIFY boardChanged)
    Q_PROPERTY(QString currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY gameOverChanged)
    Q_PROPERTY(int xWins READ xWins NOTIFY scoreChanged)
    Q_PROPERTY(int oWins READ oWins NOTIFY scoreChanged)
    Q_PROPERTY(int draws READ draws NOTIFY scoreChanged)

public:
    explicit XoxGame(QObject *parent = nullptr);

    QStringList board() const;
    QString currentPlayer() const;
    QString status() const;
    bool gameOver() const;
    int xWins() const;
    int oWins() const;
    int draws() const;

    Q_INVOKABLE bool play(int index);
    Q_INVOKABLE void newRound();
    Q_INVOKABLE void resetScore();

signals:
    void boardChanged();
    void currentPlayerChanged();
    void statusChanged();
    void gameOverChanged();
    void scoreChanged();

private:
    bool hasWinner(const QString &symbol) const;
    bool isFull() const;
    void saveScore() const;

    QStringList m_board;
    QString m_currentPlayer = QStringLiteral("X");
    QString m_status = QStringLiteral("turn");
    bool m_gameOver = false;
    int m_xWins = 0;
    int m_oWins = 0;
    int m_draws = 0;
};
