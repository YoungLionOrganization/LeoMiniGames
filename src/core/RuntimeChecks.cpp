// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "RuntimeChecks.h"
#include "AudioClip.h"
#include "AudioManager.h"
#include "SettingsManager.h"
#include <QBuffer>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>
#ifdef LMG_HAS_MULTIMEDIA
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QSoundEffect>
#endif

bool RuntimeChecks::graphics(QQmlEngine *engine, QString *error)
{
    // Exercise real codecs, not just the presence of similarly named files.
    for (const QByteArray format : {QByteArray("PNG"), QByteArray("JPEG"), QByteArray("WEBP")}) {
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(QColor(19, 77, 139));
        QByteArray bytes;
        QBuffer output(&bytes);
        output.open(QIODevice::WriteOnly);
        if (!image.save(&output, format.constData()) || QImage::fromData(bytes, format.constData()).isNull()) {
            *error = QStringLiteral("Missing or unusable %1 image codec").arg(QString::fromLatin1(format));
            return false;
        }
    }
    const QByteArray svg("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"4\" height=\"4\"><rect width=\"4\" height=\"4\" fill=\"#134d8b\"/></svg>");
    if (QImage::fromData(svg, "SVG").isNull()) {
        *error = QStringLiteral("Missing or unusable SVG image plugin");
        return false;
    }
    QQmlComponent component(engine);
    component.setData("import QtQuick\nimport QtQuick.Controls\nimport QtQuick.Shapes\nimport QtQuick.Dialogs\nItem { Shape {} FileDialog {} }", QUrl("qrc:/runtime-check.qml"));
    if (!component.isReady()) {
        *error = component.errorString();
        return false;
    }
    std::puts("PASS: PNG JPEG WebP SVG codecs and Controls Shapes Dialogs imports");
    return true;
}

int RuntimeChecks::audio()
{
#ifndef LMG_HAS_MULTIMEDIA
    qCritical("Audio probe: this build has no Multimedia support");
    return 1;
#else
    QTemporaryDir directory;
    if (!directory.isValid()) return 1;
    SettingsManager settings(directory.filePath("audio.ini"));
    settings.setSoundEnabled(true);
    settings.setSoundVolume(1.0);
    AudioManager effects(&settings);
    AudioClip music(QUrl("qrc:/sfx/win.wav"), AudioClip::Usage::Music);
    QMediaDevices devices;
    QEventLoop loop;
    QElapsedTimer elapsed;
    elapsed.start();
    QTimer poll, watchdog;
    bool failed = false, effectPlayed = false, musicPlayed = false, started = false;
    const auto fail = [&](const QString &message, const QString &source) {
        failed = true;
        qCritical().noquote() << "Audio probe:" << message << source;
        loop.quit();
    };
    QObject::connect(&effects, &AudioManager::audioError, &loop, fail);
    QObject::connect(&music, &AudioClip::errorOccurred, &loop, fail);
    QObject::connect(&watchdog, &QTimer::timeout, &loop, [&] { fail("Audio probe timed out", {}); });
    watchdog.setSingleShot(true);
    watchdog.start(8000);
    QObject::connect(&poll, &QTimer::timeout, &loop, [&] {
        if (started || failed) return;
        if (devices.defaultAudioOutput().isNull()) {
            if (elapsed.elapsed() > 3000) fail("No audio output device became available", {});
            return;
        }
        started = true;
        effects.play("win");
        for (auto *wave : effects.findChildren<QSoundEffect *>()) {
            effectPlayed |= wave->isPlaying();
            QObject::connect(wave, &QSoundEffect::playingChanged, &loop, [&, wave] { effectPlayed |= wave->isPlaying(); });
        }
        QTimer::singleShot(1000, &loop, [&] {
            if (failed) return;
            music.load();
            if (auto *player = music.findChild<QMediaPlayer *>())
                QObject::connect(player, &QMediaPlayer::playbackStateChanged, &loop, [&](QMediaPlayer::PlaybackState state) {
                    musicPlayed |= state == QMediaPlayer::PlayingState;
                });
            music.play(false);
        });
        QTimer::singleShot(2800, &loop, &QEventLoop::quit);
    });
    poll.start(100);
    loop.exec();
    effects.stopAll();
    music.stop();
    if (failed || !effectPlayed || !musicPlayed) {
        qCritical() << "Audio probe playback failed; effect/music:" << effectPlayed << musicPlayed;
        return 1;
    }
    std::puts("PASS: named effect and media-player music reached playback");
    return 0;
#endif
}
