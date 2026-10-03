# Building LeoMiniGames 0.7.3

See [Linux package choices](LINUX_PACKAGES.md) for package formats and installation/lifecycle tests.

## Requirements

- CMake 3.21+
- C++20 compiler
- Qt 6.8+ modules: Core, Gui, Qml, Quick, QuickControls2, QuickShapes, Svg, Network, Multimedia
- Ninja is recommended

## Desktop

```sh
qt-cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

On Windows, select the Qt kit matching your compiler. The deployable target is the **LeoMiniGames executable**, not one of the static built-in plugin targets.

## Android

The target keeps package name `xyz.younglion.leominigames`, minimum SDK 28 and the Android package source under `android/`. The ABI is intentionally not hard-coded in the project; use the ABI from the selected Qt for Android kit (release CI uses Qt 6.10.2 multi-ABI options).

The privacy build profile is a tracker-free/source-clean configuration and does not imply official F-Droid eligibility. Current source-available editions are not eligible for F-Droid main. Use `-DLEOMINIGAMES_PRIVACY_BUILD=ON` or `tools/privacy_clean_build.sh`; see `F_DROID_TRANSITION.md`.

## Validation

The repository validators are source/static checks; they do not replace a real Qt build:

```sh
python tools/source_guard.py
python tools/sanity_check.py
python tools/validate_project.py
python tools/security_audit.py
python tools/validate_i18n.py
python tools/validate_distribution.py
python tools/audit_prebuilt_binaries.py
python tools/validate_release_contract.py
```

Normal/release builds require Qt Multimedia; missing development files fail configuration. Only deliberate headless/test builds should use `-DLEOMINIGAMES_ENABLE_AUDIO=OFF`. Desktop packaging checks for a deployed Qt media backend; Android checks a media backend plugin in every ABI. `host_audio_compatibility` tests the shared modern/legacy audio layer and real fallback decoding, while physical device output/routing remains release QA.

## SDK examples and offline tests

Python 3.9+ is required when BUILD_TESTING=ON. The build compiles the mod/theme examples into test RCCs; `ctest --test-dir build -R sdk_examples --output-on-failure` loads them in the real external engine without a backend/key. Authoring commands and native example compilation are in [SDK Quickstart](sdk/QUICKSTART.md).

```sh
python3 tools/validate_documentation.py
python3 tools/sdk/generate_api_reference.py --check
python3 tests/test_sdk_tools.py
```
