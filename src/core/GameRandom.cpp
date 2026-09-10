// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameRandom.h"
#include <QCryptographicHash>
#include <QtGlobal>
#include <algorithm>

GameRandom::GameRandom(QObject *parent) : QObject(parent) {}
QString GameRandom::state() const { return QString::number(m_state, 16); }
void GameRandom::setState(const QString &text)
{
    bool ok = false;
    const quint64 value = text.toULongLong(&ok, 16);
    if (!ok || value == 0 || value == m_state) return;
    m_state = value;
    emit stateChanged();
}
void GameRandom::seed(const QString &seedText)
{
    const QByteArray hash = QCryptographicHash::hash(seedText.toUtf8(), QCryptographicHash::Sha256);
    quint64 value = 0;
    for (int i = 0; i < 8; ++i) value = (value << 8) | static_cast<unsigned char>(hash.at(i));
    if (value == 0) value = 0x9E3779B97F4A7C15ULL;
    if (value != m_state) { m_state = value; emit stateChanged(); }
}
quint64 GameRandom::next64()
{
    quint64 x = m_state;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    m_state = x;
    emit stateChanged();
    return x * 2685821657736338717ULL;
}
int GameRandom::nextInt(int minimumInclusive, int maximumInclusive)
{
    if (maximumInclusive < minimumInclusive) std::swap(minimumInclusive, maximumInclusive);
    const quint64 span = static_cast<quint64>(static_cast<qint64>(maximumInclusive) - minimumInclusive) + 1ULL;
    return minimumInclusive + static_cast<int>(next64() % span);
}
double GameRandom::nextReal() { return static_cast<double>(next64() >> 11) * (1.0 / 9007199254740992.0); }
bool GameRandom::chance(double probability) { return nextReal() < qBound(0.0, probability, 1.0); }
int GameRandom::pickIndex(int count) { return count > 0 ? nextInt(0, count - 1) : -1; }
QVariantList GameRandom::shuffled(const QVariantList &values)
{
    QVariantList out = values;
    for (int i = out.size() - 1; i > 0; --i) out.swapItemsAt(i, nextInt(0, i));
    return out;
}
