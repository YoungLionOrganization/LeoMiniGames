# Qt 6.12 and HarmonyOS preparation

Status: 30 October 2026. v0.7.3 remains the release priority. HarmonyOS is a possible v0.7.4 target, subject to a successful native port and device evidence. There is no HAP in the current release contract.

## Verified upstream baseline

Qt 6.12.0 was released on 30 September 2026. Its [HarmonyOS platform documentation](https://doc.qt.io/qt-6/harmonyos.html) labels the port a **technology preview** and lists HarmonyOS 6.1.0 / API 23 with an arm64-v8a device target. The [prerequisites](https://doc.qt.io/qt-6/harmonyos-prerequisites.html) describe the OpenHarmony SDK command-line tools and `OHOS_SDK_ROOT`; Linux is the currently tested toolchain host. This is distinct from running an Android APK on an older HarmonyOS device.

The [packaging documentation](https://doc.qt.io/qt-6/harmonyos-building-deploying-apps.html) provides the `LeoMiniGames_make_hap` target for a Qt executable configured with the HarmonyOS Qt toolchain. Deployment uses `harmonydeployqt`, the HarmonyOS platform plugin and `hvigor`. Bundle metadata and permissions use the [Qt HarmonyOS CMake API](https://doc.qt.io/qt-6/qt-for-harmonyos-cmake-api.html).

## First step: desktop compatibility

`.github/workflows/qt-next.yml` tests the complete native CTest suite with Qt 6.12.0 on Linux x86_64, Windows x86_64 and macOS arm64. It installs tools only on GitHub runners. This verifies source compatibility with the new Qt minor version; it does not prove HarmonyOS build, HAP signing or device behavior. Qt 6.10.2 remains the v0.7.3 online-package release kit until an explicit migration is validated.

## Port work, in priority order

1. Finish v0.7.3 and keep the API 0.7 and legacy RCC fixtures passing.
2. Run the Qt 6.12 desktop lane and address actual compiler/runtime regressions before changing release kits.
3. Provision the licensed/distributable Qt HarmonyOS kit and matching SDK on a disposable Linux GitHub runner. Verify artifact provenance and tool availability before enabling the job. Do not download DevEco Studio/SDKs onto the maintainer's C drive.
4. Add an opt-in HarmonyOS CMake path and HAP metadata using the upstream functions. Keep the package identity, app version and code synchronized with canonical release metadata. Do not route HarmonyOS through Linux distro packaging.
5. Resolve the credential-store backend. The bundled QtKeychain currently selects Unix D-Bus/libsecret code for non-Apple/non-Android Unix builds. HarmonyOS needs a supported secure-storage implementation or explicit session-only storage with no plaintext fallback. A successful compile with credentials disabled is not a completed port.
6. Review native platform boundaries: `Haptics`, `GameInput`, `GameRuntime`, `PluginManager`, `UpdateService`, `AppPaths` and external-file selection. Native L3 plugin authority must remain disabled until the target is reviewed; unsupported haptics must report unavailable. Network/save/audio APIs must retain existing game contracts.
7. Package and sign a HAP, install it with the platform tool, exercise the six builtin games and RCC/legacy fixtures, then test foreground/background, touch, display insets, audio, TLS catalog access, saves and failed updates. Record the actual device/API and exact artifact digest.
8. Only then add a release asset and update-backend platform mapping. The current generic Linux architecture/download mapping must not silently recommend a Linux package on HarmonyOS.

## Acceptance for advertising support

- A repeatable GitHub build creates a HAP with the intended ABI, identity and version.
- Required Qt QML, Multimedia, TLS and platform plugins are packaged and load.
- Clean install, restart, background save and same-identity upgrade preserve data on the actual target.
- Credential storage and external file selection have explicit, tested platform behavior.
- The release/update contract serves HarmonyOS packages separately, with checksums and signing guidance.

Until those conditions are met, documentation and UI must describe HarmonyOS as planned/experimental. Upstream preview status and SDK distribution constraints may move this work beyond v0.7.4.
