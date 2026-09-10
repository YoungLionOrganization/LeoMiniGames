# LeoMiniGames Mod SDK — RCC v1 (v0.6-compatible)

The package format is unchanged. Existing RCC v1 mods remain valid.

- `ExampleHelloMod` demonstrates the legacy/minimal API and remains compatible with v0.5.x conventions.
- `ExampleModernMod` uses additive v0.6 services (`GameSave`, `GameSettings`, `GameInput`, `Viewport`, `GameAudio`, `Haptics`, `GameI18n`, `GameStats`, `Achievements`, `Lifecycle`).

Build with Qt `rcc -binary mod.qrc -o <id>-<version>.rcc`. The app mounts the package at `qrc:/mods/<id>/`.

Do not import the private `LeoMiniGames` QML module. Use QtQuick/QtQuick.Controls and the context services. New manifest fields are optional; see `../docs/PLUGIN_FORMAT.md` and `../schemas/plugin-manifest.schema.json`.


## Licensing

Package licensing is independent from developer trust/capability. See `../docs/DEVELOPER_LICENSING.md` before choosing an open-source or proprietary package license. The current `YoungLion-Mod-License` file is only a placeholder and must not be used as final legal terms.

SDK example packages are MIT-licensed so developers may copy/adapt the example code into packages under compatible or proprietary terms. This does not change the GPL license of the LeoMiniGames host or resolve the separate Native/L3 linking boundary.
