// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameSettings.h"
#include "SettingsManager.h"
#include <QRegularExpression>

GameSettings::GameSettings(SettingsManager *settings, QObject *parent) : QObject(parent), m_settings(settings) {}
QString GameSettings::gameId() const { return m_gameId; }
QVariantMap GameSettings::schema() const { return m_schema; }

bool GameSettings::safeKey(const QString &key)
{
    static const QRegularExpression rx(QStringLiteral("^[A-Za-z0-9_.-]+(?:/[A-Za-z0-9_.-]+)*$"));
    return rx.match(key).hasMatch();
}

QString GameSettings::namespaced(const QString &key) const
{
    if (m_gameId.isEmpty() || !safeKey(key))
        return QString{};
    return QStringLiteral("games/%1/settings/%2").arg(m_gameId, key);
}

QVariant GameSettings::value(const QString &key) const
{
    return value(key, QVariant{});
}

QVariant GameSettings::value(const QString &key, const QVariant &fallback) const
{
    const QString full = namespaced(key);
    return full.isEmpty() || !m_settings ? fallback : m_settings->value(full, fallback);
}

void GameSettings::setValue(const QString &key, const QVariant &value)
{
    const QString full = namespaced(key);
    if (full.isEmpty() || !m_settings)
        return;
    m_settings->setValue(full, value);
    emit valueChanged(key, value);
}

bool GameSettings::contains(const QString &key) const
{
    const QString full = namespaced(key);
    return !full.isEmpty() && m_settings && m_settings->contains(full);
}

void GameSettings::remove(const QString &key)
{
    const QString full = namespaced(key);
    if (full.isEmpty() || !m_settings)
        return;
    m_settings->remove(full);
    emit valueChanged(key, QVariant{});
}

void GameSettings::clear()
{
    if (m_gameId.isEmpty() || !m_settings)
        return;
    m_settings->remove(QStringLiteral("games/%1/settings").arg(m_gameId));
}

void GameSettings::activate(const QString &gameId, const QVariantMap &schema)
{
    if (m_gameId == gameId && m_schema == schema)
        return;
    m_gameId = gameId;
    m_schema = schema;
    emit gameChanged();
}
