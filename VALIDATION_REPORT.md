# LeoMiniGames v0.7.0 — Validation Report

Date: 2026-09-19
Baseline reviewed/fixed: `7537efcc40750555121edf607d5fc862a9b50bc4`

## Scope

This report covers the source-available licensing migration supplied in
`LeoMiniGames_Source_Available_Licensing_v1.zip` and the concrete failures from
the latest GitHub Actions runs inspected for that baseline.

The licensing bundle's SHA-256 manifest was verified before application (22
payload files matched). The baseline SHA recorded by the licensing bundle
matches the GitHub `main` source used for this migration.

## Licensing migration

Current host/core license:

- `LicenseRef-LMG-SAPEL-1.0`
- `LICENSE` = LeoMiniGames Source-Available Publisher Ecosystem License 1.0
- not OSI Open Source / not Free Software, as stated by the supplied license

Explicit SDK license:

- `LicenseRef-YoungLion-LMG-SDK-1.0`
- `src/sdk/IGamePlugin.h` carries this SPDX identifier

Optional Publisher Package license:

- `LicenseRef-YoungLion-Publisher-Package-1.0`

Applied/added legal material includes `LICENSE_HISTORY.md`,
`CONTRIBUTOR_LICENSE_AGREEMENT.md`, `LICENSE_METADATA.json`,
`MIGRATION_FROM_GPL.md`, `LEGAL_CHANGELOG.md`, Qt/LGPL and third-party notices,
publisher terms, acceptance-evidence guidance and the Native/L3 addendum.

Obsolete current-license content is neutralized. Legacy paths that can survive when a ZIP is copied over an existing Git checkout are overwritten with `LMG_LEGACY_TOMBSTONE` redirect stubs rather than being relied upon to disappear. This includes the old GPL application path, Plugin Exception paths, old YoungLion Mod License paths, old open-source licensing docs and the F-Droid submission scaffold. Historical prior grants remain explicitly documented in `LICENSE_HISTORY.md` and `MIGRATION_FROM_GPL.md`.

Runtime/UI/package metadata was migrated too: built-in games report the new
host license identifier, Settings shows the new host ID, QtIFW presents the new
host license, and binary package legal payloads carry the new host/SDK/Publisher
license material plus Qt/third-party notices.

## GitHub Actions failures investigated

### Regression identified in the latest runs

Both latest workflows failed at their single source-validation gate before platform jobs ran. The CI job reported **226 PASS / 17 ERROR**, where all 17 errors were legacy files/directories still present after the previous ZIP was copied over the Git checkout. Because every build job has `needs: validate`, the platform jobs were then reported as skipped. This was an update-delivery bug, not evidence that all platform toolchains had independently failed.

This revision fixes the delivery model by overwriting those known legacy tracked paths with safe tombstones and by validating that every retained legacy path is either absent or explicitly tombstoned. A local simulation of the exact overlay failure mode now passes.

Latest inspected CI run: `35465901365` at commit `ce89aa5e24f206ace2c81b2dc7338af7ee6a9555`.
Latest inspected Build Release Artifacts run: `35465945026` at the same commit.

The latest real runners had already proven Windows x64/ARM64 installer jobs,
macOS x86_64/universal, all supported Android ABIs/universal, Apple device arm64
and simulator x86_64 lanes, Arch x86_64, Ubuntu x86_64, and general Ubuntu
ARM64 compilation/tests.

Remaining failures and fixes:

1. **Debian 13 x86_64 + ARM64** — distro Qt 6.8.2 failed because
   `QSortFilterProxyModel::beginFilterChange/endFilterChange` were called
   unconditionally. Both proxy models now use the already-proven Qt 6.10 path
   on Qt >= 6.10 and `invalidateRowsFilter()` on Qt 6.5-6.9.
2. **Ubuntu 24.04 ARM64 AppImage** — `linuxdeploy-plugin-qt` failed after Qt
   Multimedia dependency scanning with missing `libgstphotography-1.0.so.0`.
   Artifact setup now installs both `libgstreamer-plugins-good1.0-0` and
   `libgstreamer-plugins-bad1.0-0`.
3. **macOS ARM64 artifact** — build + CTest succeeded; DMG creation failed with
   `hdiutil: create failed - Resource busy`. DMG creation now uses a fresh
   temporary destination, stale-volume detach, bounded three-attempt retry and
   a non-empty output assertion.

## Validation results

Final local validation:

- `tools/source_guard.py`: PASS
- `tools/validate_project.py`: PASS
- `tools/sanity_check.py`: PASS
- `tools/security_audit.py`: **16 PASS / 0 ERROR**
- `tools/validate_i18n.py`: **0 ERROR**, 22 translation-coverage warnings with
  source-English fallback; these warnings pre-existed and are not workflow
  failures
- `tools/validate_distribution.py`: **244 PASS / 0 ERROR**
- `tools/audit_prebuilt_binaries.py`: PASS, including nested-archive guard
- `tools/validate_v060.py`: PASS as the v0.6.x compatibility validator
- `tools/validate_v070.py`: **51 PASS / 0 ERROR**
- `tools/relicense_headers.py` dry-run: **0 files matched**
- all `tools/*.py`: Python bytecode compilation PASS
- CI/artifact workflow YAML parse: PASS, 10 jobs each
- all `tools/*.sh`: `bash -n` PASS

Behavioral packaging smoke tests with controlled mock platform tools:

- macOS DMG retry: PASS; two simulated `Resource busy` failures followed by a
  successful third attempt
- Linux portable staging: PASS; new legal payload present in tar/AppDir
- Linux native staging: PASS; new legal payload present
- Android legal sidecar: PASS; new legal files present and old Plugin
  Exception/Mod License/Application file absent
- iOS unsigned-package flow: PASS

The stale nested `LeoMiniGames.zip` in the previous source artifact contained an older GPL/F-Droid/deployment tree and remains excluded. `DELETE_FROM_REPOSITORY.txt` is now an inert migration tombstone so archive-overwrite updates can neutralize the previously tracked file without depending on deletion semantics. The source audit continues to reject nested `.zip/.7z/.rar` delivery archives.

## External execution limitation

The connected GitHub integration can read runs and logs, but repository mutation still returns `403 Resource not accessible by integration` when creating a branch. Therefore this environment cannot push this second fix and obtain a post-fix GitHub-hosted runner result. The latest failed workflows were nevertheless fully diagnosed from their real logs, and the exact static-gate failure mode was reproduced locally and made to pass.

No physical-device QA is claimed by this report.
