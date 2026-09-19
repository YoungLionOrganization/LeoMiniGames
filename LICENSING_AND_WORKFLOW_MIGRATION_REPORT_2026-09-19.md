# LeoMiniGames — Licensing + Workflow Migration Report (2026-09-19)

Baseline: GitHub `main` commit `7537efcc40750555121edf607d5fc862a9b50bc4`, matching the licensing package review baseline.

## Licensing

Applied `LeoMiniGames_Source_Available_Licensing_v1` as the current licensing architecture. The host/core now uses `LicenseRef-LMG-SAPEL-1.0`; `IGamePlugin.h` uses the designated SDK license; the Publisher Package license and Publisher/Native-L3 terms are installed. Obsolete current GPL/plugin-exception/F-Droid submission artifacts were removed from the current tree, while historical grants remain documented by `LICENSE_HISTORY.md` and `MIGRATION_FROM_GPL.md`.

Qt and third-party rights remain separate. The repository now carries `docs/QT_LGPL_COMPLIANCE.md` and `docs/THIRD_PARTY_NOTICES.md`.

## Workflow/runtime fixes from real GitHub Actions failures

- Debian 13 Qt 6.8.x: replaced unconditional Qt 6.10-only filter-change calls with a Qt-version-gated compatibility path using `invalidateRowsFilter()` on Qt 6.5-6.9.
- Ubuntu 24.04 ARM64 AppImage: install both GStreamer Good and Bad runtime plugin libraries before linuxdeploy scans Qt Multimedia.
- macOS ARM64 artifact: DMG creation now uses a fresh temporary image, stale-volume detach and bounded retry to survive transient `hdiutil: Resource busy`.

## Verification scope

Repository validators, YAML parsing, Python compilation and shell syntax were run after this migration and passed. Controlled packaging smoke tests also exercised the Linux legal staging, Android legal sidecar, iOS packaging path and macOS three-attempt DMG retry. GitHub-hosted post-fix CI could not be started because the connected integration still receives `403 Resource not accessible by integration` on repository writes. Physical-device QA remains separate.
