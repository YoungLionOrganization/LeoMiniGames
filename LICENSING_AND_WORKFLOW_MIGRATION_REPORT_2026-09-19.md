# LeoMiniGames — Licensing + Workflow Migration Report (2026-09-19)

Baseline: GitHub `main` commit `7537efcc40750555121edf607d5fc862a9b50bc4`, matching the licensing package review baseline.

## Licensing

Applied `LeoMiniGames_Source_Available_Licensing_v1` as the current licensing architecture. The host/core now uses `LicenseRef-LMG-SAPEL-1.0`; `IGamePlugin.h` uses the designated SDK license; the Publisher Package license and Publisher/Native-L3 terms are installed. Obsolete current GPL/plugin-exception/F-Droid submission material is neutralized. Legacy paths that may remain tracked after an archive-overwrite migration are replaced with explicit `LMG_LEGACY_TOMBSTONE` redirect stubs, so they cannot continue presenting the old terms as current. Historical grants remain documented by `LICENSE_HISTORY.md` and `MIGRATION_FROM_GPL.md`.

Qt and third-party rights remain separate. The repository now carries `docs/QT_LGPL_COMPLIANCE.md` and `docs/THIRD_PARTY_NOTICES.md`.

## Workflow/runtime fixes from real GitHub Actions failures

- Debian 13 Qt 6.8.x: replaced unconditional Qt 6.10-only filter-change calls with a Qt-version-gated compatibility path using `invalidateRowsFilter()` on Qt 6.5-6.9.
- Ubuntu 24.04 ARM64 AppImage: install both GStreamer Good and Bad runtime plugin libraries before linuxdeploy scans Qt Multimedia.
- macOS ARM64 artifact: DMG creation now uses a fresh temporary image, stale-volume detach and bounded retry to survive transient `hdiutil: Resource busy`.

## Verification scope

Repository validators, YAML parsing, Python compilation and shell syntax were run after this migration and passed. Controlled packaging smoke tests also exercised the Linux legal staging, Android legal sidecar, iOS packaging path and macOS three-attempt DMG retry. GitHub-hosted post-fix CI could not be started because the connected integration still receives `403 Resource not accessible by integration` on repository writes. Physical-device QA remains separate.

## Overlay-migration regression found after the first delivery

GitHub CI run `35465901365` and artifact run `35465945026` did not show widespread platform build failures. Their static validation job failed first, and every matrix job depending on it was skipped. The reason was archive overlay semantics: copying the prior ZIP over an existing Git checkout added/replaced files but did not delete 17 previously tracked GPL/F-Droid paths.

This revision makes that migration idempotent: all known legacy tracked paths are present as non-authoritative tombstone/redirect stubs, `validate_distribution.py` accepts only absent-or-tombstoned legacy paths, and active legacy/F-Droid content still fails validation. `APPLY_TO_EXISTING_REPO.ps1` and `.sh` are included for safer application to an existing checkout.
