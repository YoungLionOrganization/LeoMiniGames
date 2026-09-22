# Android build — LeoMiniGames 0.7.1

LeoMiniGames' Android target uses Qt's Android `qt-cmake` wrapper and Ninja. The supported project contract is Android SDK Platform 36, Build Tools 36.0.0 and NDK r27c (`27.2.12479018`). Qt 6.10 uses JDK 17+; the supported Qt 6.11 configuration uses JDK 21.

## Windows: recommended local build

Install **both** a desktop/host Qt kit and the Android ABI kit for the same Qt version. For example, for Qt 6.11.1 install `llvm-mingw_64` (host) plus `android_arm64_v8a` (target). Then from the repository root:

```bat
build_android.bat arm64-v8a Release apk
```

Supported ABI arguments are `arm64-v8a`, `armeabi-v7a`, `x86_64`, and `x86`; package can be `apk`, `aab`, or `both`.

The script detects the normal `D:\Qt` layout, Qt CMake/Ninja, `%LOCALAPPDATA%\Android\Sdk`, NDK r27c and common JDK locations. It deletes the ABI/config-specific build directory before configuration by default so a stale desktop/old-ABI CMake cache cannot poison the Android build. Set `LMG_KEEP_ANDROID_BUILD=1` only when you deliberately want incremental reconfiguration.

For a custom installation:

```bat
set QT_BASE=D:\Qt
set QT_VERSION=6.11.1
set QT_ANDROID_ROOT=D:\Qt\6.11.1\android_arm64_v8a
set ANDROID_SDK_ROOT=C:\Users\YOUR_USER\AppData\Local\Android\Sdk
set ANDROID_NDK_ROOT=%ANDROID_SDK_ROOT%\ndk\27.2.12479018
build_android.bat arm64-v8a Release apk
```

## Qt Creator

Select the **LeoMiniGames application target** and an Android kit that matches the intended ABI. Never reuse a desktop build directory for Android. If the kit, Qt version, SDK, NDK, or ABI changed, clear/reconfigure the build directory.

The project sets the Android package name from CMake (`QT_ANDROID_PACKAGE_NAME`) instead of hardcoding the Gradle namespace in the source manifest. Version/app-name placeholders are replaced by `androiddeployqt`. This keeps command-line and Qt Creator packaging on the same source of truth.

## CI parity

GitHub Actions uses the same SDK/NDK contract and tests all four Qt Android ABIs. The CI explicitly initializes the Android SDK, installs the exact components, and invokes the target `qt-cmake` wrapper through `bash` on Linux to avoid archive executable-bit failures.

## Common failures

- **Wrong product / “not an application, not building an APK”**: the active Qt Creator target is not `LeoMiniGames` or the Android build step is disabled.
- **Old compiler/ABI still appears**: remove the stale Android build directory and reconfigure with the intended kit.
- **SDK/NDK missing**: install Platform 36, Build Tools 36.0.0 and NDK `27.2.12479018` or point the environment variables to them.
- **Java/Gradle failure**: use JDK 17+ for Qt 6.10; prefer JDK 21 for Qt 6.11.
- **Manifest namespace/package error**: use the manifest shipped here; it intentionally does not hardcode a `package=` attribute and relies on Qt's package-name property.
- **No APK target**: configure with a Qt Android kit, not a desktop kit.

## Manifest/package metadata

`AndroidManifest.xml` intentionally uses Qt substitution placeholders for version/app name. The package id and SDK/version values are defined on the CMake target (`QT_ANDROID_PACKAGE_NAME`, `QT_ANDROID_VERSION_*`, `QT_ANDROID_*_SDK_VERSION`). Do not reintroduce a hard-coded manifest `package=` attribute; current Android tooling treats that field as deprecated for namespace/applicationId management.
