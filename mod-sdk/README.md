# LeoMiniGames Mod SDK — RCC v1 (v0.6-compatible)

The package format is unchanged. Existing RCC v1 mods remain valid.

- `ExampleHelloMod` demonstrates the legacy/minimal API and remains compatible with v0.5.x conventions.
- `ExampleModernMod` uses additive v0.6 services (`GameSave`, `GameSettings`, `GameInput`, `Viewport`, `GameAudio`, `Haptics`, `GameI18n`, `GameStats`, `Achievements`, `Lifecycle`).

Build with Qt `rcc -binary mod.qrc -o <id>-<version>.rcc`. The app mounts the package at `qrc:/mods/<id>/`.

Do not import the private `LeoMiniGames` QML module. Use QtQuick/QtQuick.Controls and the context services. New manifest fields are optional; see `../docs/PLUGIN_FORMAT.md` and `../schemas/plugin-manifest.schema.json`.
