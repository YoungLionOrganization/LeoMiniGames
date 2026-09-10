// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QString>
#include <QVariant>
class SettingsManager;
class LegacySettingsFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool soundEnabled READ soundEnabled WRITE setSoundEnabled NOTIFY soundEnabledChanged)
    Q_PROPERTY(qreal soundVolume READ soundVolume WRITE setSoundVolume NOTIFY soundVolumeChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled NOTIFY animationsEnabledChanged)
public:
    explicit LegacySettingsFacade(SettingsManager *settings, QObject *parent = nullptr);
    bool soundEnabled() const;
    qreal soundVolume() const;
    bool animationsEnabled() const;
    void activate(const QString &gameId);
    Q_INVOKABLE QVariant value(const QString &key) const;
    Q_INVOKABLE QVariant value(const QString &key, const QVariant &fallback) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE bool contains(const QString &key) const;
    Q_INVOKABLE void remove(const QString &key);
public slots:
    void setSoundEnabled(bool enabled);
    void setSoundVolume(qreal volume);
    void setAnimationsEnabled(bool enabled);
signals:
    void soundEnabledChanged();
    void soundVolumeChanged();
    void animationsEnabledChanged();
private:
    QString storageKey(const QString &key) const;
    bool rawLegacyKeyAllowed(const QString &key) const;
    static bool safeKey(const QString &key);
    SettingsManager *m_settings = nullptr;
    QString m_gameId;
};
