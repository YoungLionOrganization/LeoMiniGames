// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameAudio.h"
#include "AudioManager.h"
#include "SettingsManager.h"
#include <QDir>
#include <QFileInfo>
#include <QtGlobal>
#ifdef LMG_HAS_MULTIMEDIA
#include <QAudioOutput>
#include <QMediaPlayer>
struct GameAudio::Impl { QAudioOutput output; QMediaPlayer player; bool resumeMusic=false; QString musicGroup=QStringLiteral("music"); };
#else
struct GameAudio::Impl { bool resumeMusic=false; QString musicGroup=QStringLiteral("music"); };
#endif
GameAudio::GameAudio(AudioManager *legacy,SettingsManager *settings,QObject *parent):QObject(parent),m_impl(std::make_unique<Impl>()),m_legacy(legacy),m_settings(settings)
{
#ifdef LMG_HAS_MULTIMEDIA
    m_impl->player.setAudioOutput(&m_impl->output);
#endif
    if(m_settings){connect(m_settings,&SettingsManager::soundVolumeChanged,this,&GameAudio::updateMusicVolume);connect(m_settings,&SettingsManager::soundEnabledChanged,this,&GameAudio::updateMusicVolume);} updateMusicVolume();
}
GameAudio::~GameAudio()=default;
void GameAudio::activate(const QString &gameId,bool restrictedExternal){
    if(m_legacy && !m_gameId.isEmpty() && m_gameId!=gameId) m_legacy->releasePrefix(QStringLiteral("qrc:/mods/%1/").arg(m_gameId));
    stopMusic(); m_gameId=gameId.trimmed(); m_restrictedExternal=restrictedExternal;
}
bool GameAudio::available() const { return m_legacy && m_legacy->available(); }
QStringList GameAudio::capabilities() const { return m_legacy ? m_legacy->capabilities() : QStringList(); }
QString GameAudio::normalizedGroup(const QString &group) const{const QString value=group.trimmed().toLower();return value==QStringLiteral("music")||value==QStringLiteral("ui")||value==QStringLiteral("ambient")?value:QStringLiteral("sfx");}
qreal GameAudio::groupVolume(const QString &group) const{if(!m_settings)return 1.0;return qBound<qreal>(0.0,m_settings->value(QStringLiteral("audio/groups/%1").arg(normalizedGroup(group)),1.0).toReal(),1.0);}
qreal GameAudio::volume(const QString &group) const{return groupVolume(group);}
void GameAudio::setVolume(const QString &group,qreal value){if(!m_settings||group.isEmpty())return;m_settings->setValue(QStringLiteral("audio/groups/%1").arg(normalizedGroup(group)),qBound<qreal>(0.0,value,1.0));updateMusicVolume();}
QUrl GameAudio::permittedUrl(const QUrl &url) const{
    if(!url.isValid()||url.isEmpty()) return QUrl();
    if(!m_restrictedExternal) return url;
    if(m_gameId.isEmpty()) return QUrl();
    const QString expected=QStringLiteral("qrc:/mods/%1/").arg(m_gameId);
    const QString value=url.toString(QUrl::FullyDecoded);
    if(url.scheme().compare(QStringLiteral("qrc"),Qt::CaseInsensitive)==0 && value.startsWith(expected) && !value.contains(QStringLiteral(".."))) return url;
    if(url.scheme().isEmpty()){
        const QString rel=value;
        if(rel.isEmpty()||rel.startsWith(QLatin1Char('/'))||rel.contains(QStringLiteral(".."))||rel.contains(QLatin1Char('\\'))||QDir::cleanPath(rel)!=rel) return QUrl();
        const QString resource=QStringLiteral(":/mods/%1/%2").arg(m_gameId,rel);
        if(QFileInfo::exists(resource)) return QUrl(QStringLiteral("qrc:/mods/%1/%2").arg(m_gameId,rel));
    }
    return QUrl();
}
void GameAudio::playEffect(const QUrl &url){playEffect(url,1.0,QStringLiteral("sfx"));}
void GameAudio::playEffect(const QUrl &url,qreal gain){playEffect(url,gain,QStringLiteral("sfx"));}
void GameAudio::playEffect(const QUrl &url,qreal gain,const QString &group){const QUrl safe=permittedUrl(url);if(m_legacy&&!safe.isEmpty())m_legacy->playUrl(safe,gain*groupVolume(group));}
void GameAudio::preload(const QUrl &url){const QUrl safe=permittedUrl(url);if(m_legacy&&!safe.isEmpty())m_legacy->preload(safe);}
void GameAudio::preload(const QUrl &url,qreal legacyGain){Q_UNUSED(legacyGain) preload(url);}
void GameAudio::playMusic(const QUrl &url){playMusic(url,true,QStringLiteral("music"));}
void GameAudio::playMusic(const QUrl &url,bool loop){playMusic(url,loop,QStringLiteral("music"));}
void GameAudio::playMusic(const QUrl &url,bool loop,const QString &group)
{
#ifdef LMG_HAS_MULTIMEDIA
    const QUrl safe=permittedUrl(url);if(safe.isEmpty())return;m_impl->resumeMusic=false;m_impl->musicGroup=normalizedGroup(group);m_impl->player.setSource(safe);m_impl->player.setLoops(loop?QMediaPlayer::Infinite:1);updateMusicVolume();m_impl->player.play();
#else
    Q_UNUSED(url) Q_UNUSED(loop) Q_UNUSED(group)
#endif
}
void GameAudio::stopMusic(){
#ifdef LMG_HAS_MULTIMEDIA
m_impl->resumeMusic=false;m_impl->player.stop();m_impl->player.setSource(QUrl());
#endif
}
void GameAudio::pauseAll(){
#ifdef LMG_HAS_MULTIMEDIA
m_impl->resumeMusic=m_impl->player.playbackState()==QMediaPlayer::PlayingState;if(m_impl->resumeMusic)m_impl->player.pause();
#endif
}
void GameAudio::resumeAll(){
#ifdef LMG_HAS_MULTIMEDIA
if(m_impl->resumeMusic){m_impl->resumeMusic=false;m_impl->player.play();}
#endif
}
void GameAudio::updateMusicVolume(){
#ifdef LMG_HAS_MULTIMEDIA
qreal master=1.0;if(m_settings)master=m_settings->soundEnabled()?m_settings->soundVolume():0.0;m_impl->output.setVolume(qBound<qreal>(0.0,master*groupVolume(m_impl->musicGroup),1.0));
#endif
}
