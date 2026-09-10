# LeoMiniGames

[![CI](https://github.com/YoungLionOrganization/LeoMiniGames/actions/workflows/ci.yml/badge.svg)](https://github.com/YoungLionOrganization/LeoMiniGames/actions/workflows/ci.yml)
![Qt](https://img.shields.io/badge/Qt-6.11.1-41CD52?logo=qt&logoColor=white)
![License](https://img.shields.io/badge/license-GPL--3.0--or--later-blue)
![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20Android%20%7C%20iOS%20%7C%20macOS-lightgrey)

LeoMiniGames is a modular Qt 6 / Qt Quick game launcher and runtime. Built-in games, RCC games/mods and themes are kept separate from the host as far as practical, while host services provide theme, save, i18n, audio, haptics, input, lifecycle, statistics and developer tooling.

This source tree is the **v0.7.0** compatibility/security upgrade. Its primary rule is that supported v0.5/v0.6 RCC content must continue to run through compatibility adapters rather than being rejected for not using the new SDK.

## v0.7.0 highlights

- API version negotiation and capability discovery.
- Canonical RCC contract (`prefix /`, mounted at `/mods/<id>`) plus a contained legacy `/mods/<id>` compatibility mount.
- Isolated external `QQmlEngine` with game-scoped compatibility facades instead of raw host objects.
- Lazy/capability-probed audio and legacy audio overload compatibility.
- Atomic/forced lifecycle saves and legacy settings copy-on-read migration.
- Layout-independent physical WASD support through centralized `GameInput`; arrow keys remain available.
- Expanded theme/i18n runtime contracts and runtime language/theme propagation.
- Server-authoritative publisher trust boundary; local manifests cannot grant Official/Verified/Native-L3 status.
- Developer Mode / Developer Lab with session-only credential verification, local RCC validation, diagnostics and device profiles.
- Cross-platform CI configuration, packaging scripts, QtIFW Windows installer, F-Droid preparation and regression fixtures.

## Built-in games

The source currently registers: 2048, Blackjack, Memory Match, Minesweeper, Reaction Tap and XOX. External RCC content appears through the same host runtime without becoming a native plugin by default.

## Architecture

```text
                    LeoMiniGames shell
                           |
       +-------------------+-------------------+
       |                   |                   |
 Built-in Qt plugins   Installed RCC       Developer RCC
       |                   |                   |
       |              validated mount      session-only mount
       |                   |                   |
       +----------- Game host/runtime --------+
                           |
       +-------------------+-----------------------------+
       | Theme | I18n | Save | Audio | Input | Lifecycle |
       | Stats | Achievements | Events | Random | Haptics |
       +-------------------------------------------------+
                           |
                 game-scoped facades
```

External QML is loaded in a separate engine. Compatibility names such as `Settings`, `Lang`, `Audio`, `App` and `Lifecycle` remain available, but they are adapters scoped to the active game and do not expose application paths, package management or unrestricted filesystem access.

## Backward compatibility

v0.7.0 supports three package paths:

1. **v0.5/v0.6 legacy** — missing `api_version` is treated as legacy and receives compatibility services.
2. **Legacy RCC prefix** — `/mods/<id>` inside the RCC remains mountable only when every resource stays under that same package namespace.
3. **v0.7 modern** — explicit `api_version`, `min_api_version` and `required_capabilities` are negotiated.

Legacy generic `capabilities` are not retroactively interpreted as hard v0.7 requirements. Existing settings can be migrated on read into game-scoped keys. Gameplay/save representations are not rewritten merely for v0.7.

Regression fixtures live in `tests/fixtures/compat/`. With Qt available, CTest builds real RCCs for canonical v0.5, legacy-prefix v0.5, v0.6 and v0.7 packages, plus a malicious namespace fixture that must be rejected.

See `docs/sdk/COMPATIBILITY.md` and `COMPATIBILITY_REPORT.md`.

## Build

Requirements:

- CMake 3.21+
- C++20 compiler
- Qt 6.11.1 recommended (minimum host requirement in CMake remains Qt 6.5)
- Qt Core, Gui, Qml, Quick, QuickControls2, Network and Svg
- Qt Multimedia is optional and capability-probed
- Ninja is recommended

Typical desktop build:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

More platform detail is in `BUILDING.md`, `docs/BUILDING.md` and `BUILD_MATRIX.md`.

## Distribution helpers

- Windows: `tools/package/package_windows.ps1 <build-dir>` — `windeployqt` portable ZIP plus Qt Installer Framework setup.
- Linux: `tools/package/package_linux.sh <build-dir>` — linuxdeploy-backed portable AppDir archive plus AppImage.
- macOS: `tools/package/package_macos.sh <build-dir>` — `macdeployqt`, ZIP and DMG; signing/notarization is not faked.
- Android: `tools/package/package_android.sh <build-dir>` — APK and AAB targets when supported by the Qt Android toolchain.
- iOS: `tools/package/package_ios.sh <build-dir>` — unsigned/test app archive when signing credentials are unavailable.

The Windows QtIFW application component is `ForcedInstallation` and `Essential`; the installer cannot intentionally install only the maintenance tool while omitting LeoMiniGames.

## Mod / game SDK

SDK documentation is under `docs/sdk/`:

- `OVERVIEW.md`
- `MANIFEST.md`
- `RESOURCES.md`
- `THEME_API.md`
- `I18N_API.md`
- `SAVE_API.md`
- `AUDIO_API.md`
- `INPUT_API.md`
- `LIFECYCLE_API.md`
- `DEVELOPER_MODE.md`
- `COMPATIBILITY.md`
- `MIGRATION_0.6_TO_0.7.md`

Examples are under `mod-sdk/`.

### Canonical RCC layout

```text
RCC internal prefix: /
manifest.json
Main.qml
assets/...
i18n/...
```

The host mounts this resource at `/mods/<game_id>`, producing URLs such as:

```text
qrc:/mods/example_game/Main.qml
qrc:/mods/example_game/assets/sfx/hit.wav
```

Do not bake `/mods/<id>` into new RCCs. That layout is supported only as a legacy compatibility path.

## Developer Mode

Developer Lab can validate and session-mount a local RCC without publishing it. It reports package ID/version/API, capabilities, entry, mount mode, locales, save version, publisher status and whether native/L3 was requested. Local manifests never receive native/L3 authority.

The supplied backend currently supports the existing scoped `lmg_...` developer credential path used by this client as a session-only fallback. The raw credential is not persisted to QSettings/disk. Full native browser-to-app OAuth callback support remains a backend integration dependency and is not fabricated inside the client.

## Theme system

`GameTheme` exposes a stable game-facing facade while the host theme runtime owns the active theme. v0.7 adds canonical semantic tokens for background/surface/text/brand/status/buttons/game cells plus spacing, radius, touch targets, icons, typography, animation and effects. Existing aliases remain available through compatibility mappings.

## Localization

Host locale normalization handles language/region variants and falls back deterministically. English source coverage is complete. Other locales intentionally fall back to source English when a translation is missing; the validator reports coverage instead of silently filling untranslated strings with fake translations.

Run:

```bash
python3 tools/validate_i18n.py
```

## Security model

- Publisher trust comes from the canonical YoungLion catalog boundary, not `manifest.json`.
- Custom catalogs may distribute content but cannot mint YoungLion Official/Verified/Native-L3 trust.
- RCC IDs, entry paths, sizes, hashes and namespace layout are validated before mount.
- External QML receives scoped service facades rather than application-internal mutable objects.
- Modern v0.7 network access is capability-gated; legacy installed RCC content retains its historical HTTPS behavior for compatibility.
- Developer local RCC network access is disabled.
- Native plugins require an explicit reviewed local trust snapshot and SHA-256 match; native dynamic loading is disabled on Android/iOS.
- Save/stat/achievement files use bounded parsing/writes and atomic save paths where applicable.

See `SECURITY.md` and `BUG_HUNTER_AUDIT.md`.

## Validation

Static validation entry points:

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

Qt runtime/physical-device tests are separate from static validation. A validator PASS must not be interpreted as proof of Android/iOS/macOS hardware behavior.

## F-Droid

Upstream Fastlane metadata and an fdroiddata recipe template are included. The template remains disabled-by-convention with `REPO_URL` / `FULL_COMMIT_SHA` placeholders until the canonical public repository and immutable full commit hash exist. No proprietary SDK is introduced by the F-Droid build option, and no fake screenshots are included.

See `F_DROID_READINESS.md` and `fdroid/README.md`.

## CI and build artifacts

`.github/workflows/ci.yml` runs static validators plus Linux, Windows, macOS arm64/x86_64, Android and unsigned iOS-simulator builds/tests. CI intentionally uses Qt 6.10.2 while `aqtinstall` catches up with the changed Qt 6.11 repository metadata; local Qt 6.11.1 remains supported and recommended.

`.github/workflows/build-artifacts.yml` is a **manual** Actions workflow that produces source ZIPs, Windows portable ZIP + QtIFW Setup EXE, Linux archive + AppImage, macOS ZIP + DMG, signed Android APK/AAB (when repository signing secrets are configured), and an unsigned iOS simulator ZIP. It does not create a GitHub Release automatically. See `docs/GITHUB_RELEASES.md`.

## Contributing and security

See `CONTRIBUTING.md`. Security-sensitive issues should follow `SECURITY.md`. Do not report a mod as trusted because its local manifest says `official`, `verified`, `reviewed`, `native` or `plugin_level: 3`.

## License

LeoMiniGames host source is licensed under **GPL-3.0-or-later**. The standard GNU license text is kept unmodified in `LICENSE`; project copyright and ecosystem notices are in `COPYRIGHT` and `NOTICE`. Games/mods/themes must declare their own license, and developer verification/Official/Native-L3 permissions are separate from copyright licensing. See `docs/DEVELOPER_LICENSING.md` and `licenses/README.md`.
