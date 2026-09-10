# LeoMiniGames v0.7.0 Build / Test Matrix

Date: 2026-09-10

Status vocabulary:
- **STATIC PASS** — source/configuration validators executed successfully here.
- **CI CONFIGURED** — workflow exists but was not executed in this delivery environment.
- **NOT EXECUTED** — no claim of success is made.

| Target | Configure/build | Unit/compat tests | Packaging | Physical/runtime QA |
| --- | --- | --- | --- | --- |
| Linux x86_64 | CI CONFIGURED (`ubuntu-24.04`) | CTest configured | linuxdeploy archive + AppImage script | NOT EXECUTED |
| Windows x86_64 | CI CONFIGURED (`windows-2025`) | CTest configured | windeployqt ZIP + QtIFW script | NOT EXECUTED |
| macOS arm64 | CI CONFIGURED (`macos-15`) | CTest configured | macdeployqt ZIP + DMG script | NOT EXECUTED |
| macOS x86_64 | CI CONFIGURED (`macos-15-intel`) | CTest configured | macdeployqt ZIP + DMG script | NOT EXECUTED |
| Android arm64-v8a | CI CONFIGURED | build tests configured; CTest disabled for target build | APK/AAB helper | NOT EXECUTED |
| Android armeabi-v7a | CI CONFIGURED | build tests configured; CTest disabled for target build | APK/AAB helper | NOT EXECUTED |
| iOS simulator arm64 | CI CONFIGURED (`macos-15`) | unsigned build configured | unsigned/test app ZIP helper | NOT EXECUTED |

## Static validation executed locally

- CMake/project version and source/resource consistency: PASS through project validators.
- GitHub Actions YAML: parsed successfully.
- Shell packaging scripts: `bash -n` PASS.
- QtIFW XML: parsed; application component is Essential + ForcedInstallation.
- F-Droid template/upstream metadata: static contract PASS, real screenshots pending.
- Prebuilt-binary audit: PASS.

## Qt toolchain limitation

The delivery container does not include Qt 6 (`qt-cmake`, `rcc`, `qmllint`, Qt headers/libraries). Native compilation and CTest execution therefore cannot be performed locally. The configured CI is the next executable verification layer.

## Section 22

No tag-triggered release/GitHub Release workflow is included. CI only runs on push, pull request and manual dispatch as requested.
