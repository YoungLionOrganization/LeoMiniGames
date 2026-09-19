# LeoMiniGames Mod SDK — RCC v1 (v0.6-compatible)

The package format is unchanged. Existing RCC v1 mods remain valid.

- `ExampleHelloMod` demonstrates the legacy/minimal API and remains compatible with v0.5.x conventions.
- `ExampleModernMod` uses additive v0.6 services (`GameSave`, `GameSettings`, `GameInput`, `Viewport`, `GameAudio`, `Haptics`, `GameI18n`, `GameStats`, `Achievements`, `Lifecycle`).

Build with Qt `rcc -binary mod.qrc -o <id>-<version>.rcc`. The app mounts the package at `qrc:/mods/<id>/`.

Do not import the private `LeoMiniGames` QML module. Use QtQuick/QtQuick.Controls and the context services. New manifest fields are optional; see `../docs/PLUGIN_FORMAT.md` and `../schemas/plugin-manifest.schema.json`.


## Licensing

Package licensing is separate from developer trust/capability. The LeoMiniGames host/core is source-available under `LicenseRef-LMG-SAPEL-1.0`; this does not automatically license independent Publisher Package code.

Files explicitly carrying `LicenseRef-YoungLion-LMG-SDK-1.0` may be used under the dedicated SDK terms in `../licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt`. Eligible proprietary Publisher Packages may use `LicenseRef-YoungLion-Publisher-Package-1.0` (`../licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt`) or another package license accepted by Platform policy.

Publishing through YoungLion is governed separately by `../docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`. Native/L3 access additionally requires `../docs/NATIVE_L3_PUBLISHER_ADDENDUM_1.0.md`; a manifest or copyright license cannot self-grant Official/Verified/Native status.
