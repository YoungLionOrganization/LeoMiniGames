# v0.7.3 implementation and verification record

The initial implementation was recorded on 1 October 2026, based on v0.7.2 commit `3f022fca55016e3f954662e4519d614b0c44bab4`. Application version is 0.7.3, Android versionCode is 703, and the game API remains 0.7. This record tracks the [original patch plan](V073_PATCH_PLAN.md), [release blockers](V073_TODO.md) and [platform audit](V073_PLATFORM_AUDIT.md). Source implementation and publication readiness are separate.

## Follow-up records

- [SDK documentation](V073_SDK_DOCUMENTATION_UPDATE.md) and [theme/audio changes](V073_THEME_AUDIO_UPDATE.md).
- [Builtin persistence and Android TLS](V073_BUILTIN_TLS_UPDATE.md).
- [Dependency updates](V073_DEPENDABOT_UPDATE.md) and [workflow repairs](V073_WORKFLOW_FIXES.md).
- [12 October runtime follow-ups](V073_RUNTIME_FOLLOWUPS.md).
- [15 October session/save integrity](V073_SESSION_INTEGRITY.md).
- [17 October Linux installer recovery](V073_INSTALLER_RECOVERY.md), [audio lifetimes](V073_AUDIO_CALLBACKS.md) and [save callback isolation](V073_SAVE_CALLBACKS.md).
- [Installation](../INSTALLATION.md), [current build matrix](../../BUILD_MATRIX.md) and [later patch priorities](../roadmap/NEXT_PATCHES.md).

## Original patch implementation

| Item | Implemented behavior | Verification and limits |
| --- | --- | --- |
| P073-01 | Canonical package directories, rejected symlink/path escapes, hashed storage keys and strict IDs. | Catalog tests cover external paths and uninstall ownership. |
| P073-02 | Failed/future-schema save loads block automatic overwrite; unchanged snapshots preserve backups; metadata types, migration depth/size are bounded. | Save tests use actual CBOR files, recovery backups and write failures. |
| P073-03 | Installed manifests retain their own API, capabilities, schema and entry metadata when the catalog changes. | Refresh/restart tests preserve the installed contract until successful replacement. |
| P073-04 | Shared install/restart compatibility, rollback on index commit failure, active-package leases and staged uninstall. | Injected network/filesystem failures cover normal recovery; not every power-loss/filesystem combination. |
| P073-05 | One guarded shutdown coordinates save, close/unload, navigation and deferred engine retirement. | External-engine and builtin tests cover reentrant close/replacement. |
| P073-06 | Save migrations bind to the game's actual JavaScript engine before component creation; callbacks retire with that engine. | Real v0.5/v0.6/v0.7 RCC fixtures cover creation, nested migration and teardown. |
| P073-07 | Native key events, focus/text-input exclusions, physical/logical key tracking and platform scan-code normalization. | Tests cover multiple keys per action and layout changes; physical layouts remain device QA. |
| P073-08 | Cancellable Memory callbacks, active-time pause/resume, invalidated background Reaction timing and session-aware clock/audio reset. | Timer/lifecycle tests cover stale callbacks and repeated pause; physical sound remains separate. |
| P073-09 | Theme identity, version/API, namespace, asset type and size checks before replacement. | Tests cover mismatches, unsupported APIs, unmount and failed removal. |
| P073-10 | Invalid/duplicate catalogs preserve the previous model; stale-origin and refresh/install races are rejected; downloads have bounded buffers/deadlines. | Injected client tests do not establish production-backend health. |
| P073-11 | Shared audio source keys, effect/media fallback, retained music pause state and bounded file/decoder cleanup. | Multimedia tests verify decoded buffers and callback lifetimes; device output/routing remains QA. |
| P073-12 | Strict SemVer/channels, approved HTTPS origins, stale replies and guarded update navigation. | Tests cover precedence, prereleases, invalid metadata and rejected launch actions. |
| P073-13 | Exact asset names, archive/native dependency checks, source/digest manifests and QtIFW repository integrity. | Package validators cover malformed outputs; actual candidate workflows and platform QA are still required. |
| P073-14 | Ordered system-keychain operations, stale-result cancellation and no plaintext fallback. | Delayed credential operations are injected; native/locked keyrings require platform evidence. |
| P073-15 | Adaptive layout, safe areas and bounded statistics; later follow-ups added full session snapshots for all six builtins. | Builtin state/session tests include 640x360; native insets, RTL, large fonts and touch need devices. Save Transfer remains later work. |
| P073-16 | Runtime, CMake, Android, installer and release metadata agree on 0.7.3/703; historical backend references remain separate. | Canonical validators enforce consistency; publication is a later gated action. |

## Platform packaging corrections

| Finding | Source correction | Required candidate evidence |
| --- | --- | --- |
| Android TLS dependency names | Correct OpenSSL SONAME/DT_NEEDED names and 16 KiB alignment. | Validate every packaged ABI/universal slice, load the real backend and perform certificate-validated HTTPS. |
| APK launch coverage | Intel emulators install/start APKs and retain process/logcat/TLS results. | ARM device launch and same-key upgrades are separate checks. |
| Windows CRT deployment | Portable and Setup payloads include matching-architecture MSVC runtime DLLs. | PE closure checks plus deployed ZIP/Setup and real Maintenance Tool servicing. |
| Linux package choices | DEB, RPM, Arch, Flatpak, native/portable tarballs, AppImage and Setup.run. | Install, launch, reinstall and remove; failed user-installer upgrades retain the previous payload. |
| macOS distribution | Deployed Qt in ZIP/DMG; explicit app bundle and Applications shortcut. | Launch extracted ZIP/mounted DMG, record Gatekeeper and signing limits. |
| Apple mobile | Unsigned iOS/iPadOS outputs remain developer test packages. | Signed device/simulator execution requires its own evidence. |

## Historical local validation

The initial 1 October Linux Qt 6.8.2 implementation passed 15 Release CTest entries, 10 package-validator tests, source/project/sanity checks and 293 distribution checks. Those dated results do not validate later commits. Subsequent native runs expanded to 22 CTest entries; follow-up records identify the changes and reproduction cases. Current verification must use the final source SHA and actual package digests.

## Publication conditions

The workflows build an immutable candidate and enforce the current 39-file public asset contract. `release/platform-qa.template.json` starts pending/failed for 25 targets. Actual environment, tester, time, evidence, package names and SHA-256 values must match that candidate, including all formats required for each target.

The platform QA workflow validates the report. Before draft/tag/publication, the publisher compares its source SHA and package digests to the validated manifest. Android ABI/signing/upgrade, Windows servicing, Linux installer lifecycle and macOS Gatekeeper retain distinct requirements. Compilation/offscreen results do not establish physical audio, touch, secure storage or device-signing behavior.

See the [release operator guide](../GITHUB_RELEASES.md). Current workflow results and the candidate QA report determine release readiness.
