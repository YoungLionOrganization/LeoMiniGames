# SDK quickstart

Requirements: Python 3.9+ and **Qt 6 rcc**, preferably the same Qt kit as the launcher (minimum Qt 6.8; release CI uses 6.10.2). A QML-only RCC is architecture independent; native plugins are not.

## Build the included examples

From the repository root:

```sh
python3 tools/sdk/build_package.py mod-sdk/ExampleModernMod --check
python3 tools/sdk/build_package.py mod-sdk/ExampleModernMod --rcc /path/to/Qt/bin/rcc --output build-sdk/example-modern.rcc
python3 tools/sdk/build_package.py mod-sdk/ExampleHelloMod --rcc /path/to/Qt/bin/rcc --output build-sdk/example-hello.rcc
python3 tools/sdk/build_package.py theme-sdk/ExampleTheme --rcc /path/to/Qt/bin/rcc --output build-sdk/example-theme.rcc
```

On Windows use `python` and a quoted path such as `--rcc "C:/Qt/6.10.2/msvc2022_64/bin/rcc.exe"`. The example `build_mod.sh`/`.bat` and `build_theme.sh`/`.bat` wrappers work from any directory and default to a `build/` subdirectory of the project. The shared builder validates source/QRC resources, selects portable zlib compression, writes the RCC atomically, and creates a SHA-256 sidecar. It does not sign, upload, grant publisher trust, or implement the full runtime validator.

For a standalone SDK project copied elsewhere, invoke `tools/sdk/build_package.py /path/to/project` from this checkout. The wrapper expects the repository's `tools/sdk` location; copy that helper with your tooling or use Qt rcc directly.

## Run a game locally

Build/start the launcher using [BUILDING](../BUILDING.md). Open Developer Lab, verify a scoped developer key against the configured backend and import the generated game RCC. Local import is session-only, inspected and network-disabled. Inspect diagnostics when a package does not load; native libraries are not accepted through this RCC path. Offline automated SDK tests need no developer key or backend.

`ExampleHelloMod` intentionally keeps legacy API fields/calls. `ExampleModernMod` 1.1.0 targets app 0.7.3 / API 0.7 and demonstrates lifecycle state restoration, schema 1→2 migration, one shared keyboard/touch action, host audio, local stats/achievements, scrollable content and live theme/language bindings.

A theme is separate content. Use Theme Market for catalog installation, or local integration through the host's `ThemeRuntime.installThemeRcc(path, sha256)` integration. An external game cannot install a theme through its `ThemeRuntime` facade. Theme Market already calls this runtime; it is not a future-only feature.

## Create your own game

1. Copy a mod example and choose a unique [id/manifest](MANIFEST.md).
2. Keep RCC internal prefix `/`; include Main.qml, manifest, license, icon, audio and translations in mod.qrc.
3. Put preferences in GameSettings and progression in GameSave. Register migrations in Component.onCompleted before the host calls load.
4. Use GameAudio/GameInput/GameResources instead of platform-specific paths/key/decoder code.
5. Read GameTheme.revision and GameI18n.language in bindings so changes update the view.
6. Build with the helper; import in Developer Lab; test pause/reopen/portrait/landscape and [publishing acceptance](TROUBLESHOOTING.md).

Publish only under an accepted package license and the platform's publisher terms; an SDK build/check result is not approval.
