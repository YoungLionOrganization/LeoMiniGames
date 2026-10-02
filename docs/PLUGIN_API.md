# LeoMiniGames Plugin API 0.7 — app 0.7.3

The authoritative signature list is the [generated host API reference](sdk/API_REFERENCE.md); usage/behavior guides are indexed in [SDK Overview](sdk/OVERVIEW.md). It is generated from the actual context-object declarations and checked for drift.

| Area | Public service / guide |
| --- | --- |
| Host version and requirements | GameRuntime properties/supports; [Manifest](sdk/MANIFEST.md) |
| Save, schema and backup | GameSave; [Save](sdk/SAVE_API.md) |
| Session hooks and close | Lifecycle + App.closeGame; [Lifecycle](sdk/LIFECYCLE_API.md) |
| Preferences | GameSettings / legacy Settings; [Settings](sdk/SETTINGS_API.md) |
| Keyboard and actions | GameInput; [Input](sdk/INPUT_API.md) |
| Effects and music, legacy Audio | GameAudio; [Audio](sdk/AUDIO_API.md) |
| Theme / geometry | GameTheme and Viewport; [Theme](sdk/THEME_API.md), [Viewport](sdk/VIEWPORT_API.md) |
| Package translation and resources | GameI18n / GameResources; [I18n](sdk/I18N_API.md), [Resources](sdk/RESOURCES.md) |
| Deterministic continuation / active clock / event bus | GameRandom / GameClock / GameEvents; [Random/clock/events](sdk/RANDOM_CLOCK_EVENTS_API.md) |
| Scores, achievements, haptics, diagnostics | GameStats / Achievements / Haptics / GameLogger; [Feedback](sdk/STATS_FEEDBACK_API.md) |
| HTTPS permissions | Installed opt-in versus Developer-local policy; [Network](sdk/NETWORK_API.md) |
| Native C++ ABI | IGamePlugin 1.0; [Native SDK](sdk/NATIVE_API.md) |

Legacy Audio/Settings/Lang/App/GameTheme/Lifecycle call shapes remain compatible. External games use scoped facades; host installation/export/native trust internals are not granted by a manifest. Do not import private LeoMiniGames QML components. Theme-aware package-local components are demonstrated in ExampleModernMod.

Host API 0.7, rcc-v1, theme API 1, native ABI 1.0 and application 0.7.3 are independent versions. An SDK example is not a production deployment or publisher approval. [Quickstart](sdk/QUICKSTART.md) provides reproducible build/test steps.
