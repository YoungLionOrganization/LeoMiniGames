# v0.7.3 runtime follow-up fixes

12 October 2026. Started from merged `main` commit `b2e78d9`; work branch: `codex/v073-runtime-followups`. Application version 0.7.3 and game API 0.7 remain unchanged.

## Corrected behavior

- Backup restoration previously copied the backup over the primary before checking schema compatibility and running migrations. Missing, throwing or incompatible migrations now leave both files and session memory unchanged. Successful recovery runs the migration once in memory, retains the backup's original bytes/metadata, and commits the migrated schema on the next save.
- A missing primary with a remaining backup is existing player data. Automatic saves now require that backup to be loaded successfully before creating a replacement primary; an older game cannot silently replace it with defaults.
- Invalid mod/theme catalog rows previously disappeared silently, allowing a malformed response to replace the current catalog with an empty or incomplete list. Non-object rows and missing/invalid IDs now reject the whole response, preserving the previous model. New diagnostics have English, Turkish and Azerbaijani entries.
- Resource URLs now encode literal resource paths using `QUrl::setPath`. Files containing `#`, `?` or `%` retain their exact RCC names instead of being interpreted as fragments, queries or escape sequences.
- Native key tracking now uses the physical scan code when available. A logical key change during a held key no longer leaves an action pressed, and remapping a physical key releases its previous action without releasing another key's shared action.
- CI runs automatically for `main` pushes and pull requests, with manual dispatch retained. Feature-branch pushes no longer create a second identical CI suite beside the PR suite. This avoids the duplicate-run cancellation that previously left PR checks red despite successful push tests.
- Packaged smoke mode now creates temporary save/cache/INI settings roots, selects Qt test-mode locations and uses a unique test application identity. Windows smoke runs no longer read or update the normal application's save files or registry settings.
- Audio retirement explicitly stops/detaches decoder sources. If Windows still holds an asynchronously loaded resource, a host-owned timer retries removing its private temporary directory after the clip is destroyed. The UI thread remains responsive, and rapid preload/unload regressions verify eventual file cleanup.

## Regression evidence

Local SDK: Qt 6.10.2, Windows x86_64, MinGW GCC 16.2.0, Debug with Multimedia enabled. CMake and the Qt SDK were installed in private Codex dependency directories because the system CMake had a missing DLL and the system Qt kit lacked QML. System packages were not modified.

Before fixing the affected paths, native CTest reproduced:

- `save_safety`: failed at `missing migration preserves both recovery files`, then independently at `backup-only data cannot be overwritten before loading`.
- `catalog_operations`: failed at `invalid mod row preserves entire catalog`.
- `patch_runtime_regressions`: failed at `resource URL preserves literal file name`, then independently at `changed logical key still releases its physical key`.

Save, catalog, theme, external-session, builtin persistence and SDK package tests now inject temporary AppPaths data/cache roots. Their save/package files are isolated on Windows as well as Linux; XDG environment variables alone do not select Windows AppData locations. Production AppPaths construction continues to use Qt's standard locations.

Static source, project, sanity and documentation checks passed. Distribution validation reported 300 PASS and no errors; SDK tooling passed 10 tests and package validators passed 16 tests. The existing 22 translation coverage warnings remain, with no translation errors.

Full native CTest passed **19/19** after the fixes, including both six-game QML smoke sizes and real audio decoding/temporary-file retirement. The first full run exposed the Windows asynchronous audio cleanup failure (18/19); the corrected run passed all tests. Fresh GitHub CI/artifact validation is tracked in the associated pull request. Physical device, audio output, interactive keyring and release-signing QA remain separate release conditions; no production release or tag was published by this work.
