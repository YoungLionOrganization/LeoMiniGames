// SPDX-License-Identifier: GPL-3.0-or-later
#include "LegacySettingsFacade.h"
#include "SettingsManager.h"
#include <QtGlobal>
#include <QStringList>

namespace {
bool isGlobalPreference(const QString &key)
{
    return key == QStringLiteral("audio/enabled") ||
           key == QStringLiteral("audio/volume") ||
           key == QStringLiteral("ui/animations");
}
bool isReservedHostKey(const QString &key)
{
    static const QStringList prefixes = {
        QStringLiteral("app/"), QStringLiteral("theme/"), QStringLiteral("themes/"),
        QStringLiteral("mods/installed"), QStringLiteral("developer/"),
        QStringLiteral("network/"), QStringLiteral("security/")
    };
    for (const QString &prefix : prefixes)
        if (key.startsWith(prefix, Qt::CaseInsensitive))
            return true;
    return false;
}
}

LegacySettingsFacade::LegacySettingsFacade(SettingsManager *settings, QObject *parent)
    : QObject(parent), m_settings(settings)
{
    if (!m_settings)
        return;
    connect(m_settings, &SettingsManager::soundEnabledChanged, this, &LegacySettingsFacade::soundEnabledChanged);
    connect(m_settings, &SettingsManager::soundVolumeChanged, this, &LegacySettingsFacade::soundVolumeChanged);
    connect(m_settings, &SettingsManager::animationsEnabledChanged, this, &LegacySettingsFacade::animationsEnabledChanged);
}

bool LegacySettingsFacade::soundEnabled() const { return m_settings && m_settings->soundEnabled(); }
qreal LegacySettingsFacade::soundVolume() const { return m_settings ? m_settings->soundVolume() : 0.0; }
bool LegacySettingsFacade::animationsEnabled() const { return !m_settings || m_settings->animationsEnabled(); }
void LegacySettingsFacade::activate(const QString &gameId) { m_gameId = gameId.trimmed(); }

bool LegacySettingsFacade::safeKey(const QString &key)
{
    const QString k = key.trimmed();
    return !k.isEmpty() && k.size() <= 192 && !k.contains(QChar(u'\0')) &&
           !k.startsWith(QLatin1Char('/')) && !k.contains(QStringLiteral(".."));
}

bool LegacySettingsFacade::rawLegacyKeyAllowed(const QString &key) const
{
    if (m_gameId.isEmpty())
        return false;
    return key.startsWith(m_gameId + QLatin1Char('/')) ||
           key.startsWith(QStringLiteral("mods/%1/").arg(m_gameId)) ||
           key.startsWith(QStringLiteral("game/%1/").arg(m_gameId)) ||
           key.startsWith(QStringLiteral("games/%1/").arg(m_gameId));
}

QString LegacySettingsFacade::storageKey(const QString &key) const
{
    const QString k = key.trimmed();
    if (isGlobalPreference(k) || rawLegacyKeyAllowed(k))
        return k;
    if (m_gameId.isEmpty() || !safeKey(k))
        return QString{};
    return QStringLiteral("compat/mods/%1/%2").arg(m_gameId, k);
}

QVariant LegacySettingsFacade::value(const QString &key) const { return value(key, QVariant{}); }
QVariant LegacySettingsFacade::value(const QString &key, const QVariant &fallback) const
{
    if (!m_settings)
        return fallback;
    const QString mapped = storageKey(key);
    if (mapped.isEmpty())
        return fallback;
    if (m_settings->contains(mapped))
        return m_settings->value(mapped, fallback);
    const QString raw = key.trimmed();
    if (!isGlobalPreference(raw) && !rawLegacyKeyAllowed(raw) && safeKey(raw) && !isReservedHostKey(raw) && m_settings->contains(raw)) {
        const QVariant legacy = m_settings->value(raw, fallback);
        // Copy-on-read migration: preserve the old value, but all future writes are game-scoped.
        m_settings->setValue(mapped, legacy);
        return legacy;
    }
    return fallback;
}

void LegacySettingsFacade::setValue(const QString &key, const QVariant &value)
{
    if (!m_settings)
        return;
    const QString mapped = storageKey(key);
    if (!mapped.isEmpty())
        m_settings->setValue(mapped, value);
}

bool LegacySettingsFacade::contains(const QString &key) const
{
    if (!m_settings)
        return false;
    const QString mapped = storageKey(key);
    if (mapped.isEmpty()) return false;
    if (m_settings->contains(mapped)) return true;
    const QString raw=key.trimmed();
    return !isGlobalPreference(raw) && !rawLegacyKeyAllowed(raw) && safeKey(raw) && !isReservedHostKey(raw) && m_settings->contains(raw);
}

void LegacySettingsFacade::remove(const QString &key)
{
    if (!m_settings)
        return;
    const QString mapped = storageKey(key);
    if (mapped.isEmpty())
        return;
    m_settings->remove(mapped);
    const QString raw = key.trimmed();
    if (!isGlobalPreference(raw) && !rawLegacyKeyAllowed(raw) && safeKey(raw) && !isReservedHostKey(raw))
        m_settings->remove(raw);
}

void LegacySettingsFacade::setSoundEnabled(bool enabled) { if (m_settings) m_settings->setSoundEnabled(enabled); }
void LegacySettingsFacade::setSoundVolume(qreal volume) { if (m_settings) m_settings->setSoundVolume(qBound<qreal>(0.0, volume, 1.0)); }
void LegacySettingsFacade::setAnimationsEnabled(bool enabled) { if (m_settings) m_settings->setAnimationsEnabled(enabled); }
