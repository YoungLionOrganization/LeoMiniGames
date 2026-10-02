# Mod SDK — rcc-v1 / host API 0.7 / app 0.7.3

Start with [Quickstart](../docs/sdk/QUICKSTART.md), [API index](../docs/sdk/OVERVIEW.md) and [generated signatures](../docs/sdk/API_REFERENCE.md).

- ExampleHelloMod 1.1.0 preserves legacy API/manifest behavior, including Audio calls.
- ExampleModernMod 1.1.0 targets API 0.7 / minimum app 0.7.3. It demonstrates lifecycle migration/save, keyboard/touch parity, host audio, optional haptics, local stats/achievements, live theme/language and scrollable short-screen layout.

```sh
python3 tools/sdk/build_package.py mod-sdk/ExampleModernMod --check
python3 tools/sdk/build_package.py mod-sdk/ExampleModernMod --rcc /path/to/Qt6/rcc --output build-sdk/example-modern.rcc
```

Run these commands from the repository root, or pass absolute paths. Example wrappers build from any working directory into their project's build/ folder and accept the rcc executable path. See Quickstart for Windows and standalone copied-project instructions. The helper creates an RCC plus SHA-256 sidecar; it does not publish/sign/grant trust. Use Qt 6 rcc; canonical mod internal prefix is `/`, mounted by the host at `qrc:/mods/<id>/`.

Every resource must appear in mod.qrc, including referenced icon, license and locale files. External games use Qt Quick and context services; they do not import the private LeoMiniGames module. Existing v0.5/v0.6 RCCs need no repacking just to run on 0.7.3. Manifest schema: [plugin-manifest](../schemas/plugin-manifest.schema.json).

With BUILD_TESTING=ON the main build compiles the examples and `ctest --test-dir build -R sdk_examples --output-on-failure` runs them offline in the real external engine. Manual Developer Lab import requires backend key verification and remains network-disabled. Physical device/codec/touch QA is still separate.

## Licensing

Package licensing is separate from developer trust/capability. The LeoMiniGames host/core is source-available under `LicenseRef-LMG-SAPEL-1.0`; this does not automatically license independent Publisher Package code.

Files explicitly carrying `LicenseRef-YoungLion-LMG-SDK-1.0` may be used under the dedicated SDK terms in `../licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt`. Eligible proprietary Publisher Packages may use `LicenseRef-YoungLion-Publisher-Package-1.0` (`../licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt`) or another package license accepted by Platform policy.

Publishing through YoungLion is governed separately by `../docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`. Native/L3 access additionally requires `../docs/NATIVE_L3_PUBLISHER_ADDENDUM_1.0.md`; a manifest or copyright license cannot self-grant Official/Verified/Native status.
