# LeoMiniGames v0.7.0 Compatibility Report

Date: 2026-09-10

## Compatibility policy

v0.7.0 is additive. A supported v0.5/v0.6 RCC package must not be rejected simply because it does not declare v0.7 fields. Compatibility is implemented by negotiation/adapters, not by granting legacy packages unrestricted host access.

## Package compatibility

| Package class | v0.7 behavior | Status |
| --- | --- | --- |
| v0.5/v0.6, no `api_version` | treated as legacy compatibility mode | PASS (static contract) |
| canonical RCC internal prefix `/` | mounted at `/mods/<id>` | PASS (static contract) |
| legacy RCC internal prefix `/mods/<id>` | compatibility mount if fully namespace-contained | PASS (static contract) |
| malicious legacy RCC with extra host namespace | rejected | PASS (fixture present/static contract) |
| v0.7 modern manifest | API/min API/required capabilities negotiated | PASS (unit test source present) |
| unknown hard required capability | readable incompatibility result | PASS (unit test source present) |

Real RCC generation/mount execution is configured in CTest but is **NOT EXECUTED in this delivery environment**, because Qt 6/rcc is not installed here.

## Preserved legacy context names

The external runtime preserves game-facing names including `App`, `Settings`, `Lang`, `Audio`, `GameTheme`, `Lifecycle`, `GameSettings`, `GameSave`, `GameInput`, `GameRandom`, `GameEvents`, `GameClock`, `Viewport`, `GameI18n`, `GameStats`, `Achievements`, `GameAudio`, `Haptics`, `GameLogger`, `GameRuntime` and `GameResources`.

Legacy objects are facades. They retain supported call shapes while restricting host-global mutation, arbitrary filesystem access and package-management privilege.

## Settings migration

Legacy unnamespaced settings remain readable. v0.7 checks the new game-scoped key first; if absent, a non-reserved legacy key can be copied into `compat/mods/<gameId>/<key>`. New writes use the game namespace. Host/security-reserved settings are not exposed through this migration.

## Audio compatibility

Legacy `Audio.play`, `playUrl`, `preload`, `preload(url, gain)` and `stopAll` call shapes remain available. `GameAudio` also retains the historical two-argument preload overload. Named effect identifiers are not misrouted to file decoders.

## Input compatibility

Arrow keys remain logical actions. WASD is centralized in `GameInput` with native/physical key mapping rather than requiring every game to maintain scan-code tables. Direct `Qt.Key_W/A/S/D` gameplay checks were not found in the current source scan.

## Save compatibility

v0.7 does not force-convert legacy gameplay representations. Atomic `GameSave` behavior is retained, and lifecycle close/background/quit paths use force-save safety commits. Legacy settings remain separate from gameplay saves.

## Theme/i18n compatibility

Existing theme aliases are retained while canonical semantic aliases are added. Missing i18n translations use deterministic source-text fallback. Locale normalization does not require old package locales to be renamed before use.

## Network compatibility

New v0.7 packages need the `network` or `network.https` capability for external HTTPS access. Installed legacy packages with no v0.7 API declaration retain their historical HTTPS behavior. Developer-local RCC network access remains disabled as a deliberate security boundary.

## Regression corpus

`tests/fixtures/compat/` contains:

- `v0_5_canonical`
- `v0_5_legacy_prefix`
- `v0_6_services`
- `v0_7_modern`
- `malicious_legacy_namespace`

`tests/CMakeLists.txt` converts them to real RCCs with Qt `rcc --binary` and executes `test_rcc_compat` when Qt is available.

## Intentional non-compatibility boundaries

Backward compatibility does not restore private/unsafe host access that was not a supported game API. External packages are not given raw application paths, package install/remove managers, arbitrary filesystem audio, diagnostic file export or self-declared native/L3 trust.

## Result

Static compatibility contracts and fixtures pass. Qt-enabled CTest and physical gameplay regression must still be executed on a machine/CI runner containing the required Qt toolchains before a production release is declared fully runtime-verified.
