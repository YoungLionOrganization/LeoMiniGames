# Package: Client Integration

**Deploy target:** merge into the LeoMiniGames Qt 6 client/launcher target.

**Purpose:** parse additive server-owned publisher/trust/license/native metadata, preserve safe legacy fallback and cache an installed trust snapshot for offline UI.

**Compatibility:** existing download -> SHA-256 -> install -> `QResource::registerResource()` behavior remains unchanged. Manifest trust flags do not produce badges.

**Rollback:** remove the helper library/model fields and revert UI badge/license additions. Old catalog behavior remains valid.

**Smoke tests:** old catalog, official/verified/unverified/suspended, Level 1/2/3, offline snapshot roundtrip, hash mismatch path in existing installer.

**Executed here:** source prepared; Qt build/test not executed because Qt development binaries were unavailable in this runtime.
