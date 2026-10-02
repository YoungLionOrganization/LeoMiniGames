// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameAudio.h"
#include "AudioManager.h"
#include "SettingsManager.h"
#include <QDir>
#include <QFileInfo>
#include <QtGlobal>
#include "AudioClip.h"
#include <cmath>
struct GameAudio::Impl { AudioClip *music=nullptr; bool paused=false; QString musicGroup=QStringLiteral("music"); };
GameAudio::GameAudio(AudioManager *legacy,SettingsManager *settings,QObject *parent):QObject(parent),m_impl(std::make_unique<Impl>()),m_legacy(legacy),m_settings(settings)
{
    if (m_legacy) connect(m_legacy, &AudioManager::audioError, this, &GameAudio::audioError);
    if(m_settings){connect(m_settings,&SettingsManager::soundVolumeChanged,this,&GameAudio::updateMusicVolume);connect(m_settings,&SettingsManager::soundEnabledChanged,this,&GameAudio::updateMusicVolume);}
}
GameAudio::~GameAudio() { stopMusic(); }
void GameAudio::activate(const QString &gameId,bool restrictedExternal){
    if(m_legacy) {
        m_legacy->stopAll();
        if(!m_gameId.isEmpty()) m_legacy->releasePrefix(QStringLiteral("qrc:/mods/%1/").arg(m_gameId));
        m_legacy->resumeAll();
    }
    stopMusic(); m_impl->paused=false; m_gameId=gameId.trimmed(); m_restrictedExternal=restrictedExternal;
}
bool GameAudio::available() const { return m_legacy && m_legacy->available(); }
QStringList GameAudio::capabilities() const {
    auto result = m_legacy ? m_legacy->capabilities() : QStringList();
    if (available()) result << QStringLiteral("music") << QStringLiteral("groups") << QStringLiteral("pause_resume") << QStringLiteral("audio_errors");
    return result;
}
QString GameAudio::normalizedGroup(const QString &group) const{const QString value=group.trimmed().toLower();return value==QStringLiteral("music")||value==QStringLiteral("ui")||value==QStringLiteral("ambient")?value:QStringLiteral("sfx");}
qreal GameAudio::groupVolume(const QString &group) const{if(!m_settings)return 1.0;const qreal value=m_settings->value(QStringLiteral("audio/groups/%1").arg(normalizedGroup(group)),1.0).toReal();return std::isfinite(value)?qBound<qreal>(0.0,value,1.0):0.0;}
qreal GameAudio::volume(const QString &group) const{return groupVolume(group);}
void GameAudio::setVolume(const QString &group,qreal value){if(!m_settings||group.isEmpty())return;m_settings->setValue(QStringLiteral("audio/groups/%1").arg(normalizedGroup(group)),(std::isfinite(value)?qBound<qreal>(0.0,value,1.0):0.0));if(m_legacy)m_legacy->refreshVolumes();updateMusicVolume();}
QUrl GameAudio::permittedUrl(const QUrl &url) const{
    if(!url.isValid()||url.isEmpty()) return QUrl();
    if(!m_restrictedExternal) return url;
    // Named launcher sounds remain shared by modern and legacy packages.
    if(url.scheme().isEmpty() && !url.toString().contains(QLatin1Char('/')) && !url.toString().contains(QLatin1Char('.'))) return url;
    if(m_gameId.isEmpty()) return QUrl();
    if(!url.host().isEmpty() || url.hasQuery() || url.hasFragment()) return QUrl();
    const QString expected=QStringLiteral("/mods/%1/").arg(m_gameId);
    const QString value=url.path(QUrl::FullyDecoded);
    if(url.scheme().compare(QStringLiteral("qrc"),Qt::CaseInsensitive)==0 && value.startsWith(expected) && !value.contains(QStringLiteral(".."))) return url;
    if(url.scheme().isEmpty()){
        const QString rel=value;
        if(rel.isEmpty()||rel.startsWith(QLatin1Char('/'))||rel.contains(QStringLiteral(".."))||rel.contains(QLatin1Char('\\'))||QDir::cleanPath(rel)!=rel) return QUrl();
        const QString resource=QStringLiteral(":/mods/%1/%2").arg(m_gameId,rel);
        if(QFileInfo::exists(resource)) { QUrl resolved; resolved.setScheme(QStringLiteral("qrc")); resolved.setPath(QStringLiteral("/mods/%1/%2").arg(m_gameId,rel)); return resolved; }
    }
    return QUrl();
}
void GameAudio::playEffect(const QUrl &url){playEffect(url,1.0,QStringLiteral("sfx"));}
void GameAudio::playEffect(const QUrl &url,qreal gain){playEffect(url,gain,QStringLiteral("sfx"));}
void GameAudio::playEffect(const QUrl &url,qreal gain,const QString &group){const QUrl safe=permittedUrl(url);if(m_legacy&&!safe.isEmpty()&&!m_impl->paused)m_legacy->playUrl(safe,gain,normalizedGroup(group));else if(safe.isEmpty())emit audioError(QStringLiteral("Audio source is outside the active game resources."),url.toString());}
void GameAudio::preload(const QUrl &url){const QUrl safe=permittedUrl(url);if(m_legacy&&!safe.isEmpty())m_legacy->preload(safe);else if(safe.isEmpty())emit audioError(QStringLiteral("Audio source is outside the active game resources."),url.toString());}
void GameAudio::preload(const QUrl &url,qreal legacyGain){Q_UNUSED(legacyGain) preload(url);}
void GameAudio::playMusic(const QUrl &url){playMusic(url,true,QStringLiteral("music"));}
void GameAudio::playMusic(const QUrl &url,bool loop){playMusic(url,loop,QStringLiteral("music"));}
void GameAudio::playMusic(const QUrl &url,bool loop,const QString &group)
{
    const QUrl safe=permittedUrl(url);
    if(safe.isEmpty() || !safe.host().isEmpty() || safe.hasQuery() || safe.hasFragment() || (safe.scheme()!=QStringLiteral("qrc") && !safe.isLocalFile())) {
        emit audioError(QStringLiteral("Music source must be a local resource belonging to the active game."),url.toString()); return;
    }
    if(!available()) { emit audioError(QStringLiteral("This build has no Qt Multimedia audio support."),url.toString()); return; }
    stopMusic();
    m_impl->musicGroup=normalizedGroup(group);
    m_impl->music=new AudioClip(safe,AudioClip::Usage::Music,this);
    connect(m_impl->music,&AudioClip::errorOccurred,this,&GameAudio::audioError);
    updateMusicVolume();
    if(m_impl->paused) m_impl->music->pause();
    m_impl->music->play(loop);
}
void GameAudio::stopMusic(){ delete m_impl->music; m_impl->music=nullptr; }
void GameAudio::pauseAll(){
    if(m_impl->paused) return;
    m_impl->paused=true;
    if(m_legacy)m_legacy->pauseAll();
    if(m_impl->music)m_impl->music->pause();
}
void GameAudio::resumeAll(){
    if(!m_impl->paused) return;
    m_impl->paused=false;
    if(m_legacy)m_legacy->resumeAll();
    if(m_impl->music)m_impl->music->resume();
}
void GameAudio::updateMusicVolume(){
    if(!m_impl->music)return;
    const qreal master=m_settings?(m_settings->soundEnabled()?m_settings->soundVolume():0.0):1.0;
    m_impl->music->setVolume(master*groupVolume(m_impl->musicGroup));
}
