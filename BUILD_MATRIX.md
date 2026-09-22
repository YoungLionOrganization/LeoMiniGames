# LeoMiniGames v0.7.1 Build Matrix

This matrix distinguishes **configured CI coverage** from **physically verified release behavior**. A successful GitHub Actions job proves that the source builds/packages on that runner; it does not replace real-device QA.

## Current GitHub evidence

As of 2026-09-22, **LeoMiniGames CI #21** and **Build Release Artifacts #9** both completed successfully for exact commit `9943b5c1824b81fad87f2d67741fefd714c869f6`. The first `Publish Release` run completed preflight successfully in validation mode; its publish job was intentionally skipped because `mode=publish` was not selected.

This is CI/build evidence only. It does not convert any platform row into a physical-device verification claim.

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

## Compatibility and packaging notes

Debian 13 intentionally validates the documented Qt 6.5+ source floor with distro Qt 6.8.x. Filter refresh code therefore uses the Qt 6.10 filter-change API when available and `invalidateRowsFilter()` on Qt 6.5-6.9.

Ubuntu AppImage packaging installs both GStreamer Good and Bad runtime plugin libraries before linuxdeploy scans Qt Multimedia, covering `libgstphotography` and `libgstplay` dependencies on supported Ubuntu runners.

macOS DMG generation uses a fresh temporary image, stale-volume detach and bounded retry so a transient `hdiutil: Resource busy` does not discard an otherwise successful build/package stage.

## Real-device QA

Windows/Linux desktop and Android/iOS/iPadOS/macOS hardware behavior must still be recorded separately. Audio backend behavior, haptics, lifecycle/background save, touch/safe-area handling, installer/signing/notarization and update signatures cannot be proven solely by static validation or cross-compilation.
