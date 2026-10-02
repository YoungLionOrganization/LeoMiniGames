// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/AudioManager.h"
#include "core/AudioClip.h"
#include "core/GameAudio.h"
#include "core/LegacyAudioFacade.h"
#include "core/SettingsManager.h"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QThread>
#include <QPointer>
#include <QDebug>
#include <cmath>
#ifdef LMG_HAS_MULTIMEDIA
#include <QMediaPlayer>
#include <QAudioBufferOutput>
#include <QAudioBuffer>
#endif

int main(int argc, char **argv)
{
    QTemporaryDir config;
    qputenv("XDG_CONFIG_HOME",config.path().toUtf8());
    QGuiApplication app(argc,argv);
    app.setOrganizationName("LMGTests"); app.setApplicationName("Audio");
    int count=0;
    auto check=[&](bool ok,const char *name){++count;if(!ok)qFatal("FAIL: %s",name);};
    auto wait=[&](auto predicate){QElapsedTimer timer;timer.start();while(!predicate()&&timer.elapsed()<5000){app.processEvents();QThread::msleep(5);}return predicate();};
    auto near=[](qreal a,qreal b){return std::abs(a-b)<0.00001;};
    SettingsManager settings; settings.setSoundEnabled(true); settings.setSoundVolume(1.0);
    AudioManager audio(&settings); GameAudio modern(&audio,&settings); LegacyAudioFacade legacy(&audio);
    modern.activate("test_audio",true);legacy.activate("test_audio");
    int errors=0; QObject::connect(&modern,&GameAudio::audioError,&app,[&]{++errors;});
#ifndef LMG_HAS_MULTIMEDIA
    check(!audio.available()&&!modern.available(),"disabled build reports unavailable");
    check(audio.capabilities().isEmpty(),"disabled build does not promise effect support");
    audio.play("click");check(errors==1,"disabled audio reports diagnostic without crashing");
#else
    check(modern.available()&&modern.capabilities().contains("music"),"modern capability discovery includes music");
    modern.setVolume("UI",0.4);modern.playEffect(QUrl("click"),0.5,"UI");
    auto clips=audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly);
    check(clips.size()==1,"modern named effect supported for external package");
    auto *clip=clips.first();
    check(near(clip->volume(),0.2),"master group and per-effect gain composed");
    check(clip->playbackUrl().isLocalFile()&&QFileInfo::exists(clip->playbackUrl().toLocalFile()),"RCC audio materialized to native local file");
    settings.setSoundVolume(0.5);check(near(clip->volume(),0.1),"master change preserves group and gain");
    modern.setVolume("ui",0.2);check(near(clip->volume(),0.05),"group change updates active effects");
    legacy.playUrl(QUrl("click"),0.8);check(near(clip->volume(),0.4),"legacy named overload preserves gain");
    settings.setSoundEnabled(false);check(clip->volume()==0&&!clip->pendingPlay(),"muting cancels queued effects");
    settings.setSoundEnabled(true);check(near(clip->volume(),0.4),"unmute restores effective volume");
    modern.pauseAll();modern.pauseAll();check(clip->paused()&&!clip->pendingPlay(),"pause idempotently stops effects");
    modern.playEffect(QUrl("move"));check(audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly).size()==1,"background effect requests suppressed");
    modern.resumeAll();check(!clip->paused()&&!clip->pendingPlay(),"resume does not replay old effects");
    audio.preload(QUrl("qrc:/sfx/./click.wav"));check(audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly).size()==1,"normalized source cache deduplicates URLs");
    modern.preload(QUrl("click"),0.5);check(audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly).size()==1,"named preload overload supported");
    const int before=errors;modern.playEffect(QUrl("file:///outside.wav"));check(errors==before+1,"rejected external source visible through modern API");
    modern.playMusic(QUrl("qrc:/mods/test_audio/missing.ogg"));check(errors==before+2,"music resource failure surfaced");modern.stopMusic();
    legacy.preload(QUrl("qrc:/mods/test_audio/click.wav"));
    QPointer<AudioClip> resourceClip;
    for(auto *item:audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly))if(item!=clip)resourceClip=item;
    check(resourceClip,"legacy resource uses common playback");
    const QString local=resourceClip->playbackUrl().toLocalFile();
    audio.releasePrefix("qrc:/mods/test_audio/");check(resourceClip.isNull()&&!QFileInfo::exists(local),"unload releases decoder and temporary audio file");
    modern.preload(QUrl("tone%231.ogg"));legacy.preload(QUrl("qrc:/mods/test_audio/tone%231.ogg"));
    check(audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly).size()==2,"encoded file names resolve equally through modern and legacy APIs");
    audio.releasePrefix("qrc:/mods/test_audio/");
    // Force the QSoundEffect decoder rejection while keeping a valid Ogg stream.
    // The shared host must fall back, decode real PCM, and preserve the request.
    AudioClip fallback(QUrl("qrc:/mods/test_audio/fallback.wav"),AudioClip::Usage::Effect);
    fallback.load();check(wait([&]{return fallback.usesMediaPlayer();}),"WAV decoder rejection falls back to media backend");
    auto *player=fallback.findChild<QMediaPlayer *>();
    QAudioBufferOutput buffers; qint64 decoded=0;
    QObject::connect(&buffers,&QAudioBufferOutput::audioBufferReceived,&app,[&](const QAudioBuffer &buffer){decoded+=buffer.byteCount();});
    player->setAudioBufferOutput(&buffers);
    fallback.play();check(wait([&]{return decoded>0;}),"fallback decodes real audio buffers");
    fallback.stop();decoded=0;fallback.play();check(wait([&]{return decoded>0;}),"fallback effect can be played again");
    player->setAudioBufferOutput(nullptr);
    modern.playMusic(QUrl("tone.ogg"),true,"ambient");
    auto music=modern.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly);
    check(music.size()==1&&music.first()->usesMediaPlayer(),"relative modern music uses host media player");
    modern.pauseAll();modern.pauseAll();check(music.first()->paused(),"music pause during async loading retained");
    modern.resumeAll();check(!music.first()->paused(),"music resumes after repeated pause");
    modern.pauseAll();modern.playMusic(QUrl("tone.ogg"));
    auto *backgroundMusic=modern.findChild<AudioClip *>();
    check(backgroundMusic&&backgroundMusic->paused()&&backgroundMusic->pendingPlay(),"music requested while paused waits for foreground");
    modern.resumeAll();check(!backgroundMusic->paused(),"background music request resumes through host");
    modern.activate("",false);check(modern.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly).isEmpty(),"session close releases music");
    // Bound cached decoders/files even when preloading many package paths.
    for(int i=0;i<40;++i)audio.preload(QUrl::fromLocalFile(config.filePath(QString::number(i)+".wav")));
    check(audio.findChildren<AudioClip *>(QString(),Qt::FindDirectChildrenOnly).size()==32,"effect cache remains bounded");
    modern.setVolume("sfx",std::nan(""));check(modern.volume("sfx")==0,"non-finite group volume becomes silence");
    settings.setSoundVolume(std::nan(""));check(settings.soundVolume()==0,"non-finite master volume becomes silence");
#endif
    qInfo()<<"PASS:"<<count<<"audio assertions";
}
