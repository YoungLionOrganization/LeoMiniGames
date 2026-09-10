// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QUrl>
#include <memory>
class AudioManager; class SettingsManager;
class GameAudio final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
public:
    explicit GameAudio(AudioManager *legacy, SettingsManager *settings, QObject *parent=nullptr);
    ~GameAudio() override;
    bool ready() const { return true; }
    bool available() const;
    QString version() const { return QStringLiteral("0.7"); }
    void activate(const QString &gameId, bool restrictedExternal);
    Q_INVOKABLE QStringList capabilities() const;
    Q_INVOKABLE void playEffect(const QUrl &url);
    Q_INVOKABLE void playEffect(const QUrl &url,qreal gain);
    Q_INVOKABLE void playEffect(const QUrl &url,qreal gain,const QString &group);
    Q_INVOKABLE void preload(const QUrl &url);
    Q_INVOKABLE void preload(const QUrl &url,qreal legacyGain);
    Q_INVOKABLE void playMusic(const QUrl &url);
    Q_INVOKABLE void playMusic(const QUrl &url,bool loop);
    Q_INVOKABLE void playMusic(const QUrl &url,bool loop,const QString &group);
    Q_INVOKABLE void stopMusic();
    Q_INVOKABLE void pauseAll();
    Q_INVOKABLE void resumeAll();
    Q_INVOKABLE qreal volume(const QString &group) const;
    Q_INVOKABLE void setVolume(const QString &group,qreal value);
private:
    struct Impl; std::unique_ptr<Impl> m_impl; AudioManager *m_legacy=nullptr; SettingsManager *m_settings=nullptr;
    QString m_gameId; bool m_restrictedExternal=false;
    QString normalizedGroup(const QString &group) const; qreal groupVolume(const QString &group) const; void updateMusicVolume();
    QUrl permittedUrl(const QUrl &url) const;
};
