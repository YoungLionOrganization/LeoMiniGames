# LeoMiniGames SDK — application 0.7.3 / host API 0.7

Start with [Quickstart](QUICKSTART.md). The package format remains **rcc-v1**, theme API remains **1**, and native C++ ABI remains **1.0**. These are separate version numbers. The launcher supports legacy v0.5/v0.6 RCC packages through scoped compatibility facades.

## Documentation map

| Task | Guide |
| --- | --- |
| Build/run a first game or theme | [Quickstart](QUICKSTART.md), [Developer Lab](DEVELOPER_MODE.md) |
| All exposed properties, methods and signals | [Generated API reference](API_REFERENCE.md) |
| Manifest and package resources | [Manifest](MANIFEST.md), [Resources](RESOURCES.md) |
| State, schema migrations and backup errors | [Save](SAVE_API.md), [Lifecycle](LIFECYCLE_API.md) |
| Preferences and a settings schema | [Settings](SETTINGS_API.md) |
| Unified platform effects/music and legacy calls | [Audio](AUDIO_API.md) |
| Actions, touch and keyboard focus | [Input](INPUT_API.md) |
| Theme/language bindings and viewport sizing | [Theme](THEME_API.md), [I18n](I18N_API.md), [Viewport](VIEWPORT_API.md) |
| Random continuation, timers and local events | [Random/clock/events](RANDOM_CLOCK_EVENTS_API.md) |
| Local scores, counters, achievements and haptics | [Stats/feedback](STATS_FEEDBACK_API.md) |
| HTTPS permission and offline behavior | [Network](NETWORK_API.md) |
| Legacy migration and intentional boundaries | [Compatibility](COMPATIBILITY.md), [Migration](MIGRATION_0.6_TO_0.7.md) |
| Native C++ plugin boundary | [Native SDK](NATIVE_API.md) |
| Package errors, testing and publishing checklist | [Troubleshooting](TROUBLESHOOTING.md) |

## Runtime model

External QML packages run in a dedicated QQmlEngine with their root Item sized by the host. Context services are `App`, `Settings`, `Lang`, `Audio`, `GameTheme` (also `ThemeRuntime` as a facade alias), `Lifecycle`, `GameSettings`, `GameSave`, `GameInput`, `GameRandom`, `GameClock`, `GameEvents`, `GameI18n`, `GameAudio`, `GameResources`, `GameRuntime`, `GameStats`, `Achievements`, `Haptics`, `Viewport` and `GameLogger`. Do not import the private `LeoMiniGames` QML module or depend on host C++ internals.

Use `GameRuntime.apiVersion`, `GameRuntime.version`, `GameRuntime.capabilities` (a property, not a function), and `GameRuntime.supports("resources")`. A service-specific `capabilities()` is a method only where listed in the reference. Capability discovery describes host features, not hardware availability or permission to use network/native code.

New packages declare `api_version: "0.7"`; put only hard requirements in `required_capabilities`. Missing API fields retain legacy behavior. Package metadata does not grant Official/Verified/Native trust. [Licensing](../DEVELOPER_LICENSING.md) and publisher onboarding are separate from runtime capabilities.

The API reference is generated from headers and checked in CI. SDK examples are built as real RCCs and tested in the external engine. These checks do not replace Android/desktop physical-device QA or a production backend deployment.
