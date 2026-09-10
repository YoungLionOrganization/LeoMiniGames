// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
class AudioManager;
class LegacyAudioFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
public:
    explicit LegacyAudioFacade(AudioManager *audio, QObject *parent = nullptr);
    bool available() const;
    bool ready() const { return true; }
    QString version() const { return QStringLiteral("0.7"); }
    void activate(const QString &gameId);
    Q_INVOKABLE QStringList capabilities() const;
    Q_INVOKABLE void play(const QString &name);
    Q_INVOKABLE void playUrl(const QUrl &url);
    Q_INVOKABLE void playUrl(const QUrl &url, qreal gain);
    Q_INVOKABLE void preload(const QUrl &url);
    Q_INVOKABLE void preload(const QUrl &url, qreal gain);
    Q_INVOKABLE void stopAll();
private:
    QUrl normalized(const QUrl &url) const;
    AudioManager *m_audio = nullptr;
    QString m_gameId;
};
