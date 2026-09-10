# LeoMiniGames v0.7.0 — Final Validation Report

Validation date: 2026-09-10
Scope: LeoMiniGames application/source tree only. Backend, Account and Admin reference packages are not modified or bundled.

## Executive result

The source-level v0.7.0 validation suite is clean for the checks that can be executed in this environment.

- `tools/source_guard.py`: **PASS**
- `tools/validate_project.py`: **PASS**
- `tools/sanity_check.py`: **PASS**
- `tools/security_audit.py`: **16 PASS / 0 WARNING / 0 ERROR**
- `tools/audit_prebuilt_binaries.py`: **PASS**
- `tools/validate_v070.py`: **51 PASS / 0 WARNING / 0 ERROR**
- `tools/validate_distribution.py`: **82 PASS / 1 WARNING / 0 ERROR**
- `tools/validate_i18n.py`: **0 ERROR / 22 coverage warnings**
- Python validator/package scripts: **syntax PASS**
- Shell scripts: **`bash -n` PASS**
- Deterministic source ZIP reproduction check: **PASS (byte-identical)**

The warnings are deliberate release-readiness disclosures, not hidden failures:

1. Non-English host translations do not yet cover every one of the 188 current source strings. Runtime source-text fallback remains available, so missing locale keys do not make the UI fail to load.
2. Real upstream F-Droid screenshots are not present. Placeholder/fake screenshots were intentionally not created.

## Backward compatibility

The v0.7.0 validator confirms the compatibility contract for legacy content:

- manifests without `api_version` are treated as legacy content rather than rejected;
- canonical RCC internal prefix `/` is supported;
- legacy RCC prefix `/mods/<game_id>` is supported through the compatibility mount path;
- a malicious legacy RCC that escapes its own namespace is rejected;
- legacy `Settings`, `Lang`, `Audio`, `Lifecycle`, `App`, theme and game-service adapter surfaces remain available;
- legacy settings support copy-on-read migration into the game-scoped namespace;
- old `GameAudio` compatibility overloads remain present;
- installed-mod indexes are processed once;
- modern `required_capabilities` are enforced without retroactively converting legacy capability metadata into hard requirements.

Static compatibility fixtures validated:

- `v0_5_canonical`
- `v0_5_legacy_prefix`
- `v0_6_services`
- `v0_7_modern`
- `malicious_legacy_namespace` (expected reject case)

A Qt/CTest target is included to compile these fixtures into real `.rcc` files and exercise `RccPackageInspector` when Qt is available.

## Security findings closed in the application

Source-level validation confirms the following hardening is present:

- external QML service exposure uses an allowlisted facade surface;
- external games run through `ExternalGameRuntime` instead of receiving the main application context directly;
- arbitrary local/file network access is not exposed through the external runtime network manager;
- local Developer RCC packages require authenticated Developer Mode, are size-limited, validated, session-mounted and cannot self-grant native/L3 trust;
- RCC legacy namespace escape/injection is rejected;
- Developer API-key format, redirect policy and response size are bounded, and no obvious credential persistence is present;
- publisher/native trust is not granted from arbitrary custom-catalog declarations;
- logger facade does not expose file export/clear operations to external games;
- legacy audio facade is game-scoped;
- `GameSave` empty game IDs fail closed instead of producing root/current-directory paths;
- `GameStats` and `Achievements` have bounded state handling and empty-ID guards;
- lifecycle close/background/quit paths use force-save safety paths;
- prebuilt executable/library artifacts are not committed into the source tree.

## Distribution checks

Static distribution validation confirms:

- CI covers Ubuntu 22.04/24.04 x86_64, Ubuntu 24.04 ARM64, Windows MSVC/LLVM-MinGW x86_64, Windows ARM64 cross-build, macOS arm64/x86_64, all four Qt Android ABIs and unsigned iOS simulator arm64/x86_64 build paths;
- Section 22 remains intentionally excluded: no tag-triggered GitHub Release automation is added;
- QtIFW package metadata is v0.7.0 and marks the main application component forced/essential;
- Windows packaging creates a portable ZIP and QtIFW installer path;
- Linux packaging requires `linuxdeploy` and produces the portable archive from the deployed AppDir, plus AppImage;
- macOS packaging uses `macdeployqt` and provides ZIP/DMG paths;
- Android package script checks for actual APK/AAB output rather than claiming an artifact that does not exist;
- iOS packaging explicitly labels its output unsigned/test when signing credentials are unavailable;
- F-Droid scaffold now uses the real canonical public repository URLs; only the final immutable 40-character submission commit SHA and a tested fdroidserver Qt source-build stanza remain unresolved;
- backend/account/admin reference material is excluded from the application source package.

## i18n coverage

The host contains 188 unique current translatable source strings.

- English source fallback: 188/188 (100%)
- Azerbaijani: 92/188 (48.9%)
- Turkish: 92/188 (48.9%)
- Other currently supplied host locale tables: typically 29/188 (15.4%)

Missing translation coverage is reported rather than filled with fake translations. Locale parsing, placeholder validation and runtime fallback remain enabled.

## Tests not executed in this environment

This environment does not contain a Qt 6 SDK, `qt-cmake`, `qmake6` or `qmllint`. Therefore the following are **NOT EXECUTED**, and are not represented as PASS:

- native C++/Qt compilation;
- `qmllint`/QML runtime loading;
- CTest execution using real generated RCC binaries;
- Windows QtIFW installer execution on a fresh VM;
- Windows portable launch test;
- Linux AppImage launch on a clean distribution;
- Android APK/AAB device/emulator execution;
- physical keyboard-layout testing (Azerbaijani/AZERTY/QWERTZ/Russian/Turkish);
- Android/iOS lifecycle, safe-area, audio and haptics physical-device testing;
- macOS signing/notarization and physical runtime testing;
- iOS signing/App Store packaging.

The repository contains CI/build/test paths for these checks, but configured CI is not the same as an executed physical-device test.

## Remaining release-readiness limitations

- Complete human-reviewed translations are still required for full multilingual parity.
- Real application screenshots must be captured before an F-Droid submission that uses upstream screenshots.
- The final immutable 40-character source commit SHA is required before generating the fdroiddata submission recipe.
- Native browser-to-LeoMiniGames OAuth loopback/App-Link flow depends on backend/account support; the application retains a session-only scoped developer credential fallback and does not invent a nonexistent backend endpoint.

## Final package policy

`tools/package/package_source.py` runs the application validators before packaging, uses deterministic ZIP ordering/timestamps, excludes build output, IDE-user state and backend deployment material, and emits SHA-256 checksum files beside the final source archive.

## Qt 6.11 build hotfix verification

A user-side Qt 6.11.1 `qmlcachegen` build exposed an invalid semicolon between nested QML object declarations in `qml/DeveloperLabPage.qml`. The runtime-diagnostics actions were rewritten as normal multi-line `BronzeButton` declarations inside a responsive `Flow`. Repository-wide scanning found no second instance of this separator pattern. `tools/source_guard.py` now rejects this syntax class before packaging.

## Qt 6.11 Windows compile hotfix — second pass

A real Qt 6.11.1 LLVM-MinGW build supplied after the QML syntax hotfix exposed three additional source/build defects that source-only validation had not caught:

- `qt_add_executable(LeoMiniGames)` was followed by an explicit `qt_finalize_executable(LeoMiniGames)`, causing Qt to finalize the target twice. The explicit finalizer was removed; automatic end-of-directory finalization is retained.
- `GameAudio::activate()` passed a `QUrl` to `AudioManager::releasePrefix(const QString &)`. The call now passes the expected `QString` prefix.
- `ExternalGameRuntime.cpp` included nonexistent `<QNetworkAccessManagerFactory>`. It now includes Qt's actual `<QQmlNetworkAccessManagerFactory>` header and owns the factory until the `QQmlEngine` is destroyed.

The same user build also reported many `unqualified` warnings in `DeveloperLabPage.qml`. The page now uses `pragma ComponentBehavior: Bound`, receives Developer/GameLogger as required properties, and references them through the root object. The Repeater delegate already uses an explicit required `modelData` property, which is compatible with bound component behavior.

`tools/source_guard.py` now rejects the three exact CMake/C++ regression patterns above and rejects direct unqualified Developer/GameLogger context access in `DeveloperLabPage.qml`.

This environment still does not have a functioning local Qt SDK installation, so the patched source cannot be represented here as a locally compiled Qt binary. The reported Windows Qt 6.11 compiler diagnostics have nevertheless been addressed directly and the source-level validator suite has been rerun.
## Build Hotfix 3 verification
- `GameResources::GameResources(QObject*)` linker definition: PASS
- Generic out-of-line QObject constructor definition guard: PASS
- Source guard: PASS
- v0.7 validator: 47 PASS / 0 WARNING / 0 ERROR
- Security audit: 16 PASS / 0 WARNING / 0 ERROR


## Publisher Trust Hotfix Validation

- Root cause: historical admin-only Mod rows expose no v0.7 publisher trust fields; historical Theme rows can expose stale `publisher_verified=false`. The v0.7 client therefore downgraded admin-published official packages to Unverified.
- Client fix: canonical YoungLion legacy catalog rows with no modern trust shape resolve to `Official Publisher`; modern explicit trust remains authoritative and third-party origins remain unverified.
- CTest regression source: `tests/test_publisher_trust_resolver.cpp`.


## GitHub Actions / F-Droid workflow hotfix verification

- Current GitHub Android arm64 failure root cause addressed: the standalone `<QNativeInterface>` include was removed; Qt documents `QNativeInterface::QAndroidApplication` through the Core application header.
- Current GitHub Android armeabi-v7a failure root cause addressed: target `qt-cmake` is invoked through `bash`, so a missing executable bit no longer prevents configuration.
- Android CI now covers `arm64-v8a`, `armeabi-v7a`, `x86_64` and `x86`.
- Desktop CI adds Linux ARM64, Windows ARM64 cross-build and a Windows LLVM-MinGW lane matching local development expectations.
- Linux release packaging uses `linuxdeploy-plugin-qt` and the maintained `AppImage/appimagetool` project.
- F-Droid SourceCode/Repo/IssueTracker/website/application/version identifiers are no longer placeholders.
- F-Droid Qt source-build status is documented accurately: Qt 6 is feasible in fdroiddata, but LeoMiniGames still needs its own tested source-build stanza and final immutable commit.
- README and RELEASES were expanded with architecture, artifact, compatibility, trust, signing, F-Droid and validation documentation.
