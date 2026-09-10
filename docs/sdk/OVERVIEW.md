# LeoMiniGames v0.7 SDK Overview

LeoMiniGames hosts built-in C++ plugins and external `rcc-v1` QML packages. v0.7 keeps the v0.5/v0.6 context-service contract through compatibility facades while adding capability discovery and stricter resource isolation.

## Runtime model

External packages are inspected before mount and then loaded in a dedicated `QQmlEngine`. The package receives host services, not raw application internals. The stable context names include `App`, `Settings`, `Lang`, `Audio`, `GameTheme`, `Lifecycle`, `GameSettings`, `GameSave`, `GameInput`, `GameI18n`, `GameAudio`, `GameResources`, `GameRuntime`, `GameStats`, `Achievements`, `Haptics`, `GameEvents`, `GameClock`, `Viewport`, and `GameLogger`.

Missing `api_version` means legacy compatibility mode. New packages should declare `api_version: "0.7"` and only put true hard requirements in `required_capabilities`.

## Compatibility rule

A v0.5/v0.6 package is not rejected merely because it lacks v0.7 fields. Canonical RCC layout is preferred, legacy `/mods/<id>` layout is accepted when the RCC contains no resources outside that namespace. Security-sensitive publisher/native trust never comes from `manifest.json`.

## Network

`GameRuntime.capabilities()` advertises `network.https`. Modern v0.7 installed packages opt into runtime HTTPS; local Developer RCC remains network-disabled. This permission model is additive and does not retroactively reject legacy v0.5/v0.6 packages.
