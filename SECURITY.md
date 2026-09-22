# Security Policy

## Scope and trust boundaries

LeoMiniGames treats external RCC QML/JS as untrusted application content. It is not treated as reviewed native code simply because a manifest requests a higher plugin level.

External packages are inspected before mount and execute in a separate `QQmlEngine` with game-scoped facades. They do not receive raw application paths, plugin/package managers, unrestricted diagnostics export or arbitrary filesystem audio access.

A separate QML engine is a privilege-reduction boundary, **not a process sandbox**. Hostile QML can still consume process CPU/memory; true hostile-code containment would require a process/platform sandbox.

## RCC packages

Canonical packages use internal resource prefix `/` and are mounted at `/mods/<id>`. Legacy RCCs with internal `/mods/<id>` layout remain supported only when all RCC resources are contained in that same package namespace. This prevents compatibility mounting from injecting host namespaces such as `/themes`.

Package IDs, entry paths, sizes and SHA-256 metadata are validated before activation. Replacement installation uses rollback to retain the previous working package when activation fails.

## Publisher trust

`official`, `verified`, `reviewed`, `native`, `plugin_level` or similar values inside `manifest.json` are not authoritative.

YoungLion publisher verification is accepted only from the canonical YoungLion catalog trust boundary. A custom catalog can provide downloadable content but cannot mint a YoungLion Official/Verified/Native-L3 badge.

Installed trust is refreshed from authoritative catalog metadata rather than treating old client-side cache fields as permanent authority.

## Native / L3 plugins

Native plugins are a separate high-trust path and are not loaded from an RCC manifest. The current loader requires a local reviewed trust snapshot, verified-publisher indication, correct Qt/API metadata and an exact SHA-256 match. Trust-snapshot parsing and native-library hashing have size limits.

Dynamic native plugin loading is disabled on Android and iOS. Signing/provenance fields beyond the current reviewed local snapshot remain future protocol work; do not describe the existing local trust file as a remote cryptographic publisher signature.

## Developer Mode

Local Developer RCC import requires an authenticated developer session. The current compatible path validates a scoped `lmg_...` credential against the dedicated canonical `/api/v1/developer/auth/verify` endpoint; content-listing permissions are not used as a proxy for authentication. The raw credential is used for that request only and is not persisted by `DeveloperManager` to QSettings or application files.

Authentication redirects are refused, responses are bounded and local RCC files are copied to a hashed session cache and re-hashed before mount. Installed/built-in ID collisions are rejected. Local manifests never receive Native/L3 authority, and Developer-local RCC network access is disabled.

## Network

Catalog/ticket requests use HTTPS, transfer timeouts, bounded responses and redirect policies. Download ticket endpoints must remain on the configured catalog origin. Download artifacts may use HTTPS CDN URLs, but size and SHA-256 integrity are verified before installation.

Modern v0.7 external games require a declared network capability for HTTPS runtime access. Installed legacy v0.5/v0.6 RCC games retain their historical HTTPS behavior for backward compatibility. External network access does not grant local filesystem access.

## Save and state

GameSave uses a bounded CBOR payload, SHA-256 checksum and `QSaveFile` atomic commit with backup recovery. Empty game IDs fail closed instead of deriving root/current-directory save paths. Lifecycle close/background/quit paths use force-save safety commits.

GameStats and Achievements use bounded CBOR files, bounded IDs and entry counts to limit disk/memory abuse while preserving normal legacy identifiers.

## Audio

Qt Multimedia is capability-probed. Failure to initialize audio is non-fatal. Audio objects are lazy/cache-bounded. External packages can access game-scoped RCC audio; arbitrary filesystem/network audio and global host cache mutation are not part of the external API.

## Reporting

When reporting a security issue, include the LeoMiniGames version, platform, package ID/version and relevant diagnostics, but remove access tokens, API keys, authorization headers, private save data and personal account information. Avoid publishing an immediately exploitable vulnerability before a fix is available.
