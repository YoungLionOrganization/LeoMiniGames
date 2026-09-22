# Changelog

## 0.7.1 — Release, Update Gateway and Developer Lab Servicing

### Fixed
- Developer Lab no longer validates API keys by calling the publisher content-list endpoint. It uses the dedicated backend `GET /api/v1/developer/auth/verify` contract, preventing valid keys from being rejected because onboarding or `content:read` publication gates are unrelated to local RCC testing.
- Developer Lab sends the current application version in its User-Agent and validates an explicit `authenticated/key/developer` response contract while preserving redirect, timeout and response-size protections.
- Android CI/release packaging now retries only recognized transient Gradle distribution/network failures. This addresses the real CI #24 x86_64 failure where the Gradle distribution host returned HTTP 500 without hiding genuine compilation/package failures.
- Publish Release continues to require an exact-SHA successful CI run. Publish #4 correctly refused publication after CI #24 failed, rather than bypassing the failed release gate.

### Backend integration
- Added a dedicated Developer Lab API-key verification endpoint and cPanel/FastCGI-safe Authorization-header extraction.
- Application update metadata/downloads remain routed through `leominigames.younglion.xyz`; GitHub remains a replaceable backend provider rather than a client dependency.

## 0.7.1 — Release, Maintenance and Update Infrastructure

### Release engineering
- Added canonical release metadata under `release/`, a strict public asset allowlist and deterministic `release-assets` aggregation.
- Added the guarded manual `Publish Release` workflow with separate validation/publish modes, exact-`main`-SHA CI/build gates, authorized publisher checking, duplicate tag/release protection and protected `release` Environment approval.
- Publishing now uses a draft-first transaction: validated assets are uploaded and compared exactly before the QtIFW repository is published and the draft is made public.
- Added rollback protection for the `updates` branch if final GitHub Release publication fails after the QtIFW repository push.

### Installer and updates
- Windows QtIFW packages are hybrid installers and generate update repositories for x86_64 and ARM64.
- Added `LeoMiniGamesMaintenance` integration for update, repair/modify and uninstall flows.
- Added `UpdateService` with Stable/Preview channels, SemVer prerelease comparison, bounded HTTPS GitHub metadata handling and installer/portable-aware update behavior.
- Fixed Apple mobile builds by keeping the Windows-only `QProcess::startDetached()` Maintenance Tool launch path out of iOS/iPadOS compilation.

### Validation / compatibility
- Added `validate_v071.py` release/update regression checks.
- Preserved the v0.5/v0.6/v0.7 game/mod compatibility contract; no v0.7.1 game API migration is required.
- Save Transfer and the portable `.lmgsave` exchange format remain deferred to v0.8.0.


## 0.7.0 — Compatibility, SDK, Security and Distribution

### GitHub / release engineering
- Fixed the current Android CI failures: Android `qt-cmake` wrappers are invoked through `bash` to avoid lost executable bits in extracted kits, and the Haptics Android native-interface include now follows the Qt Core header contract.
- Expanded CI to Ubuntu 22.04/24.04 x86_64, Ubuntu 24.04 ARM64, Windows MSVC x86_64, Windows LLVM-MinGW x86_64, Windows ARM64 cross-build, macOS arm64/x86_64, all four Qt Android ABIs, and iOS simulator arm64/x86_64.
- Expanded manual release artifacts to Windows x86_64/ARM64, Linux x86_64/ARM64, macOS arm64/x86_64, per-ABI Android APKs, an arm64 AAB lane, and unsigned iOS simulator archives.
- Linux AppImage packaging now uses the maintained AppImage/appimagetool repository plus linuxdeploy-plugin-qt so Qt/QML resources are actually deployed.
- Corrected F-Droid upstream metadata to the real YoungLionOrganization repository and documented the existing fdroiddata Qt 6 source-build precedent (`Qt5@v6.10.1`).
- Reworked CI to use a verified Qt 6.10.2 compatibility lane while aqtinstall 3.3.x cannot reliably consume the changed Qt 6.11 online repository metadata; local Qt 6.11.1 support is unchanged.
- Added a manual Build Release Artifacts workflow producing direct-download source ZIP, Windows portable ZIP + QtIFW installer, Linux archive + AppImage, macOS ZIP + DMG, signed Android APK/AAB, and unsigned iOS simulator ZIP.
- Added Android signing through repository secrets without committing keystore material.
- Added CODEOWNERS, issue forms, pull-request template, Dependabot GitHub Actions updates and repository text/binary normalization.
- Added project copyright/notice files and explicit developer/package licensing guidance while keeping the standard GPL license text verbatim.

### New
- Added explicit game API version negotiation and capability discovery.
- Added Developer Mode / Developer Lab local RCC validation, diagnostics and device profiles.
- Added v0.5/v0.6/v0.7 compatibility fixtures and real-RCC CTest targets for Qt-enabled CI.
- Added cross-platform CI configuration, packaging helpers, F-Droid metadata/template and distribution validation.

### Changed
- External RCC QML now runs in a separate QQmlEngine with game-scoped service facades.
- Qt Multimedia is optional/capability-probed and audio allocation is lazy/cache-bounded.
- Modern v0.7 network access is capability-gated; legacy installed RCC HTTPS behavior is retained.
- Theme and i18n service contracts expose explicit ready/version/capability behavior.

### Fixed
- Fixed Qt 6.11 build errors reported by a real Windows LLVM-MinGW build: removed duplicate `qt_finalize_executable(LeoMiniGames)`, corrected `GameAudio` `releasePrefix()` argument type, and switched the external-runtime factory include to the actual Qt `QQmlNetworkAccessManagerFactory` header.
- Fixed `ExternalGameRuntime` network-factory lifetime: `QQmlEngine` does not own the factory, so the runtime now retains it until engine teardown instead of leaking it.
- Reduced `DeveloperLabPage.qml` unqualified-access warnings with `pragma ComponentBehavior: Bound`, required service properties and root-qualified service access.
- Fixed Android haptics portability by relying on the Qt Core application header for `QNativeInterface::QAndroidApplication` instead of a standalone `QNativeInterface` include that failed in the Qt 6.10 Android kit.
- Fixed `DeveloperLabPage.qml` runtime-diagnostics action row syntax that caused Qt 6.11 `qmlcachegen` to fail with `Unexpected token `;``.
- Fixed lifecycle paths that could skip a required disk save; close/background/quit now use force-save safety paths.
- Fixed layout-dependent direct WASD handling by routing through centralized physical/native GameInput mappings.
- Fixed RCC canonical/legacy double-prefix ambiguity and blocked legacy namespace injection.
- Fixed Mod Market model row races between catalog refresh/uninstall and resolving/downloading/installing callbacks.
- Fixed empty stats/achievements paths that could trigger QFSFileEngine empty-file warnings on game close.
- Replaced built-in playing-card suit font glyph rendering with vector Canvas rendering while preserving legacy card/save representation.
- Fixed Linux portable packaging so the archive contains a linuxdeploy-produced runtime rather than only the executable.

### Compatibility
- Missing api_version remains accepted as legacy v0.5/v0.6.
- Legacy /mods/<id> RCC prefixes remain supported when namespace-contained.
- Legacy Settings, Lang, Audio, App, Lifecycle and related game-facing call shapes remain available through adapters.
- Legacy unnamespaced settings use safe copy-on-read migration to game-scoped storage.
- Old generic capabilities are not reinterpreted as mandatory v0.7 required_capabilities.

### Security
- Local manifests cannot grant Official/Verified/Native-L3 authority.
- Custom catalogs cannot mint YoungLion trust badges.
- Developer API credentials are validated session-only with redirect and response-size restrictions.
- Developer local RCC network access is disabled; external logger/audio/filesystem surfaces are narrowed.
- Native plugin trust snapshot/library sizes are bounded and dynamic loading is disabled on Android/iOS.
- Stats/achievements parsing and cardinality are bounded against resource exhaustion.

### Localization
- Host source English coverage is complete; missing non-English translations use deterministic source fallback and are reported as coverage warnings.

### Known limitations
- This delivery environment has no Qt 6 SDK, so native compilation, qmllint, CTest RCC execution and physical-device testing are not claimed here.
- Real F-Droid screenshots and an immutable public-repository commit SHA are still required before an fdroiddata submission.
- Direct native browser-to-app OAuth callback support depends on backend work; the client keeps the existing scoped developer-key session fallback.

## v0.6.2 Theme Runtime Hotfix

- Fixed missing semantic Constants properties that produced `undefined` numeric/color assignments at runtime.
- Guarded optional catalog/icon/avatar image sources so empty metadata no longer opens `qrc:/.../undefined` or empty file paths.
- Sanitized ModCard progress values before assigning geometry.
- Removed the inherited `enabled` property shadow in `PressableSurface`.
- Replaced application QML direct Bronze/Espresso base-palette usage with semantic aliases so external themes affect the entire shell and built-in games consistently.
- Updated the legacy `Theme.qml` facade to resolve through active semantic theme values.
- Replaced Qt 6.11 deprecated proxy filter invalidation with `beginFilterChange()/endFilterChange()`.
- Includes the build fixes for `QuickShapes`, `AutoFitText.minimumPixelSize`, and `ThemeInstalledModel::countChanged`.


## 0.6.2 — Theme Market & Semantic UI Runtime

- Expanded the theme system to 300+ base colors, 1,600+ tokens and 5,000+ aliases covering colors, geometry, typography, motion, scale, opacity, elevation, touch/layout and game presentation.
- Added protected primitive metrics and `GameRules` separation so external themes cannot change gameplay/application logic.
- Added a reusable QML component library for themed controls, overlays, layout helpers, game HUD/tiles/inventory, virtual controls and market UI.
- Added online Theme Market Explore/Installed flows with search, category filtering, sorting, verified HTTPS download tickets, SHA-256/size validation, update/reinstall and local/sideloaded theme visibility.
- Added explicit single-active-theme behavior. Active themes are highlighted and cannot be removed from the Installed UI; the built-in fallback cannot be removed.
- Added Library-level Theme Market access while keeping themes out of the game registry/main game list.
- Fixed external theme replacement/update and fallback texture resource-root handling.

## 0.6.1 — Theme Runtime & Expanded Game API

- Added dynamic `ThemeManager`, separate RCC theme packages and installed-theme management.
- Added Qt Design Studio-style `Constants.qml` with 320 base colors, 1,200+ semantic component/state color aliases, and centralized metrics/scales/radii/timing/unit tokens.
- Added `ThemeSurface` for theme-controlled solid/gradient/texture surfaces used by the application and built-in games.
- Kept `Theme.qml` as a compatibility facade.
- Added `GameTheme`, deterministic `GameRandom`, lifecycle-aware `GameClock` and `GameEvents`.
- Added theme-market client boundary without changing backend endpoints.
- Added plugin-level practical capability guidance.

## 0.6.0 - 2026-08-29

### Plugin/runtime architecture
- Added `GameRegistry`, combining built-in Qt plugins, installed legacy/modern RCC games, and reviewed trusted-native plugins into one game model.
- Preserved RCC v1 package layout and legacy manifest compatibility; all v0.6 manifest fields are optional.
- Installed RCC games appear in the main library immediately after install and disappear immediately after removal.
- Added library search across name, localized name, category, tags, and publisher.
- Added Explore/Installed mod models with search, category filtering and sorting.
- Added backend-metadata placeholders for publisher verification, review status, plugin level, native status, license and update metadata.
- Level 3 trust is not granted from manifests. Desktop native loading requires a reviewed local trust snapshot plus SHA-256 match.
- RCC install/reinstall/update now uses rollback so a failed replacement can restore the previous package.

### Game APIs
- Added namespaced `GameSettings` backed by QSettings strictly for preferences/configuration.
- Added `.lmgsave` CBOR saves with header metadata, SHA-256 integrity, QSaveFile atomic commit, backup restore, slots, autosave and JavaScript migration callbacks.
- Added lifecycle, action input, safe viewport, game audio, haptics, per-plugin i18n, statistics, achievements and plugin diagnostics services.
- Android background state triggers pause/background/save/autosave/audio pause; foreground resumes lifecycle/audio.

### Data migration
- Built-in persistent scores/statistics moved from QSettings to `saves/<gameId>/local_stats.cbor`.
- Existing v0.5.x statistic keys are migrated once and removed only after successful CBOR persistence.

### Licensing / distribution
- Application license changed to `GPL-3.0-or-later`.
- Adopted `LicenseRef-YoungLion-Mod-License-1.0` for eligible proprietary packages and the LeoMiniGames Plugin / Mod Exception v1.0 as a GPLv3 section 7 additional permission for qualifying independent public-interface packages. The canonical GPLv3 text remains unchanged.
- Added F-Droid/source-build documentation and `LEOMINIGAMES_FDROID` CMake option.
- Added manifest JSON Schema and a modern RCC v1 SDK example.
### Build hotfix 3
- Fixed undefined linker symbol `GameResources::GameResources(QObject*)` by adding the missing constructor definition.
- Extended `source_guard.py` to detect QObject classes that declare out-of-line constructors without matching definitions.


### Publisher trust hotfix
- Restored Gold **Official Publisher** provenance for packages published through the historical admin-key-only `/api/v1/mods` and `/api/v1/themes` catalogs when those legacy rows predate v0.7 publisher trust metadata.
- Added centralized `PublisherTrustResolver` shared by Mod and Theme catalogs. Modern explicit `publisher.type` / `publisher_type` always wins, so verified/unverified community publishers are not accidentally promoted.
- Third-party catalog origins remain fail-closed and cannot gain YoungLion Official/Verified trust through the legacy fallback.
- Added a CTest regression for legacy admin Mod/Theme, modern verified/unverified publishers, scalar Official provenance, and third-party spoof attempts.
