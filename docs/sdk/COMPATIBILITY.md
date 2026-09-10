# Backward Compatibility

v0.7 supports three paths:

1. **v0.5/v0.6 legacy package** — missing `api_version` is accepted; legacy context names are provided by compatibility facades.
2. **Legacy RCC prefix** — `/mods/<id>` internal layout is accepted only when namespace-contained.
3. **v0.7 package** — explicit API/capability negotiation is used.

Compatibility facades preserve `Settings`, `Lang`, `Audio`, `App`, `GameTheme`, and `Lifecycle` call shapes while restricting access to host internals. `GameSave`, `GameSettings`, `GameInput`, and other newer services remain available.

Known intentional security boundary: external packages no longer receive raw application paths, plugin manager, package install/remove APIs, arbitrary filesystem audio, or lifecycle re-attachment. A legacy package depending on those private/unsafe internals was never part of the supported mod contract and is reported with a readable incompatibility diagnostic rather than silently granting host privilege.

Regression sources live under `tests/fixtures/compat/`.
