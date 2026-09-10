# LeoMiniGames Releases

## v0.7.0

v0.7.0 is a compatibility-first runtime and developer-ecosystem upgrade. It does not intentionally require v0.5/v0.6 RCC games to be repackaged.

### New
- API version/capability negotiation.
- Developer Lab local RCC validation and diagnostics.
- Cross-platform CI/build matrix and packaging validation.
- Expanded SDK documentation and regression corpus.

### Changed
- External QML is isolated in a separate engine and receives game-scoped facades.
- Audio is lazy and optional-backend aware.
- Modern packages opt into network capability; legacy installed packages retain compatible HTTPS behavior.
- Theme/i18n contracts are more explicit and runtime-reactive.

### Fixed
- Lifecycle force-save gaps.
- Alternate-keyboard-layout WASD handling.
- RCC double-prefix/namespace injection paths.
- Market refresh/install/uninstall callback races.
- Empty stats/achievement file-path warnings.
- Font-dependent built-in card suit rendering.
- Linux "portable" archive missing its Qt runtime.

### Compatibility
A missing `api_version` means legacy. Canonical v0.7 RCCs use internal prefix `/`; legacy `/mods/<id>` resources are still accepted when the RCC is fully contained in its own namespace. Legacy game-facing service names remain available through adapters.

See `COMPATIBILITY_REPORT.md` and `docs/sdk/MIGRATION_0.6_TO_0.7.md`.

### Developer API
Modern packages may declare `api_version`, `min_api_version` and `required_capabilities`. Unsupported required capabilities produce a readable compatibility error. Local developer packages are never granted native/L3 authority merely from their manifest.

### Theme API
v0.7 adds canonical semantic game/UI aliases without removing the existing alias surface. Runtime theme switching remains host-owned.

### Localization
English source coverage is complete. Other locale gaps fall back safely and are explicitly reported; untranslated text is not fabricated to obtain a false parity PASS.

### Platform / Build
CI is configured for Windows x86_64, Linux x86_64, macOS arm64/x86_64, Android arm64-v8a/armeabi-v7a and unsigned iOS simulator builds. Packaging helpers cover Windows ZIP/QtIFW, Linux runtime archive/AppImage, macOS ZIP/DMG, Android APK/AAB and unsigned iOS test archive.

### Security
Publisher verification is server-authoritative at the canonical YoungLion catalog boundary. External QML no longer receives raw host internals. RCC namespace, path, size/hash, developer credential, network, native plugin, diagnostics and bounded-state checks were hardened.

### Known limitations
- Qt SDK/runtime and physical devices are unavailable in the delivery environment; those tests are marked NOT EXECUTED rather than PASS.
- F-Droid submission still needs the canonical public repository, exact full source commit hash and real application screenshots.
- Direct native browser-to-app OAuth callback flow remains a backend dependency; session-only scoped developer credentials remain the compatible client fallback.

### Section 22 exclusion
No tag-triggered automatic GitHub Release workflow is included. This is intentional per the v0.7.0 task constraint.
