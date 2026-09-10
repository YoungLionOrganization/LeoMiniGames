// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QVariantList>

class GameRandom final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)
public:
    explicit GameRandom(QObject *parent = nullptr);
    QString state() const;
    void setState(const QString &state);
    Q_INVOKABLE void seed(const QString &seedText);
    Q_INVOKABLE int nextInt(int minimumInclusive, int maximumInclusive);
    Q_INVOKABLE double nextReal();
    Q_INVOKABLE bool chance(double probability);
    Q_INVOKABLE int pickIndex(int count);
    Q_INVOKABLE QVariantList shuffled(const QVariantList &values);
signals:
    void stateChanged();
private:
    quint64 next64();
    quint64 m_state = 0x9E3779B97F4A7C15ULL;
};
