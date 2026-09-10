# LeoMiniGames v0.7.0 Build Matrix

This matrix distinguishes **configured CI coverage** from **physically verified release behavior**. A workflow entry is not evidence that a real device was tested.

| Target | CPU / ABI | Toolchain / runner | CI action | Artifact |
| --- | --- | --- | --- | --- |
| Linux | x86_64 | Ubuntu 22.04 + Qt 6.10.2 | build + CTest | compatibility lane |
| Linux | x86_64 | Ubuntu 24.04 + Qt 6.10.2 | build + CTest | tar.gz + AppImage |
| Linux | arm64 | Ubuntu 24.04 ARM + Qt 6.10.2 | build + CTest | tar.gz + AppImage |
| Windows | x86_64 | MSVC 2022 | build + CTest | ZIP + QtIFW EXE |
| Windows | x86_64 | LLVM-MinGW | build + CTest | compatibility lane |
| Windows | ARM64 | MSVC 2022 cross kit | build | ZIP + QtIFW EXE |
| macOS | arm64 | macOS 15 | build + CTest | ZIP + DMG |
| macOS | x86_64 | macOS 15 Intel | build + CTest | ZIP + DMG |
| Android | arm64-v8a | NDK r27c/JDK 17 | APK | signed APK + AAB lane |
| Android | armeabi-v7a | NDK r27c/JDK 17 | APK | signed APK |
| Android | x86_64 | NDK r27c/JDK 17 | APK | signed APK |
| Android | x86 | NDK r27c/JDK 17 | APK | signed APK |
| iOS Simulator | arm64 | Xcode/macOS 15 | unsigned build | ZIP |
| iOS Simulator | x86_64 | Xcode/macOS Intel | unsigned build | ZIP |

Qt 6.11.1 remains a supported local-development kit. CI uses 6.10.2 because the pinned GitHub Qt installer path can currently resolve it reliably. The source-level minimum remains Qt 6.5.

## Real-device QA

Windows/Linux desktop and Android/iOS/macOS hardware behavior must still be recorded separately. In particular, audio backend behavior, haptics, lifecycle/background save, touch/safe-area handling, installer/signing/notarization and update signatures cannot be proven solely by static validation or cross-compilation.
