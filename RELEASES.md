# LeoMiniGames Releases

This document is the canonical release/upgrade guide for LeoMiniGames. It is intentionally more detailed than a short GitHub Release description: it records artifact expectations, compatibility guarantees, migration rules, validation status and known limitations.

## Release policy

LeoMiniGames uses semantic-style `MAJOR.MINOR.PATCH` versions for the application/runtime contract. Package/API compatibility is handled separately through manifest negotiation so an application release does not automatically invalidate older RCC content.

The GitHub repository does **not** automatically create Releases from tags. `.github/workflows/build-artifacts.yml` is a manual artifact builder; generated files are intended to be reviewed and then attached to a GitHub Release by the maintainer.

## v0.7.0 — compatibility/security/SDK upgrade

### Release objective

v0.7.0 strengthens the host/runtime boundary without forcing supported v0.5/v0.6 RCC games to be repackaged. The central design is **compatibility through scoped adapters**: old service names and package layouts remain usable while raw host internals are no longer unnecessarily exposed to external game QML.

### Major additions

Developer Lab can validate and session-mount local RCC packages, report API/capability/mount information, export diagnostics and preview device profiles. Modern packages can declare API requirements with explicit `required_capabilities` rather than relying on implicit runtime assumptions.

The game-facing theme and i18n contracts are expanded. Input centralizes physical WASD handling. Save/lifecycle paths use explicit forced persistence at durability boundaries. Audio becomes lazy/capability-aware instead of assuming every platform/backend behaves identically.

### Security and trust changes

Publisher verification is authoritative only when it comes from the trusted YoungLion catalog boundary. A local manifest or custom catalog cannot self-assign Official, Verified or Native/L3 status. Legacy admin-only canonical catalog entries are compatibility-mapped to Official so historical official releases do not incorrectly become Unverified.

External QML receives game-scoped service facades in a separate QML engine. Legacy Settings/Audio/Logger convenience APIs remain available where required, but their access is scoped to the active game/package. Legacy RCC prefix support verifies namespace containment before using a global resource mount.

Native/L3 remains a separate reviewed capability. License choice does not imply Native/L3 permission.

### Backward-compatibility contract

A missing `api_version` remains valid and identifies a legacy package. New RCCs use internal prefix `/` and are mounted at `/mods/<id>`. Historical RCCs that already contain `/mods/<id>` remain accepted when the package is fully contained in that namespace.

Legacy public service names remain available through adapters. Existing game save representations are not rewritten merely because the host upgraded to v0.7.0. Legacy settings can be copied into namespaced storage on read, preserving previous user configuration while isolating future writes.

Compatibility fixtures cover canonical v0.5, legacy-prefix v0.5, v0.6 services, v0.7 modern packages and a deliberately malicious legacy namespace fixture that must be rejected.

### Fixed runtime issues

This release includes fixes for QML syntax/cache-generation errors in Developer Lab, duplicate Qt target finalization, GameAudio URL/string type mismatch, QML network factory API/lifetime handling, missing `GameResources` constructor linkage, installed-content index duplicate iteration, network capability propagation, market model mutation races during downloads, lifecycle force-save gaps, empty state-service paths and inherited QML property shadowing.

Android CI additionally fixes two concrete workflow/build failures: `Haptics.cpp` no longer includes a standalone `QNativeInterface` header when Qt exposes the Android application interface via the Core application header, and Android target `qt-cmake` is invoked through `bash` so a lost archive execute bit does not break armeabi-v7a configuration.

### Platform and CPU coverage

The v0.7.0 GitHub CI configuration covers:

| Platform | CPU/ABI/toolchain | CI operation |
| --- | --- | --- |
| Linux Ubuntu 22.04 | x86_64 | build + CTest |
| Linux Ubuntu 24.04 | x86_64 | build + CTest |
| Linux Ubuntu 24.04 | arm64 | build + CTest |
| Windows | x86_64 MSVC 2022 | build + CTest |
| Windows | x86_64 LLVM-MinGW | build + CTest |
| Windows | ARM64 MSVC cross-build | build |
| macOS | arm64 | build + CTest |
| macOS | x86_64 | build + CTest |
| Android | arm64-v8a | APK |
| Android | armeabi-v7a | APK |
| Android | x86_64 | APK |
| Android | x86 | APK |
| iOS Simulator | x86_64 | unsigned build |

These are configured CI lanes. A newly added lane is not labelled “physically verified” until its workflow and, where relevant, hardware QA have actually completed.

### Release artifact matrix

The manual Build Release Artifacts workflow is expected to produce:

| Target | Artifact names/purpose |
| --- | --- |
| Source | `LeoMiniGames-v0.7.0-Source.zip` + SHA-256 |
| Windows x86_64 | portable ZIP + QtIFW Setup EXE |
| Windows ARM64 | portable ZIP + QtIFW Setup EXE |
| Linux x86_64 | runtime tar.gz + AppImage |
| Linux ARM64 | runtime tar.gz + AppImage |
| macOS arm64 | deployed ZIP + DMG |
| macOS x86_64 | deployed ZIP + DMG |
| Android arm64-v8a | signed APK + signed AAB lane |
| Android armeabi-v7a | signed APK |
| Android x86_64 | signed APK |
| Android x86 | signed APK |
| iOS Simulator x86_64 | unsigned test ZIP |

### Windows installation

The portable ZIP is deployed with `windeployqt`. The Qt Installer Framework installer includes the same staged application payload. The main component is marked `ForcedInstallation` and `Essential` so an installer build cannot intentionally present only the Maintenance Tool as the installed product.

Windows artifacts are not claimed to be Authenticode-signed unless a signing certificate/signing stage has actually been configured.

### Linux installation

The portable archive and AppImage derive from a `linuxdeploy` AppDir that contains the application, Qt runtime dependencies, desktop entry and icon. x86_64 and ARM64 use architecture-matching AppImage tooling.

### macOS installation

`macdeployqt` stages the application before ZIP/DMG creation. The current workflow does not pretend to provide Developer ID signing or notarization without Apple credentials. Unsigned artifacts are appropriate for CI/testing, not a substitute for a notarized public macOS distribution policy.

### Android installation and signing

Release APKs use the maintainer-provided GitHub Actions keystore secrets. The keystore itself is never committed. Configured ABIs are `arm64-v8a`, `armeabi-v7a`, `x86_64` and `x86`.

To preserve Android update compatibility, continue using the correct established upload/distribution key for the same application ID. A differently signed APK may not be accepted as an update over an existing installation.

The current AAB lane is explicitly arm64-v8a. A future multi-ABI AAB should use Qt's CMake multi-ABI support after all corresponding Qt Android kits are installed and verified; v0.7.0 does not falsely label a single-ABI bundle as universal.

### iOS status

CI produces unsigned arm64 device builds and x86_64 simulator archives for both iOS and iPadOS. These are developer/test artifacts. Device IPA/App Store distribution requires Apple signing/provisioning and is not claimed by this workflow.

### Source-available / F-Droid transition

Starting with the source-available licensing transition, the LeoMiniGames host uses `LicenseRef-LMG-SAPEL-1.0`. This license restricts redistribution and therefore current/future source-available editions must not be represented as eligible for the official F-Droid main repository.

The previous F-Droid submission scaffold has been removed. The tracker-free build intent remains available as the neutral **privacy build** profile (`LEOMINIGAMES_PRIVACY_BUILD` / `tools/privacy_clean_build.sh`). A YoungLion-controlled Android repository can be operated separately subject to signing, Qt/LGPL and other third-party obligations. See `docs/F_DROID_TRANSITION.md`.

### Qt version policy

`CMakeLists.txt` retains Qt 6.5 as the minimum API floor. Local development may use Qt 6.11.1. GitHub CI currently pins Qt 6.10.2 because that version has been reliably retrievable by the pinned `install-qt-action`/aqt path. This CI pin does not redefine the application's maximum/supported Qt version.

### Validation

Repository validation includes source guard, project consistency, sanity, security audit, i18n coverage/placeholder validation, distribution checks, prebuilt-binary audit and the v0.7 contract validator. Qt-enabled jobs additionally compile and run CTest where target/toolchain semantics allow it.

Static validation is evidence about source/package contracts; it is not equivalent to real device testing. See `BUG_HUNTER_AUDIT.md`, `GAMER_AUDIT.md`, `VALIDATION_REPORT.md`, `BUILD_MATRIX.md` and `COMPATIBILITY_REPORT.md`.

### Upgrade notes for users

Users upgrading from v0.5/v0.6 should not need to reinstall compatible RCC content solely because of the v0.7 API. Existing game state should be preserved by the save compatibility paths. If a third-party package fails to load, diagnostics should be checked for namespace, malformed manifest, unsupported required capability or an actual runtime error rather than assuming all legacy content is unsupported.

### Migration notes for game/mod developers

Existing legacy packages may remain unchanged while they are supported. New development should move to canonical prefix `/`, documented game-scoped host services and explicit `api_version`/`required_capabilities`. Do not put trust assertions such as Official/Verified/native permission in a manifest expecting the host to honor them.

### Known limitations

Translation coverage is not yet 100% across every supported locale; source-English fallback remains intentional. Full native browser-to-app Developer OAuth callback support remains a backend integration dependency. Apple public distribution signing/notarization is not automated. Newly added CI architecture lanes require their first successful GitHub run before being recorded as verified.

## Manual release procedure

1. Ensure `CMakeLists.txt`, Android versionName/versionCode, installer metadata, changelog and release documentation agree.
2. Run CI and resolve all required job failures.
3. Run **Build Release Artifacts** manually with the exact version.
4. Download and inspect the produced artifacts; verify checksums/signatures where applicable.
5. Perform platform smoke tests on the release binaries, especially Windows and Android.
6. Create the GitHub Release manually and attach the reviewed artifacts.
7. Record known limitations rather than hiding skipped signing/device tests.

## Rollback principle

If a v0.7 runtime/security change breaks a legitimate v0.5/v0.6 package, prefer a compatibility adapter/migration/fallback over deleting old package support. Security exceptions must remain scoped: backward compatibility is not a reason to restore global host filesystem/settings/trust access to external QML.

## Older version documentation

Historical notes remain in `V0.5.0_CHANGES.md`, `V0.5.1_CHANGES.md`, `V0.5.2_CHANGES.md`, `V0.6.1_CHANGES.md` and `V0.6.2_CHANGES.md`.

## Section 22 exclusion

The repository intentionally contains **no tag-triggered automatic GitHub Release creation**. Manual artifact generation and manual release publication remain separate operations unless that policy is explicitly changed in a future task.
