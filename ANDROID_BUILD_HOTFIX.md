# LeoMiniGames v0.6.2 Android build hotfix

This archive keeps the v0.6.2 project/version and applies Android-specific build/runtime fixes.

## Fixed

- Replaced the failing `#include <QNativeInterface>` path in `Haptics.cpp` with the Qt-supported `#include <QCoreApplication>` entry point for `QNativeInterface::QAndroidApplication`.
- Reworked Android haptics to use `VibrationEffect.createOneShot()` (available because minSdk is 28) instead of deprecated `Vibrator.vibrate(long)`.
- Replaced `QApplication` with `QGuiApplication` and removed the unused Qt Widgets dependency. The application is Qt Quick/QML-only, so Android no longer needs a Widgets module just to start the app.
- Added the VIBRATE permission to the CMake Android permission list while retaining the manifest declaration for compatibility.
- Made the `_3` Android OpenSSL suffix conditional on the bundled KDAB OpenSSL path. privacy/source-built OpenSSL builds no longer inherit a suffix intended for the prebuilt bundle.
- Removed the forced `arm64-v8a` target property. The selected Qt Android kit now controls the ABI by default, avoiding kit/ABI mismatches and allowing Qt 6.11 multi-ABI packaging.
- Added an explicit `android/res/xml/qtprovider_paths.xml` matching the FileProvider declared by the Android manifest, so the custom Android package source is self-contained.

## Android build target

In Qt Creator, deploy/build the `LeoMiniGames` executable target. The built-in game plugin targets are static dependencies and are not Android application targets.

## Validation performed in this archive

The project source validators, XML parsing, JSON parsing, CMake static checks, and Android-specific source guards were run after the patch. A real APK build still requires a local Qt for Android SDK/NDK installation.
