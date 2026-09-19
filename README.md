# LeoMiniGames

[![CI](https://github.com/YoungLionOrganization/LeoMiniGames/actions/workflows/ci.yml/badge.svg)](https://github.com/YoungLionOrganization/LeoMiniGames/actions/workflows/ci.yml)
[![Qt](https://img.shields.io/badge/Qt-6.5%2B-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-Source--Available%20SAPEL--1.0-orange)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.7.0-B8860B)](RELEASES.md)
[![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20Android%20%7C%20iOS%20%7C%20macOS-informational)](#platform-and-architecture-matrix)

**LeoMiniGames** is a modular, cross-platform Qt 6 / Qt Quick mini-game launcher, runtime and content ecosystem. Games, mods and themes are designed to remain as independent from the host application as practical while reusing a common service layer for theme, save, localization, audio, haptics, input, lifecycle, statistics, achievements, package management and developer tooling.

The v0.7.0 line is a **compatibility-first security and SDK upgrade**. A major design requirement is that supported v0.5/v0.6 RCC games continue to run through compatibility adapters instead of being rejected merely because they predate the new API negotiation model.

> **Project rule:** reliability and backward compatibility take priority over forcing old content to migrate. New security boundaries are introduced through scoped adapters, validation and trust provenance rather than by deleting legacy public APIs.

## Contents

- [Highlights](#v070-highlights)
- [Built-in games](#built-in-games)
- [Architecture](#architecture)
- [Runtime/content types](#runtime-and-content-types)
- [Platform and architecture matrix](#platform-and-architecture-matrix)
- [Backward compatibility](#backward-compatibility)
- [Host service API](#host-service-api)
- [RCC package contract](#rcc-package-contract)
- [Developer Mode and Developer Lab](#developer-mode-and-developer-lab)
- [Publisher trust](#publisher-trust-model)
- [Theme system](#theme-system)
- [Localization](#localization)
- [Save/lifecycle](#save-and-lifecycle)
- [Audio/haptics/input](#audio-haptics-and-input)
- [Security](#security-model)
- [Building](#building)
- [Release artifacts](#release-artifacts)
- [GitHub Actions](#github-actions)
- [Android signing](#android-signing-in-github-actions)
- [Source-available distribution](#source-available-distribution)
- [Validation and QA](#validation-and-qa)
- [Repository layout](#repository-layout)
- [Contributing and licensing](#contributing-and-licensing)

## v0.7.0 highlights

### Runtime and compatibility

- API version negotiation and capability discovery for modern packages.
- Missing `api_version` remains a valid legacy package signal.
- Canonical RCC layout (`prefix /`, host mount `/mods/<id>`) plus contained support for historical `/mods/<id>` RCC prefixes.
- Legacy context/service names remain available through game-scoped adapters.
- Existing settings may migrate on read into namespaced storage without rewriting gameplay/save formats.
- Native/L3 requests from local manifests never grant authority by themselves.

### Reliability

- Forced lifecycle saves on background/close paths.
- Lazy and bounded audio resources.
- Package install/update/uninstall model mutation races hardened.
- Empty game-state/statistics path handling hardened.
- Built-in card suits rendered without depending on Unicode card glyph font coverage.
- Cross-platform package scripts validate that expected artifacts actually exist.

### Developer ecosystem

- Developer Lab for local RCC validation and session-only test mounts.
- Diagnostics export with file/line/severity information.
- Device profile previews and package capability inspection.
- Expanded SDK documentation in `docs/sdk/`.
- Compatibility fixtures and CTest integration for real RCC mount tests when Qt is present.

### Security

- External QML runs in a separate engine and receives scoped service facades.
- Custom catalogs cannot mint YoungLion Official/Verified/Native-L3 trust.
- RCC namespace, entry path, size and identity validation before mount.
- Game-scoped Settings/Audio/Logger facades prevent legacy convenience APIs from becoming unrestricted host access.
- Native plugin loading is review/hash gated and disabled on mobile where appropriate.

## Built-in games

| Game | Type | Core interaction | Persistence |
| --- | --- | --- | --- |
| XOX | Board | Mouse/touch | Session/game stats |
| Blackjack | Card | Mouse/touch | Game state/stats |
| Minesweeper | Puzzle | Mouse/touch/flag | Game state/stats |
| 2048 | Puzzle | Keyboard/swipe | Game state/stats |
| Memory Match | Puzzle | Mouse/touch | Game state/stats |
| Reaction Tap | Reflex | Mouse/touch | Scores/stats |

Built-ins are compiled Qt plugins. Market/downloaded games are normally RCC/QML packages and use the external runtime rather than being promoted to native plugins.

## Architecture

```text
                           LeoMiniGames Shell
                                  |
             +--------------------+--------------------+
             |                    |                    |
       Built-in plugins      Installed RCC       Developer RCC
             |                    |                session-only
             |              validated mount            |
             +--------------------+--------------------+
                                  |
                         Game Host / Runtime
                                  |
    +----------------------------------------------------------------+
    | Theme | I18n | Save | Audio | Haptics | Input | Lifecycle      |
    | Stats | Achievements | Events | Random | Resources | Diagnostics|
    +----------------------------------------------------------------+
                                  |
                         game-scoped facades
                                  |
                         External QQmlEngine
```

The main application owns package installation, trusted publisher metadata, filesystem locations and global settings. External game QML does not receive those mutable internals directly.

## Runtime and content types

| Content | Typical format | Runtime trust | Notes |
| --- | --- | --- | --- |
| Built-in game | C++/Qt plugin | Host-shipped | Compiled with app |
| RCC game/mod | `.rcc` | Sandboxed/scoped | Preferred downloadable format |
| Theme | theme package/RCC | Data-driven | Host applies theme tokens |
| Developer RCC | local `.rcc` | Session-only | No native/L3 escalation |
| Native/L3 | reviewed native module | Explicit server/local trust | Higher-risk, separately authorized |

## Platform and architecture matrix

The application target set remains Windows, Linux, Android, iOS and macOS. GitHub CI deliberately tests more OS/toolchain/CPU combinations than the minimum release set.

| Platform | Architecture/toolchain | CI role | Release artifact |
| --- | --- | --- | --- |
| Windows | x86_64 / MSVC 2022 | build + CTest | portable ZIP + QtIFW EXE |
| Windows | x86_64 / LLVM-MinGW | build + CTest | compatibility lane |
| Windows | ARM64 / MSVC cross-build | build | portable ZIP + QtIFW EXE |
| Linux | Ubuntu 22.04 x86_64 | build + CTest | compatibility lane |
| Linux | Ubuntu 24.04 x86_64 | build + CTest | tar.gz + AppImage |
| Linux | Ubuntu 24.04 ARM64 | build + CTest | tar.gz + AppImage |
| macOS | Apple Silicon arm64 | build + CTest | ZIP + DMG |
| macOS | Intel x86_64 | build + CTest | ZIP + DMG |
| Android | arm64-v8a | APK build | signed APK + AAB lane |
| Android | armeabi-v7a | APK build | signed APK |
| Android | x86_64 | APK build | signed APK |
| Android | x86 | APK build | signed APK |
| iOS Simulator | arm64 | unsigned build | test ZIP |
| iOS Simulator | x86_64 | unsigned build | test ZIP |

A configured matrix is not the same as physical-device verification. Hardware QA is tracked separately in `GAMER_AUDIT.md` and `BUILD_MATRIX.md`.

## Backward compatibility

### Package generations

| Generation | Identification | Runtime behavior |
| --- | --- | --- |
| v0.5 legacy | no modern API metadata | compatibility adapters |
| v0.6 legacy | legacy capability/service conventions | compatibility adapters |
| v0.7 modern | `api_version` / `required_capabilities` | negotiated capabilities |

A package is **not** rejected simply because `api_version` is absent. Legacy generic `capabilities` are not retroactively reinterpreted as strict v0.7 requirements.

### Legacy RCC prefix

New packages should use RCC internal prefix `/`. The host mounts them under `/mods/<game_id>`.

Historical packages that already baked `/mods/<game_id>` into the RCC remain supported only when every resource is contained under that same namespace. A legacy package attempting to inject resources into host paths such as `/themes` or `/i18n` is rejected.

### Legacy services

Compatibility-facing names such as `Settings`, `Lang`, `Audio`, `App` and `Lifecycle` remain available. They are implemented as scoped adapters rather than raw host objects. This preserves common old calls while preventing one game from mutating another game's state or application-global security settings.

See [`docs/sdk/COMPATIBILITY.md`](docs/sdk/COMPATIBILITY.md) and [`COMPATIBILITY_REPORT.md`](COMPATIBILITY_REPORT.md).

## Host service API

| Service | Purpose | Persistence/security note |
| --- | --- | --- |
| `GameTheme` | semantic theme tokens | host-owned active theme |
| `GameI18n` | localized text/fallback/plurals | normalized locale paths |
| `GameSave` | game state slots | atomic/forced lifecycle save |
| `GameAudio` | named/package audio | game-scoped URL policy |
| `Haptics` | tactile feedback | capability/failure safe |
| `GameInput` | logical/physical input | keyboard-layout-aware WASD |
| `GameLifecycle` | pause/resume/save | host lifecycle bridge |
| `GameStats` | bounded statistics | per-game storage |
| `Achievements` | achievements/progress | per-game storage |
| `GameResources` | resource lookup | package namespace aware |
| `GameLogger` | diagnostics | scoped facade for external games |

Modern packages should use the documented SDK surface rather than discovering host QObject internals dynamically.

## RCC package contract

Canonical layout:

```text
RCC internal prefix: /
manifest.json
Main.qml
assets/
i18n/
```

At runtime an `example_game` package becomes:

```text
qrc:/mods/example_game/manifest.json
qrc:/mods/example_game/Main.qml
qrc:/mods/example_game/assets/...
qrc:/mods/example_game/i18n/...
```

Do **not** bake `/mods/<id>` into newly authored RCC packages. That is a backward-compatibility path, not the modern authoring contract.

A modern manifest can declare API requirements, but publisher verification/native authority never comes from self-declared manifest flags.

## Developer Mode and Developer Lab

Developer Lab is intended for local package iteration without publishing every build. It can inspect package ID/version/API, entry point, capabilities, mount mode, locale/save declarations and publisher/native requests before a session mount.

Local RCCs are copied to a session cache, are not inserted into the normal installed-content database and do not automatically remount after restart. Installed content with the same ID cannot be silently shadowed by a local developer package.

The current production integration supports scoped `lmg_...` developer credentials as a session-only fallback. Raw developer credentials are not intentionally persisted to QSettings/disk. A full browser-to-native-app OAuth callback remains a separate backend integration concern.

## Publisher trust model

Publisher status is a **trust property**, not a manifest preference.

| Badge | Meaning |
| --- | --- |
| Gold / Official | YoungLion-authoritative official publication |
| Green / Verified Native/L3 | verified publisher with explicit higher native permission |
| Blue / Verified | verified publisher |
| Unverified | no authoritative verification |

Legacy admin-only canonical catalog rows that predate modern publisher metadata are compatibility-mapped to Official. This fallback is restricted to the canonical YoungLion catalog boundary; a custom/third-party catalog cannot gain a Gold badge by copying old response fields.

## Theme system

The host owns one active theme. Games consume semantic aliases such as background, surface, text, border, accent, status, button and gameplay roles rather than maintaining a second theme engine. v0.7 adds canonical spacing, radius, touch-target, icon-size, typography, animation and effect tokens while retaining legacy aliases.

Runtime theme changes should propagate without forcing a game restart when the game uses the host facade correctly.

## Localization

The host normalizes locale values, supports deterministic fallback and propagates runtime language changes. English source strings are the canonical fallback. Missing translations are reported rather than filled with fake translated text.

Minimum ecosystem target languages include English, Türkçe, Azərbaycanca, Русский, Deutsch, Français, Español, Português, 中文 and 日本語; actual per-string coverage is reported by the validator.

```bash
python3 tools/validate_i18n.py
```

## Save and lifecycle

Game state and application settings are intentionally different concerns. Settings may use host settings storage; game save data goes through `GameSave` and atomic persistence paths.

Lifecycle events use forced-save paths where a normal dirty optimization could otherwise lose a just-completed action. Background/close behavior is therefore treated as a durability boundary rather than a cosmetic callback.

## Audio, haptics and input

Qt Multimedia is optional at configure time. The host exposes an audio capability/fallback rather than crashing when a platform/backend cannot provide sound. Dynamic effects are lazily allocated and bounded.

Android haptics use the Qt Android context and fail safely when vibrator capability is unavailable.

Keyboard input centralizes physical WASD handling so AZ/TR/DE/RU keyboard layouts are not expected to hard-code their own duplicated scan-code logic. Arrow keys remain a logical fallback. Mobile games should provide touch/swipe/drag controls appropriate to their genre.

## Security model

Security boundaries include:

- canonical-origin publisher trust;
- RCC ID/entry/path/size/hash validation;
- legacy namespace containment;
- isolated external QML engine;
- scoped Settings/Audio/Logger service facades;
- session-only Developer RCC mounts;
- no local-manifest native/L3 authority;
- bounded stats/achievement/save files;
- native plugin review/hash checks;
- mobile native-loading restrictions;
- explicit network capability for modern external packages.

Compatibility is not implemented by restoring unrestricted raw host QObject access.

See [`SECURITY.md`](SECURITY.md) and [`BUG_HUNTER_AUDIT.md`](BUG_HUNTER_AUDIT.md).

## Building

### Requirements

- CMake 3.21+
- C++20 compiler
- Qt 6.5 minimum
- Qt Core, Gui, Qml, Quick, QuickControls2, Network and Svg
- Qt Multimedia optional
- Ninja recommended where supported

Local development currently uses Qt 6.11.1 successfully. GitHub Actions intentionally uses Qt 6.10.2 because the current `install-qt-action`/aqt path has been more reliable against that public binary repository. **This is a CI tooling choice, not a downgrade of LeoMiniGames' Qt 6.11 support.**

### Desktop quick build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Platform-specific details are documented in [`BUILDING.md`](BUILDING.md), [`docs/BUILDING.md`](docs/BUILDING.md) and [`BUILD_MATRIX.md`](BUILD_MATRIX.md).

## Release artifacts

The manual **Build Release Artifacts** workflow creates assets for manual GitHub Releases. It deliberately does not create a release/tag automatically.

| Target | Artifact |
| --- | --- |
| Source | deterministic source ZIP + SHA-256 |
| Windows x86_64 | portable ZIP + QtIFW Setup EXE |
| Windows ARM64 | portable ZIP + QtIFW Setup EXE |
| Linux x86_64 | runtime tar.gz + AppImage |
| Linux ARM64 | runtime tar.gz + AppImage |
| macOS arm64 | ZIP + DMG |
| macOS x86_64 | ZIP + DMG |
| Android 4 ABIs | signed per-ABI APKs |
| Android arm64 | signed AAB lane |
| iOS simulators | unsigned arm64/x86_64 ZIPs |

Windows packaging runs `windeployqt` before creating the portable archive. The QtIFW application component is `ForcedInstallation` and `Essential` to prevent a maintenance-tool-only install.

## GitHub Actions

`ci.yml` is push/PR validation. It runs static validators first, then expands across desktop/mobile operating systems and architectures.

`build-artifacts.yml` is manual (`workflow_dispatch`) release asset generation. It requires the requested version to match source metadata. **There is no tag-triggered automatic GitHub Release creation.**

Recent CI fixes include invoking Android target `qt-cmake` through `bash` because some Android Qt archives can lose the executable permission bit, and removing an unnecessary `<QNativeInterface>` include from Android haptics—the Android application native interface is exposed through the Qt Core application header.

## Android signing in GitHub Actions

Never commit the `.jks`/`.keystore` file. Configure these GitHub Actions repository secrets:

```text
ANDROID_KEYSTORE_BASE64
ANDROID_KEY_ALIAS
ANDROID_KEYSTORE_PASSWORD
ANDROID_KEY_PASSWORD
```

The workflow decodes the keystore into the runner's temporary directory with restricted permissions, exports Qt signing variables, builds the artifact, and discards the ephemeral runner afterwards.

Keep the same upload/distribution key when application update compatibility requires signature continuity.

## Source-available distribution

LeoMiniGames host/core is licensed under **`LicenseRef-LMG-SAPEL-1.0`**, the LeoMiniGames Source-Available Publisher Ecosystem License 1.0 in [`LICENSE`](LICENSE). It is source-available, but it is **not** OSI Open Source or Free Software. Redistribution, mirrors, rebranded builds and unofficial releases are restricted by the license.

The repository retains a tracker-free/privacy-oriented Android build profile, but that profile is not an official F-Droid build. Current source-available editions are not eligible for the official F-Droid main repository. See [`docs/F_DROID_TRANSITION.md`](docs/F_DROID_TRANSITION.md).

Earlier public copies that were validly distributed under GPL terms retain the rights already granted for those copies; the new license does not retroactively revoke them. See [`LICENSE_HISTORY.md`](LICENSE_HISTORY.md) and [`MIGRATION_FROM_GPL.md`](MIGRATION_FROM_GPL.md).

## Validation and QA

Static validators:

```bash
python3 tools/source_guard.py
python3 tools/validate_project.py
python3 tools/sanity_check.py
python3 tools/security_audit.py
python3 tools/validate_i18n.py
python3 tools/validate_distribution.py
python3 tools/audit_prebuilt_binaries.py
python3 tools/validate_v070.py
```

Qt-enabled CTest adds compiled/runtime-oriented checks including RCC compatibility fixtures. Static PASS is not presented as physical-device verification. Gamer/QA reports explicitly distinguish code review, CI configuration and actual hardware testing.

## Repository layout

```text
.github/              Actions, issue/PR templates, CODEOWNERS
android/              Android package template/resources
docs/                 architecture/build/SDK documentation
fastlane/             Android store metadata and graphics
installer/             Qt Installer Framework configuration
licenses/              SDK and Publisher Package licensing material
mod-sdk/               RCC mod/game examples and SDK resources
plugins/builtin/       built-in native games
qml/                   application and reusable QML components
resources/             app resources/branding/audio
src/core/              host runtime/services
src/sdk/               public native SDK boundary
tests/                 CTest and compatibility fixtures
theme-sdk/             theme authoring resources
tools/                 validators and packaging helpers
```

## Contributing and licensing

LeoMiniGames host/core uses **`LicenseRef-LMG-SAPEL-1.0`**. Only files explicitly marked **`LicenseRef-YoungLion-LMG-SDK-1.0`** receive the separate developer SDK grant. Eligible independent Publisher Packages can use their own accepted license, a recognized open-source license, or **`LicenseRef-YoungLion-Publisher-Package-1.0`** where applicable.

Publisher trust, Official/Verified status and Native/L3 permission are separate from copyright licensing and remain backend-authoritative; package metadata cannot self-grant them. Publishing through YoungLion is additionally governed by [`docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`](docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md), and Native/L3 uses the separate addendum.

Contributions should follow [`CONTRIBUTING.md`](CONTRIBUTING.md) and [`CONTRIBUTOR_LICENSE_AGREEMENT.md`](CONTRIBUTOR_LICENSE_AGREEMENT.md). Third-party/Qt obligations remain separate; see [`docs/THIRD_PARTY_NOTICES.md`](docs/THIRD_PARTY_NOTICES.md) and [`docs/QT_LGPL_COMPLIANCE.md`](docs/QT_LGPL_COMPLIANCE.md). Security-sensitive issues should follow [`SECURITY.md`](SECURITY.md).

## Project links

- Website: <https://leominigames.younglion.xyz>
- Source: <https://github.com/YoungLionOrganization/LeoMiniGames>
- Issues: <https://github.com/YoungLionOrganization/LeoMiniGames/issues>
- Release documentation: [`RELEASES.md`](RELEASES.md)
- Compatibility report: [`COMPATIBILITY_REPORT.md`](COMPATIBILITY_REPORT.md)

---

LeoMiniGames aims to be more than a launcher that merely builds: the target is a stable, modular, backward-compatible and genuinely playable ecosystem across desktop and mobile platforms.


## Licensing architecture

- **Host/core:** `LicenseRef-LMG-SAPEL-1.0` (`LICENSE`)
- **Explicit SDK files:** `LicenseRef-YoungLion-LMG-SDK-1.0`
- **Optional proprietary Publisher Package license:** `LicenseRef-YoungLion-Publisher-Package-1.0`
- **Publisher service terms:** `docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`
- **Native/L3:** `docs/NATIVE_L3_PUBLISHER_ADDENDUM_1.0.md`
- **Historical grants:** documented in `LICENSE_HISTORY.md`; prior valid GPL grants are not retroactively revoked.

Qt and other third-party components remain governed by their own licenses.
