# LeoMiniGames v0.7.0 — Build & Packaging Fix Report

Date: 2026-09-14

## Resolved source/build failures

- Fixed `DeveloperLabPage.qml` syntax failure (`Unexpected token ;`) and retained qualified/bound-safe QML access.
- Removed the duplicate Qt target-finalization path that produced `qt6_finalize_target twice` warnings.
- Fixed `GameAudio` → `AudioManager::releasePrefix()` type mismatch by passing `QString` instead of `QUrl`.
- Corrected the QML network access factory include to `QQmlNetworkAccessManagerFactory`.
- Added the missing `GameResources::GameResources(QObject*)` implementation that caused the undefined linker symbol.
- Removed the unavailable standalone `<QNativeInterface>` include on Android; Android haptics obtains `QNativeInterface::QAndroidApplication` through Qt Core headers.
- Fixed CI's false failure where `validate_distribution.py` searched obsolete literal artifact names and skipped the whole platform matrix before compilation. The validator now parses workflow YAML and validates job/matrix semantics.
- Android CI invokes target `qt-cmake` through `bash`, avoiding archive execute-bit failures seen on the armv7 Qt package.
- Android package/application metadata is sourced from CMake (`QT_ANDROID_*`) and the custom manifest uses Qt substitution placeholders instead of duplicating version/application data.
- Removed the unused AndroidX `FileProvider`, avoiding an unnecessary AndroidX dependency in a pure Qt package.
- Added Windows `.ico` executable resource, macOS `.icns`, Android launcher icon property and QtIFW installer/window/page-list artwork.
- Windows installer shortcuts now set working directory and explicit executable icon.
- Source packaging no longer accidentally drops `build_android.bat` / `build_android.sh` merely because their filenames begin with `build`. Build *directories* are still excluded.
- Release artifact workflow uses Qt Installer Framework 4.11 (`qt.tools.ifw.411`) and a manual-only artifact pipeline.

## Compatibility retained

The runtime compatibility fixtures for v0.5 canonical RCC, v0.5 legacy-prefix RCC, v0.6 service packages and v0.7 modern packages remain validated. New license metadata is not made mandatory for loading historical installed packages.

## Validation status

Static validation passes with no errors. The only expected distribution warning is the absence of real upstream F-Droid screenshots. Translation coverage warnings remain quality debt rather than runtime failures because source-text fallback is enabled.

## Not claimed

This package has not been physically executed on every Windows/Linux/macOS/Android/iOS target in this environment. CI/device success must be confirmed after the final tree is pushed to GitHub and the native workflows run with the required signing credentials.
