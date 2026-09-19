// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "Haptics.h"

#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>

namespace {
QJniObject androidVibrator()
{
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return QJniObject();

    const QJniObject serviceName = QJniObject::fromString(QStringLiteral("vibrator"));
    return context.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        serviceName.object<jstring>());
}
}
#endif

Haptics::Haptics(QObject *parent)
    : QObject(parent)
{
}

bool Haptics::available() const
{
#ifdef Q_OS_ANDROID
    const QJniObject vibrator = androidVibrator();
    return vibrator.isValid() && vibrator.callMethod<jboolean>("hasVibrator", "()Z");
#else
    return false;
#endif
}

void Haptics::light()
{
    vibrate(16);
}

void Haptics::medium()
{
    vibrate(28);
}

void Haptics::heavy()
{
    vibrate(45);
}

void Haptics::pulse(const QString &level)
{
    const QString value = level.toLower();
    if (value == QStringLiteral("heavy"))
        heavy();
    else if (value == QStringLiteral("medium"))
        medium();
    else
        light();
}

void Haptics::vibrate(int milliseconds)
{
#ifdef Q_OS_ANDROID
    const QJniObject vibrator = androidVibrator();
    if (!vibrator.isValid() || !vibrator.callMethod<jboolean>("hasVibrator", "()Z"))
        return;

    // minSdk is 28, so VibrationEffect (API 26+) is always available. Using it
    // avoids the deprecated Vibrator.vibrate(long) path on current Android.
    const jlong duration = static_cast<jlong>(qBound(1, milliseconds, 500));
    constexpr jint defaultAmplitude = -1; // VibrationEffect.DEFAULT_AMPLITUDE
    const QJniObject effect = QJniObject::callStaticObjectMethod(
        "android/os/VibrationEffect",
        "createOneShot",
        "(JI)Landroid/os/VibrationEffect;",
        duration,
        defaultAmplitude);

    if (!effect.isValid())
        return;

    vibrator.callMethod<void>(
        "vibrate",
        "(Landroid/os/VibrationEffect;)V",
        effect.object<jobject>());
#else
    Q_UNUSED(milliseconds)
#endif
}
