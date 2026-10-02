// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "SettingsManager.h"

#include <QtGlobal>
#include <cmath>

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
}

bool SettingsManager::soundEnabled() const
{
    return m_settings.value(QStringLiteral("audio/enabled"), true).toBool();
}

qreal SettingsManager::soundVolume() const
{
    const qreal volume = m_settings.value(QStringLiteral("audio/volume"), 0.72).toReal();
    return std::isfinite(volume) ? qBound<qreal>(0.0, volume, 1.0) : 0.0;
}

bool SettingsManager::animationsEnabled() const
{
    return m_settings.value(QStringLiteral("ui/animations"), true).toBool();
}

QVariant SettingsManager::value(const QString &key) const
{
    return value(key, QVariant{});
}

QVariant SettingsManager::value(const QString &key, const QVariant &fallback) const
{
    return m_settings.value(key, fallback);
}

void SettingsManager::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

bool SettingsManager::contains(const QString &key) const
{
    return m_settings.contains(key);
}

void SettingsManager::remove(const QString &key)
{
    m_settings.remove(key);
}

void SettingsManager::setSoundEnabled(bool enabled)
{
    if (enabled == soundEnabled())
        return;
    m_settings.setValue(QStringLiteral("audio/enabled"), enabled);
    emit soundEnabledChanged();
}

void SettingsManager::setSoundVolume(qreal volume)
{
    volume = std::isfinite(volume) ? qBound<qreal>(0.0, volume, 1.0) : 0.0;
    if (qFuzzyCompare(volume, soundVolume()))
        return;
    m_settings.setValue(QStringLiteral("audio/volume"), volume);
    emit soundVolumeChanged();
}

void SettingsManager::setAnimationsEnabled(bool enabled)
{
    if (enabled == animationsEnabled())
        return;
    m_settings.setValue(QStringLiteral("ui/animations"), enabled);
    emit animationsEnabledChanged();
}
