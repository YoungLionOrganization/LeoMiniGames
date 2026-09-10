// SPDX-License-Identifier: GPL-3.0-or-later
#include "AudioManager.h"
#include "SettingsManager.h"
#include <QHash>
#include <QQueue>
#include <QtGlobal>
#include <utility>
#ifdef LMG_HAS_MULTIMEDIA
#include <QSoundEffect>
#endif

namespace { constexpr int kMaxDynamicEffects = 32; }
struct AudioManager::Impl
{
    QHash<QString, QUrl> builtins;
#ifdef LMG_HAS_MULTIMEDIA
    QHash<QString, QSoundEffect *> effects;
    QHash<QString, QSoundEffect *> dynamicEffects;
    QQueue<QString> dynamicOrder;
#endif
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
    QStringList result{QStringLiteral("named_effects"), QStringLiteral("local_rcc_audio"), QStringLiteral("volume")};
#ifdef LMG_HAS_MULTIMEDIA
    result << QStringLiteral("preload");
#endif
    return result;
}
qreal AudioManager::effectiveVolume(qreal gain) const { if (!m_settings || !m_settings->soundEnabled()) return 0.0; return qBound<qreal>(0.0, m_settings->soundVolume()*qBound<qreal>(0.0,gain,2.0),1.0); }
bool AudioManager::localUrlAllowed(const QUrl &url) const
{
    if (!url.isValid() || url.isEmpty()) return false;
    const QString scheme=url.scheme().toLower();
    return scheme==QStringLiteral("qrc") || scheme==QStringLiteral("file");
}
void AudioManager::play(const QString &name)
{
#ifdef LMG_HAS_MULTIMEDIA
    const QUrl source=m_impl->builtins.value(name.trimmed().toLower());
    if (source.isEmpty()) { emit audioError(QStringLiteral("Unknown named sound effect."), name); return; }
    QSoundEffect *effect=m_impl->effects.value(name,nullptr);
    if (!effect) {
        effect=new QSoundEffect(this); effect->setLoopCount(1); effect->setSource(source);
        connect(effect,&QSoundEffect::statusChanged,this,[this,effect,source]{ if(effect->status()==QSoundEffect::Error) emit audioError(QStringLiteral("Could not decode sound effect."),source.toString()); });
        m_impl->effects.insert(name,effect);
    }
    effect->setVolume(effectiveVolume()); if(effect->isPlaying()) effect->stop(); effect->play();
#else
    Q_UNUSED(name)
#endif
}
void AudioManager::preload(const QUrl &url)
{
#ifdef LMG_HAS_MULTIMEDIA
    if (!localUrlAllowed(url)) { emit audioError(QStringLiteral("Audio source must be a local qrc:/ or file: URL."),url.toString()); return; }
    const QString key=url.toString(QUrl::FullyEncoded); if(m_impl->dynamicEffects.contains(key)) return;
    while(m_impl->dynamicOrder.size()>=kMaxDynamicEffects) { const QString old=m_impl->dynamicOrder.dequeue(); if(auto *e=m_impl->dynamicEffects.take(old)) e->deleteLater(); }
    auto *effect=new QSoundEffect(this); effect->setLoopCount(1); effect->setVolume(effectiveVolume()); effect->setSource(url);
    connect(effect,&QSoundEffect::statusChanged,this,[this,effect,url]{ if(effect->status()==QSoundEffect::Error) emit audioError(QStringLiteral("Could not decode local sound effect."),url.toString()); });
    m_impl->dynamicEffects.insert(key,effect); m_impl->dynamicOrder.enqueue(key);
#else
    Q_UNUSED(url)
#endif
}
void AudioManager::playUrl(const QUrl &url) { playUrl(url,1.0); }
void AudioManager::playUrl(const QUrl &url,qreal gain)
{
    if (url.scheme().isEmpty() && !url.toString().contains(QLatin1Char('/'))) { play(url.toString()); return; }
#ifdef LMG_HAS_MULTIMEDIA
    if (!localUrlAllowed(url) || effectiveVolume(gain)<=0.0) { if(!localUrlAllowed(url)) emit audioError(QStringLiteral("Audio source must be local."),url.toString()); return; }
    preload(url); const QString key=url.toString(QUrl::FullyEncoded); auto *effect=m_impl->dynamicEffects.value(key,nullptr); if(!effect) return;
    effect->setVolume(effectiveVolume(gain)); if(effect->isPlaying()) effect->stop(); effect->play();
#else
    Q_UNUSED(gain)
#endif
}
void AudioManager::releasePrefix(const QString &urlPrefix)
{
#ifdef LMG_HAS_MULTIMEDIA
    const auto keys=m_impl->dynamicEffects.keys();
    for(const QString &key:keys) if(key.startsWith(urlPrefix)) { if(auto *e=m_impl->dynamicEffects.take(key)) e->deleteLater(); m_impl->dynamicOrder.removeAll(key); }
#else
    Q_UNUSED(urlPrefix)
#endif
}
void AudioManager::stopAll()
{
#ifdef LMG_HAS_MULTIMEDIA
    for(auto *e:std::as_const(m_impl->effects)) e->stop(); for(auto *e:std::as_const(m_impl->dynamicEffects)) e->stop();
#endif
}
void AudioManager::updateVolume()
{
#ifdef LMG_HAS_MULTIMEDIA
    const qreal volume=effectiveVolume(); for(auto *e:std::as_const(m_impl->effects)) e->setVolume(volume); for(auto *e:std::as_const(m_impl->dynamicEffects)) e->setVolume(volume);
#endif
}
