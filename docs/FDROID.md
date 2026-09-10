# F-Droid Readiness

LeoMiniGames 0.6.0 is structured for a future F-Droid submission:

- application ID remains `xyz.younglion.leominigames`;
- application code is `GPL-3.0-or-later`;
- no AdMob, Firebase, Google Play Services, proprietary analytics or mandatory proprietary SDK is linked;
- core runtime is Qt 6 plus standard FLOSS C++/Qt dependencies;
- version information is deterministic in CMake;
- network catalog functionality is not required to run built-in games.

## Build profile

Configure with:

```sh
qt-cmake -S . -B build-fdroid -G Ninja -DLEOMINIGAMES_FDROID=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-fdroid --target LeoMiniGames
```

`LEOMINIGAMES_FDROID=ON` disables the normal KDAB Android OpenSSL package fetch. The F-Droid recipe must build/provide OpenSSL from source in the Qt/Android toolchain or a FLOSS srclib and make the resulting runtime libraries available to Qt Network. Do not replace it with Google Play networking services.

The pinned KDAB package remains available for normal developer Android builds because it is the existing v0.5.x HTTPS path; it is not used by the F-Droid profile.

## Reproducibility

The release recipe should pin Qt, NDK, CMake/Ninja and OpenSSL source revisions, set a stable `SOURCE_DATE_EPOCH`, and avoid embedding build-host paths. A starter metadata template is under `fdroid/` and Fastlane text metadata is under `fastlane/metadata/android/en-US/`.
