// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "AudioClip.h"
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QDir>
#include <cmath>
#ifdef LMG_HAS_MULTIMEDIA
#include <QSoundEffect>
#include <QAudioOutput>
#include <QMediaPlayer>
#endif

struct AudioClip::Impl {
    QUrl source, playback;
    Usage usage;
    QTemporaryDir files;
    qreal volume = 1.0;
    bool loaded = false, pending = false, paused = false, resume = false, loop = false;
#ifdef LMG_HAS_MULTIMEDIA
    QSoundEffect *wave = nullptr;
    QAudioOutput *output = nullptr;
    QMediaPlayer *media = nullptr;
#endif
};
AudioClip::AudioClip(const QUrl &source, Usage usage, QObject *parent)
    : QObject(parent), m_impl(std::make_unique<Impl>())
{
    m_impl->source = source;
    m_impl->usage = usage;
}
AudioClip::~AudioClip()
{
#ifdef LMG_HAS_MULTIMEDIA
    // Release decoder file handles before the temporary resource directory.
    delete m_impl->wave;
    // A fallback wave may still be queued for deleteLater in this event turn.
    qDeleteAll(findChildren<QSoundEffect *>(QString(), Qt::FindDirectChildrenOnly));
    delete m_impl->media;
    delete m_impl->output;
#endif
}
void AudioClip::load()
{
    if (m_impl->loaded) return;
    m_impl->loaded = true;
#ifdef LMG_HAS_MULTIMEDIA
    const QUrl source = m_impl->source;
    if (source.scheme() == QStringLiteral("qrc")) {
        // Native/FFmpeg backends do not all support RCC streams. Keep a local
        // file alive until the decoder/player is destroyed, including on Android.
        QFile input(QStringLiteral(":") + source.path());
        const qint64 limit = m_impl->usage == Usage::Effect ? 32LL*1024*1024 : 128LL*1024*1024;
        if (!m_impl->files.isValid() || !input.open(QIODevice::ReadOnly) || input.size() <= 0 || input.size() > limit) {
            emit errorOccurred(QStringLiteral("Audio resource is missing, too large, or cannot be read."), source.toString());
            return;
        }
        const QString suffix = QFileInfo(source.path()).suffix().toLower();
        const QString path = m_impl->files.filePath(QStringLiteral("clip.") + suffix);
        QFile output(path);
        if (!output.open(QIODevice::WriteOnly)) {
            emit errorOccurred(QStringLiteral("Cannot prepare a local audio resource."), source.toString());
            return;
        }
        while (!input.atEnd()) {
            const QByteArray bytes = input.read(64*1024);
            if (bytes.isEmpty() || output.write(bytes) != bytes.size()) {
                output.close(); output.remove();
                emit errorOccurred(QStringLiteral("Cannot copy an audio resource."), source.toString());
                return;
            }
        }
        output.close();
        m_impl->playback = QUrl::fromLocalFile(path);
    } else if (source.isLocalFile() && QFileInfo(source.toLocalFile()).isFile()) {
        m_impl->playback = source;
    } else {
        emit errorOccurred(QStringLiteral("Audio source must be an existing local qrc:/ or file: URL."), source.toString());
        return;
    }
    if (m_impl->usage == Usage::Music || QFileInfo(m_impl->playback.path()).suffix().compare(QStringLiteral("wav"), Qt::CaseInsensitive) != 0) {
        useMediaPlayer();
        return;
    }
    m_impl->wave = new QSoundEffect(this);
    m_impl->wave->setLoopCount(1);
    m_impl->wave->setVolume(m_impl->volume);
    connect(m_impl->wave, &QSoundEffect::statusChanged, this, [this] {
        if (!m_impl->wave) return;
        if (m_impl->wave->status() == QSoundEffect::Error) {
            // A WAV encoding unsupported by QSoundEffect may still be decoded
            // by the host's Qt Multimedia backend. Never require game patches.
            m_impl->wave->stop();
            m_impl->wave->deleteLater();
            m_impl->wave = nullptr;
            useMediaPlayer();
        } else startIfReady();
    });
    m_impl->wave->setSource(m_impl->playback);
#else
    emit errorOccurred(QStringLiteral("This build has no Qt Multimedia audio support."), m_impl->source.toString());
#endif
}
void AudioClip::useMediaPlayer()
{
#ifdef LMG_HAS_MULTIMEDIA
    if (m_impl->media || m_impl->playback.isEmpty()) return;
    m_impl->output = new QAudioOutput(this);
    m_impl->output->setVolume(m_impl->volume);
    m_impl->media = new QMediaPlayer(this);
    m_impl->media->setAudioOutput(m_impl->output);
    connect(m_impl->media, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &message) {
        m_impl->pending = false;
        m_impl->resume = false;
        emit errorOccurred(QStringLiteral("Audio decoder/backend failed: %1").arg(message), m_impl->source.toString());
    });
    connect(m_impl->media, &QMediaPlayer::mediaStatusChanged, this, [this] { startIfReady(); });
    m_impl->media->setSource(m_impl->playback);
    startIfReady();
#endif
}
void AudioClip::startIfReady()
{
#ifdef LMG_HAS_MULTIMEDIA
    if (!m_impl->pending || m_impl->paused) return;
    if (m_impl->wave && m_impl->wave->status() == QSoundEffect::Ready) {
        m_impl->pending = false;
        m_impl->wave->play();
    } else if (m_impl->media && (m_impl->media->mediaStatus() == QMediaPlayer::LoadedMedia || m_impl->media->mediaStatus() == QMediaPlayer::BufferedMedia || m_impl->media->mediaStatus() == QMediaPlayer::EndOfMedia)) {
        m_impl->pending = false;
        m_impl->media->setLoops(m_impl->loop ? QMediaPlayer::Infinite : 1);
        m_impl->media->play();
    }
#endif
}
void AudioClip::play(bool loop)
{
    if (m_impl->usage == Usage::Effect && (m_impl->paused || m_impl->volume <= 0.0)) return;
    stop();
    m_impl->loop = loop;
    m_impl->pending = true;
    if (m_impl->paused && m_impl->usage == Usage::Music) m_impl->resume = true;
    load();
    if (m_impl->playback.isEmpty()) m_impl->pending = false;
    startIfReady();
}
void AudioClip::stop()
{
    m_impl->pending = false;
    m_impl->resume = false;
#ifdef LMG_HAS_MULTIMEDIA
    if (m_impl->wave) m_impl->wave->stop();
    if (m_impl->media) { m_impl->media->stop(); m_impl->media->setPosition(0); }
#endif
}
void AudioClip::pause()
{
    if (m_impl->paused) return;
    m_impl->paused = true;
    if (m_impl->usage == Usage::Effect) { stop(); return; }
#ifdef LMG_HAS_MULTIMEDIA
    m_impl->resume = m_impl->pending || (m_impl->media && m_impl->media->playbackState() == QMediaPlayer::PlayingState);
    if (m_impl->media) m_impl->media->pause();
#endif
}
void AudioClip::resume()
{
    if (!m_impl->paused) return;
    m_impl->paused = false;
    if (!m_impl->resume) return;
    m_impl->resume = false;
    m_impl->pending = true;
    startIfReady();
}
void AudioClip::setVolume(qreal volume)
{
    m_impl->volume = std::isfinite(volume) ? qBound<qreal>(0.0, volume, 1.0) : 0.0;
#ifdef LMG_HAS_MULTIMEDIA
    if (m_impl->wave) m_impl->wave->setVolume(m_impl->volume);
    if (m_impl->output) m_impl->output->setVolume(m_impl->volume);
#endif
    if (m_impl->usage == Usage::Effect && m_impl->volume <= 0.0) stop();
}
qreal AudioClip::volume() const { return m_impl->volume; }
QUrl AudioClip::playbackUrl() const { return m_impl->playback; }
bool AudioClip::pendingPlay() const { return m_impl->pending; }
bool AudioClip::paused() const { return m_impl->paused; }
bool AudioClip::usesMediaPlayer() const {
#ifdef LMG_HAS_MULTIMEDIA
    return m_impl->media != nullptr;
#else
    return false;
#endif
}
