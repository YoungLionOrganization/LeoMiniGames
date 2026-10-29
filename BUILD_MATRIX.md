# LeoMiniGames v0.7.3 Build Matrix

This matrix distinguishes **configured CI coverage** from **physically verified release behavior**. A successful GitHub Actions job proves that the source builds/packages on that runner; it does not replace real-device QA.

## Current GitHub evidence

CI run [39050064290](https://github.com/YoungLionOrganization/LeoMiniGames/actions/runs/39050064290) for `3747c5f2f0ed4fdf348d056f354928eea714423e` initially passed 23 jobs, including Windows, Linux distro servicing and both Flatpak architectures. Three Android emulator catalog probes received HTTP 500 after successful APK compilation and OpenSSL loading. The public mods/themes APIs returned HTTP 200 when rechecked on 30 October; the failed jobs were rerun. These results apply only to that source SHA. A new source commit requires its own CI and artifact validation.

This is CI/build evidence only. It does not convert any platform row into a physical-device verification claim.

| Target | CPU / ABI | Toolchain / runner | CI action | Release artifact |
| --- | --- | --- | --- | --- |
| Ubuntu 22.04 | x86_64 | Qt 6.10.2 | build + CTest; packaged installer lifecycle + Xvfb | tar.gz + AppImage + Setup.run |
| Ubuntu 24.04 | x86_64 | Qt 6.10.2 | build + CTest; packaged installer lifecycle + Xvfb | tar.gz + AppImage + Setup.run |
| Ubuntu 24.04 | arm64 | native ARM runner + Qt 6.10.2 | build + CTest; packaged installer lifecycle + Xvfb | tar.gz + AppImage + Setup.run |
| Debian 13 | x86_64 / arm64 | distro Qt; native ARM runner for arm64 | build + CTest; install/reinstall/uninstall + Xvfb | DEB + native tar.gz |
| Fedora 43 | x86_64 / arm64 | distro Qt; native ARM runner for arm64 | build + CTest; install/reinstall/uninstall + Xvfb | RPM + native tar.gz |
| Arch Linux | x86_64 | official `archlinux:latest` container | build + CTest; install/reinstall/uninstall + Xvfb | pkg.tar.zst + native tar.gz |
| Flatpak | x86_64 / aarch64 | KDE 6.10 SDK/runtime | sandbox build; install/reinstall/uninstall + Xvfb | flatpak bundle |
| Windows | x86_64 baseline and AVX2 | MSVC 2022 | build + CTest | ZIP + QtIFW Setup EXE for each CPU profile |
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

The source minimum is Qt **6.8**. The v0.7.3 online-package release lanes use **6.10.2**. The separate `Qt 6.12 compatibility` workflow builds/tests **6.12.0** on Linux, Windows and macOS GitHub runners. It does not install Qt onto the maintainer's computer or change the release kit. A configured lane is not a passing result until its own run completes.

## Compatibility and packaging notes

Debian 13 validates the Qt 6.8 source floor with distro Qt. Filter refresh uses the Qt 6.10 filter-change API when available and the older API on supported earlier kits. Native distro tarballs depend on system Qt; AppImage/portable tarballs deploy their runtime. Flatpak uses its declared KDE runtime and keeps its own data directory.

Package selection and installation are documented in [Türkçe kurulum rehberi](docs/KURULUM.md) and [Linux packages](docs/LINUX_PACKAGES.md). HarmonyOS/HAP work is [planned separately](docs/roadmap/HARMONYOS.md); it is not a v0.7.3 release target.

Ubuntu AppImage packaging installs both GStreamer Good and Bad runtime plugin libraries before linuxdeploy scans Qt Multimedia, covering `libgstphotography` and `libgstplay` dependencies on supported Ubuntu runners.

macOS DMG generation uses a fresh temporary image, stale-volume detach and bounded retry so a transient `hdiutil: Resource busy` does not discard an otherwise successful build/package stage.

## Real-device QA

Windows/Linux desktop and Android/iOS/iPadOS/macOS hardware behavior must still be recorded separately. Audio backend behavior, haptics, lifecycle/background save, touch/safe-area handling, installer/signing/notarization and update signatures cannot be proven solely by static validation or cross-compilation.


## v0.7.1 Windows installer / CPU matrix follow-up — 2026-09-24

| Artifact | CPU scope | Installer | Update architecture |
| --- | --- | --- | --- |
| Windows x86_64 baseline | Intel + AMD 64-bit x86 | ZIP + QtIFW Setup | `x86_64` |
| Windows x86_64 AVX2 | Newer Intel + AMD with AVX2 | optimized ZIP + QtIFW Setup | `x86_64-avx2` |
| Windows ARM64 | Windows on ARM | ZIP + QtIFW Setup | `arm64` |
| Windows x86 32-bit | not a supported Qt 6 Windows release target | not produced | not advertised |

The AVX2 lane is a real MSVC `/arch:AVX2` build, not a renamed baseline archive. Physical execution of the newly generated installer variants still requires the next Windows GitHub Actions build / Windows QA run and is not claimed by this source audit.

For v0.7.2 the user confirmed the previously failing existing-installation Update route and Setup screens work. The other variants and platform operations still need their own hardware checks.
