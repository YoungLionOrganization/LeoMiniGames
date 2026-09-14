# LeoMiniGames Mod SDK — RCC v1 (v0.6-compatible)

The package format is unchanged. Existing RCC v1 mods remain valid.

- `ExampleHelloMod` demonstrates the legacy/minimal API and remains compatible with v0.5.x conventions.
- `ExampleModernMod` uses additive v0.6 services (`GameSave`, `GameSettings`, `GameInput`, `Viewport`, `GameAudio`, `Haptics`, `GameI18n`, `GameStats`, `Achievements`, `Lifecycle`).

Build with Qt `rcc -binary mod.qrc -o <id>-<version>.rcc`. The app mounts the package at `qrc:/mods/<id>/`.

Do not import the private `LeoMiniGames` QML module. Use QtQuick/QtQuick.Controls and the context services. New manifest fields are optional; see `../docs/PLUGIN_FORMAT.md` and `../schemas/plugin-manifest.schema.json`.


## Licensing

Package licensing is independent from developer trust/capability. See `../docs/DEVELOPER_LICENSING.md` before choosing an open-source or proprietary package license. Eligible proprietary packages may use `LicenseRef-YoungLion-Mod-License-1.0`; the adopted text is `../licenses/YOUNGLION_MOD_LICENSE_1.0.txt`. Qualifying independent packages using only designated public interfaces may rely on the separate GPLv3 section 7 permission in `../licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` subject to its exact conditions.

SDK example packages are MIT-licensed so developers may copy/adapt the example code into packages under compatible or proprietary terms. This does not change the GPL license of the LeoMiniGames host. Native/L3 proprietary interoperability is permitted only when the package satisfies the adopted Exception and uses the expressly designated public ABI; private/internal coupling remains outside that permission.
