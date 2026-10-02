// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "AudioManager.h"
#include "SettingsManager.h"
#include "AudioClip.h"
#include <QHash>
#include <QQueue>
#include <QtGlobal>
#include <cmath>
#include <utility>

namespace { constexpr int kMaxDynamicEffects = 32; }
struct AudioManager::Impl
{
    struct Effect { AudioClip *clip; qreal gain = 1.0; QString group = QStringLiteral("sfx"); };
    QHash<QString, QUrl> builtins;
    QHash<QString, Effect> effects;
    QQueue<QString> order;
    bool paused = false;
};

AudioManager::AudioManager(SettingsManager *settings, QObject *parent)
    : QObject(parent), m_impl(std::make_unique<Impl>()), m_settings(settings)
{
    const auto add = [this](const char *name, const char *resource) { m_impl->builtins.insert(QString::fromLatin1(name), QUrl(QString::fromLatin1(resource))); };
    add("click","qrc:/sfx/click.wav"); add("move","qrc:/sfx/move.wav"); add("card","qrc:/sfx/card.wav");
    add("flag","qrc:/sfx/flag.wav"); add("flip","qrc:/sfx/flip.wav"); add("win","qrc:/sfx/win.wav");
    add("lose","qrc:/sfx/lose.wav"); add("success","qrc:/sfx/success.wav"); add("error","qrc:/sfx/error.wav");
    add("deal","qrc:/sfx/deal.wav"); add("shuffle","qrc:/sfx/shuffle.wav"); add("tap","qrc:/sfx/tap.wav");
    add("match","qrc:/sfx/match.wav"); add("miss","qrc:/sfx/miss.wav"); add("merge","qrc:/sfx/merge.wav");
    add("ready","qrc:/sfx/ready.wav"); add("download","qrc:/sfx/download.wav"); add("install","qrc:/sfx/install.wav");
    add("uninstall","qrc:/sfx/uninstall.wav");
    if (m_settings) {
        connect(m_settings, &SettingsManager::soundVolumeChanged, this, &AudioManager::updateVolume);
        connect(m_settings, &SettingsManager::soundEnabledChanged, this, &AudioManager::updateVolume);
    }
}
AudioManager::~AudioManager() = default;
bool AudioManager::available() const {
#ifdef LMG_HAS_MULTIMEDIA
    return true;
#else
    return false;
#endif
}
QStringList AudioManager::capabilities() const
{
    if (!available()) return QStringList{};
    return {QStringLiteral("named_effects"), QStringLiteral("local_rcc_audio"),
        QStringLiteral("volume"), QStringLiteral("preload"), QStringLiteral("media_effect_fallback")};
}
qreal AudioManager::effectiveVolume(qreal gain, const QString &group) const
{
    if (!std::isfinite(gain) || (m_settings && !m_settings->soundEnabled())) return 0.0;
    const qreal master = m_settings ? m_settings->soundVolume() : 1.0;
    const qreal groupGain = m_settings ? m_settings->value(QStringLiteral("audio/groups/") + group, 1.0).toReal() : 1.0;
    if (!std::isfinite(groupGain)) return 0.0;
    return qBound<qreal>(0.0, master * qBound<qreal>(0.0, gain, 2.0) * qBound<qreal>(0.0, groupGain, 1.0), 1.0);
}
bool AudioManager::localUrlAllowed(const QUrl &url) const
{
    return url.isValid() && !url.isEmpty() && url.host().isEmpty()
        && !url.hasQuery() && !url.hasFragment()
        && (url.scheme() == QStringLiteral("qrc") || url.isLocalFile());
}
void AudioManager::play(const QString &name) { play(name, 1.0, QStringLiteral("sfx")); }
void AudioManager::play(const QString &name, qreal gain, const QString &group)
{
    const QUrl source = m_impl->builtins.value(name.trimmed().toLower());
    if (source.isEmpty()) { emit audioError(QStringLiteral("Unknown named sound effect."), name); return; }
    playUrl(source, gain, group);
}
void AudioManager::preload(const QUrl &url)
{
    if (url.scheme().isEmpty() && !url.toString().contains(QLatin1Char('/'))) {
        const QUrl named = m_impl->builtins.value(url.toString().trimmed().toLower());
        if (named.isEmpty()) { emit audioError(QStringLiteral("Unknown named sound effect."), url.toString()); return; }
        preload(named); return;
    }
    if (!available()) { emit audioError(QStringLiteral("This build has no Qt Multimedia audio support."), url.toString()); return; }
    if (!localUrlAllowed(url)) { emit audioError(QStringLiteral("Audio source must be a local qrc:/ or file: URL."), url.toString()); return; }
    const QUrl source = url.adjusted(QUrl::NormalizePathSegments);
    const QString key = source.toString(QUrl::FullyEncoded);
    if (m_impl->effects.contains(key)) return;
    while (m_impl->order.size() >= kMaxDynamicEffects) {
        const QString old = m_impl->order.dequeue();
        delete m_impl->effects.take(old).clip;
    }
    auto *clip = new AudioClip(source, AudioClip::Usage::Effect, this);
    connect(clip, &AudioClip::errorOccurred, this, &AudioManager::audioError);
    m_impl->effects.insert(key, {clip, 1.0, QStringLiteral("sfx")});
    m_impl->order.enqueue(key);
    clip->setVolume(effectiveVolume());
    if (m_impl->paused) clip->pause();
    clip->load();
}
void AudioManager::playUrl(const QUrl &url) { playUrl(url, 1.0); }
void AudioManager::playUrl(const QUrl &url, qreal gain) { playUrl(url, gain, QStringLiteral("sfx")); }
void AudioManager::playUrl(const QUrl &url, qreal gain, const QString &group)
{
    if (url.scheme().isEmpty() && !url.toString().contains(QLatin1Char('/'))) { play(url.toString(), gain, group); return; }
    if (m_impl->paused || effectiveVolume(gain, group) <= 0.0) return;
    preload(url);
    const QString key = url.adjusted(QUrl::NormalizePathSegments).toString(QUrl::FullyEncoded);
    auto it = m_impl->effects.find(key);
    if (it == m_impl->effects.end()) return;
    it->gain = gain; it->group = group;
    m_impl->order.removeAll(key); m_impl->order.enqueue(key);
    it->clip->setVolume(effectiveVolume(gain, group));
    it->clip->play();
}
void AudioManager::releasePrefix(const QString &urlPrefix)
{
    const auto keys = m_impl->effects.keys();
    for (const QString &key : keys) if (key.startsWith(urlPrefix)) {
        delete m_impl->effects.take(key).clip;
        m_impl->order.removeAll(key);
    }
}
void AudioManager::stopAll()
{
    for (const auto &effect : std::as_const(m_impl->effects)) effect.clip->stop();
}
void AudioManager::pauseAll()
{
    m_impl->paused = true;
    for (const auto &effect : std::as_const(m_impl->effects)) effect.clip->pause();
}
void AudioManager::resumeAll()
{
    m_impl->paused = false;
    for (const auto &effect : std::as_const(m_impl->effects)) effect.clip->resume();
}
void AudioManager::updateVolume()
{
    for (const auto &effect : std::as_const(m_impl->effects))
        effect.clip->setVolume(effectiveVolume(effect.gain, effect.group));
}
void AudioManager::refreshVolumes() { updateVolume(); }
