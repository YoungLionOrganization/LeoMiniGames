// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QString>
#include <QUrl>
#include <memory>
class SettingsManager;
class AudioManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
public:
    explicit AudioManager(SettingsManager *settings, QObject *parent = nullptr);
    ~AudioManager() override;
    bool available() const;
    bool ready() const { return true; }
    QString version() const { return QStringLiteral("0.7"); }
    Q_INVOKABLE QStringList capabilities() const;
    Q_INVOKABLE void play(const QString &name);
    Q_INVOKABLE void playUrl(const QUrl &url);
    Q_INVOKABLE void playUrl(const QUrl &url, qreal gain);
    Q_INVOKABLE void preload(const QUrl &url);
    Q_INVOKABLE void releasePrefix(const QString &urlPrefix);
    Q_INVOKABLE void stopAll();
signals:
    void audioError(const QString &message, const QString &source);
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    SettingsManager *m_settings = nullptr;
    void updateVolume();
    qreal effectiveVolume(qreal gain = 1.0) const;
    bool localUrlAllowed(const QUrl &url) const;
};
