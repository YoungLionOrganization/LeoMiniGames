# LeoMiniGames v0.7.0 Build Matrix

This matrix distinguishes **configured CI coverage** from **physically verified release behavior**. A successful GitHub Actions job proves that the source builds/packages on that runner; it does not replace real-device QA.

| Target | CPU / ABI | Toolchain / runner | CI action | Release artifact |
| --- | --- | --- | --- | --- |
| Ubuntu 22.04 | x86_64 | Qt 6.10.2 | build + CTest | tar.gz + AppImage |
| Ubuntu 24.04 | x86_64 | Qt 6.10.2 | build + CTest | tar.gz + AppImage |
| Ubuntu 24.04 | arm64 | native ARM runner + Qt 6.10.2 | build + CTest | tar.gz + AppImage |
| Debian 13 | x86_64 | distro Qt 6.8.x | build + CTest | native tar.gz |
| Debian 13 | arm64 | native ARM runner + distro Qt 6.8.x | build + CTest | native tar.gz |
| Arch Linux | x86_64 | official `archlinux:latest` container | build + CTest | native tar.gz |
| Windows | x86_64 | MSVC 2022 | build + CTest | ZIP + QtIFW Setup EXE |
| Windows | x86_64 | LLVM-MinGW | build + CTest | compatibility lane |
| Windows | ARM64 | native MSVC 2022 ARM runner | build | ZIP + QtIFW Setup EXE |
| macOS | arm64 | macOS 15 | build + CTest | ZIP + DMG |
| macOS | x86_64 | macOS 15 Intel | build + CTest | ZIP + DMG |
| macOS | universal | arm64 + x86_64 | build + CTest | ZIP + DMG |
| Android | arm64-v8a | NDK r27c / JDK 17 | APK | APK + AAB |
| Android | armeabi-v7a | NDK r27c / JDK 17 | APK | APK + AAB |
| Android | x86_64 | NDK r27c / JDK 17 | APK | APK + AAB |
| Android | x86 | NDK r27c / JDK 17 | APK | APK + AAB |
| Android universal | 4 ABIs | Qt multi-ABI | APK | APK + AAB |
| iOS device | arm64 | Xcode / Qt iOS | unsigned build | unsigned app ZIP |
| iOS Simulator | x86_64 | Xcode / Qt iOS | unsigned build | unsigned app ZIP |
| iPadOS device | arm64 | Xcode / Qt iOS, iPad family | unsigned build | unsigned app ZIP |
| iPadOS Simulator | x86_64 | Xcode / Qt iOS, iPad family | unsigned build | unsigned app ZIP |

## Architecture notes

The official Docker `archlinux` image does not currently publish a Linux/arm64 manifest, so `Arch-arm64` is not advertised as an official CI lane. ARM64 Linux coverage is provided by native Ubuntu 24.04 and Debian 13 runners instead.

The Qt 6.10.2 online iOS kit used by CI supplies an arm64 **device** slice and an x86_64 **simulator** slice. It does not provide a usable arm64 simulator slice for the Qt libraries/plugins in this lane. Therefore CI does not claim an iOS/iPadOS arm64-simulator build. Device arm64 and simulator x86_64 remain covered for both iOS and iPadOS.

Qt 6.11.1 remains a supported local-development kit. CI uses 6.10.2 for Qt online-package lanes because that version is currently resolved reliably by the pinned installer action. The source-level minimum remains Qt 6.5.

## Real-device QA

Windows/Linux desktop and Android/iOS/iPadOS/macOS hardware behavior must still be recorded separately. Audio backend behavior, haptics, lifecycle/background save, touch/safe-area handling, installer/signing/notarization and update signatures cannot be proven solely by static validation or cross-compilation.
