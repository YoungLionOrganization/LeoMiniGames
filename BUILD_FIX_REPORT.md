# LeoMiniGames v0.7.0 — Build & Packaging Fix Report

Date: 2026-09-14

## Resolved source/build failures

- Fixed `DeveloperLabPage.qml` syntax failure (`Unexpected token ;`) and retained qualified/bound-safe QML access.
- Removed the duplicate Qt target-finalization path that produced `qt6_finalize_target twice` warnings.
- Fixed `GameAudio` → `AudioManager::releasePrefix()` type mismatch by passing `QString` instead of `QUrl`.
- Corrected the QML network access factory include to `QQmlNetworkAccessManagerFactory`.
- Added the missing `GameResources::GameResources(QObject*)` implementation that caused the undefined linker symbol.
- Removed the unavailable standalone `<QNativeInterface>` include on Android; Android haptics obtains `QNativeInterface::QAndroidApplication` through Qt Core headers.
- Fixed CI's false failure where `validate_distribution.py` searched obsolete literal artifact names and skipped the whole platform matrix before compilation. The validator now parses workflow YAML and validates job/matrix semantics.
- Android CI invokes target `qt-cmake` through `bash`, avoiding archive execute-bit failures seen on the armv7 Qt package.
- Android package/application metadata is sourced from CMake (`QT_ANDROID_*`) and the custom manifest uses Qt substitution placeholders instead of duplicating version/application data.
- Removed the unused AndroidX `FileProvider`, avoiding an unnecessary AndroidX dependency in a pure Qt package.
- Added Windows `.ico` executable resource, macOS `.icns`, Android launcher icon property and QtIFW installer/window/page-list artwork.
- Windows installer shortcuts now set working directory and explicit executable icon.
- Source packaging no longer accidentally drops `build_android.bat` / `build_android.sh` merely because their filenames begin with `build`. Build *directories* are still excluded.
- Release artifact workflow uses Qt Installer Framework 4.11 (`qt.tools.ifw.411`) and a manual-only artifact pipeline.

## Compatibility retained

The runtime compatibility fixtures for v0.5 canonical RCC, v0.5 legacy-prefix RCC, v0.6 service packages and v0.7 modern packages remain validated. New license metadata is not made mandatory for loading historical installed packages.

## Validation status

Static validation passes with no errors. The only expected distribution warning is the absence of real upstream F-Droid screenshots. Translation coverage warnings remain quality debt rather than runtime failures because source-text fallback is enabled.

## Not claimed

This package has not been physically executed on every Windows/Linux/macOS/Android/iOS target in this environment. CI/device success must be confirmed after the final tree is pushed to GitHub and the native workflows run with the required signing credentials.

## 2026-09-22 v0.7.1 workflow follow-up

GitHub CI run `35752864666` (CI #24) at `d22534e2170948f2ab6c0f8b333cec5380e88f6a` had one failed lane: Android x86_64. Compilation/configuration succeeded; `androiddeployqt` failed while the Gradle wrapper fetched `gradle-8.14.3-bin.zip` because the upstream request returned HTTP 500. The same commit's Build Release Artifacts #11 succeeded, confirming this was a transient distribution transport failure rather than a source incompatibility.

Android CI and packaging now use `tools/ci/android_build_with_retry.sh`. It retries only logs matching Gradle-distribution/network failures and does not retry ordinary compile/link/package errors. `Publish Release` remains exact-SHA gated and therefore correctly refused Publish #4 while CI #24 was red.

Full evidence for the current workflow + Developer Lab servicing pass is recorded in `WORKFLOW_DEVELOPER_LAB_FIX_REPORT.md`.

## 2026-09-22 v0.7.1 release-artifact packaging fix

GitHub Build Release Artifacts #12 (`35757409242`) at commit `2fbd4b7ffc9259c5d3171ff5247d9393e1fa2bbb` exposed a release-only Android packaging regression. The Android source/configuration steps succeeded, but every per-ABI release job and the universal multi-ABI job failed immediately in `tools/package/package_android.sh` with exit code 65:

```text
Missing Android retry helper: .../tools/ci/android_build_with_retry.sh
```

The helper was not actually absent. The exact Git tree contains `tools/ci/android_build_with_retry.sh` with mode `100644`. `package_android.sh` incorrectly required `[[ -x ... ]]` and then invoked the script directly. That assumption is unnecessary and is fragile across Git mode handling and ZIP/source-package extraction.

Resolution:

- Android packaging now checks the helper with `[[ -f ... ]]` rather than executable permission.
- APK and AAB targets invoke the helper explicitly through `bash`, matching CI's already-working invocation model.
- `validate_v071.py` now rejects a future `-x` dependency and requires both APK/AAB helper calls to use `bash`.

This fix does not weaken build validation and does not retry ordinary compile/link failures. It only removes an irrelevant filesystem-mode precondition from the release packaging path.
