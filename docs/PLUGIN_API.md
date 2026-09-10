# LeoMiniGames Plugin API 0.6

All services below are QML context objects. Legacy names `Audio` and `Settings` remain available.

## `GameSettings`

Per-game QSettings namespace: `games/<gameId>/settings/...`.

- `value(key, fallback)`
- `setValue(key, value)`
- `contains(key)`
- `remove(key)`
- `clear()`
- `schema` exposes the optional settings schema for shared/editor UI.

Do not store progress here.

## `GameSave`

- `get`, `set`, `remove`, `contains`
- `save(slot)`, `load(slot)`
- `createSlot`, `deleteSlot`, `listSlots`
- `autosave()`
- `restoreBackup(slot)`
- `registerMigration(fromVersion, toVersion, callback)`

Payloads are CBOR inside `.lmgsave`; writes are atomic and checksummed.

## `Lifecycle`

Events/actions: `load`, `start`, `pause`, `resume`, `background`, `foreground`, `save`, `close`, `unload`. Implement same-named zero-argument functions in a game root object if desired. Missing lifecycle methods are legal for legacy games.

## `GameInput`

Action-based input: `press`, `release`, `setValue`, `isPressed`, `value`. Signals: `actionPressed`, `actionReleased`, `actionValueChanged`. Standard action names include `move_left`, `move_right`, `up`, `down`, `jump`, `fire`, `pause`.

## `Viewport`

`safeWidth`, `safeHeight`, `safeTop`, `safeBottom`, `density`, `portrait`, `landscape`, `valid`. Dimensions are clamped positive so Canvas/font/radius calculations do not start from zero or negative geometry.

## `GameAudio`

`playEffect`, `preload`, `playMusic`, `stopMusic`, `pauseAll`, `resumeAll`, `volume(group)`, `setVolume(group,value)`. `playEffect(url,gain,group)` and `playMusic(url,loop,group)` support the standard `SFX`, `Music`, `UI` and `Ambient` groups; shorter overloads default to SFX/Music. Group volumes are stored under `audio/groups/`.

## `Haptics`

`light()`, `medium()`, `heavy()`, `pulse(level)`. Unsupported platforms are a no-op.

## `GameI18n`

`text(key, fallback)` resolution: requested locale → language base → plugin default locale → English → fallback/key.

## `GameStats` / `Achievements`

Local-only APIs for high score, counters, games played, total time, achievement unlock and progress. Files are CBOR and can later be synchronized by an online profile layer without changing plugin calls.

## `GameLogger`

`log(level,message,source,line)` plus automatic QML-engine warnings. Diagnostics include game ID/version/type and are isolated from the app UI via `Loader.Error` handling.

## Theme API (0.6.2)

External RCC plugins should use the public `GameTheme` context service for runtime token/surface lookup. They must not import the application's private `LeoMiniGames` QML module. `Constants.qml` and the full reusable component library are used by the application/built-in games; external developers can use the same semantic keys through `GameTheme.number()`, `GameTheme.color()`, `GameTheme.value()` and `GameTheme.surface()`.

`mod-sdk/ExampleModernMod` includes `SdkPanel.qml`, `SdkText.qml` and `SdkButton.qml` showing how to build reusable theme-aware plugin components without a private application import. Theme packages are data-only RCC packages and are separate from mod packages. See `docs/THEMING.md` and `docs/UI_COMPONENTS.md`.

## GameRandom

Deterministic per-process PRNG service: `seed(text)`, `nextInt(min,max)`, `nextReal()`, `chance(p)`, `pickIndex(count)`, `shuffled(list)`, plus serializable hexadecimal `state`. Save the state in `GameSave` when deterministic continuation matters.

## GameClock

Lifecycle-friendly monotonic gameplay clock with `reset()`, `pause()`, `resume()`, `elapsedMs()`, `elapsedSeconds()` and bounded `timeScale`. The host connects game lifecycle start/pause/resume to it.

## GameEvents

Lightweight local event bus: `emitEvent(name, payload)` emits `eventEmitted(name,payload)`. It is intended for decoupling HUD, achievements, quests and game systems inside one game, not for cross-plugin communication.
