# Building LeoMiniGames 0.6.0

## Requirements

- CMake 3.24+
- C++20 compiler
- Qt 6.8+ modules: Core, Gui, Qml, Quick, QuickControls2, QuickShapes, Svg, Network, Multimedia
- Ninja is recommended

## Desktop

```sh
qt-cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target LeoMiniGames
```

On Windows, select the Qt kit matching your compiler. The deployable target is the **LeoMiniGames executable**, not one of the static built-in plugin targets.

## Android

The target keeps package name `xyz.younglion.leominigames`, minimum SDK 28 and the Android package source under `android/`. The ABI is intentionally not hard-coded in the project; use the ABI from the selected Qt for Android kit (or Qt 6.11 multi-ABI options).

The normal Android profile can fetch the pinned KDAB Android OpenSSL package used by v0.5.x for HTTPS. F-Droid builds disable that prebuilt-fetch path and must provide a source-built FLOSS OpenSSL runtime; see `FDROID.md`.

## Validation

The repository validators are source/static checks; they do not replace a real Qt build:

```sh
python tools/source_guard.py
python tools/sanity_check.py
python tools/validate_project.py
python tools/validate_v060.py
```
