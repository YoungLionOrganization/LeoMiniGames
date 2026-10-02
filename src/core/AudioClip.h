// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QUrl>
#include <memory>

// Host-owned playback. Package developers never select a platform backend.
class AudioClip final : public QObject
{
    Q_OBJECT
public:
    enum class Usage { Effect, Music };
    AudioClip(const QUrl &source, Usage usage, QObject *parent = nullptr);
    ~AudioClip() override;
    void load();
    void play(bool loop = false);
    void stop();
    void pause();
    void resume();
    void setVolume(qreal volume);
    qreal volume() const;
    QUrl playbackUrl() const;
    bool usesMediaPlayer() const;
    bool pendingPlay() const;
    bool paused() const;
signals:
    void errorOccurred(const QString &message, const QString &source);
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    void useMediaPlayer();
    void startIfReady();
};
