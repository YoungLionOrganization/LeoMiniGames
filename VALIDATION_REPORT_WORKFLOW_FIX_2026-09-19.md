# LeoMiniGames v0.7.0 — Workflow/Artifact Fix Validation Report

Baseline GitHub commit: `54ee7c0062e1e905c1c4826a01c61b4ca46fa791`.
Final local verification date: 2026-09-19.

## Real GitHub Actions failures addressed

- Debian 13 amd64/arm64: Qt QML private headers were missing; workflow now installs `qt6-declarative-private-dev`.
- Arch ARM64: the official `archlinux:latest` container has no `linux/arm64/v8` manifest; unsupported official Arch ARM64 lane removed. Arch x86_64 remains; Linux ARM64 remains covered by Ubuntu 24.04 ARM64 and Debian 13 ARM64.
- iOS/iPadOS ARM64 simulator: Qt 6.10.2 online iOS package contains device-arm64 Qt plugin objects and x86_64 simulator FFmpeg slices; unsupported ARM64 simulator lanes removed. Device ARM64 and simulator x86_64 remain.
- Windows QtIFW 4.11: `<Default>` and `<Checkable>` were mutually exclusive. `<Default>` removed while `ForcedInstallation=true`, `Essential=true`, `Checkable=false` remain.
- Ubuntu 24.04 ARM64 AppImage: `linuxdeploy-plugin-qt` could not resolve `libgstplay-1.0.so.0`; portable workflow now installs `libgstreamer-plugins-bad1.0-0`.

## Final local validation

The final tree was rebuilt from the exact source artifact produced by GitHub Actions run `35253271428`, then only the workflow/package/validator fixes above were applied.

Passed checks:

- `tools/source_guard.py`
- `tools/validate_project.py`
- `tools/sanity_check.py`
- `tools/security_audit.py`: 16 PASS, 0 WARNING, 0 ERROR
- `tools/validate_i18n.py`: 0 ERROR; existing translation-coverage warnings remain
- `tools/validate_distribution.py`: 220 PASS, 1 expected F-Droid screenshot warning, 0 ERROR
- `tools/audit_prebuilt_binaries.py`
- `tools/validate_v070.py`: 51 PASS, 0 WARNING, 0 ERROR
- `python3 -m py_compile tools/*.py`
- `bash -n` for every `tools/**/*.sh`
- YAML parse for both workflows
- QtIFW `package.xml` parse and selection-contract assertions
- workflow regression assertions for Debian private headers, ARM64 AppImage GStreamer runtime, removed unsupported Arch ARM64 / Apple ARM64-simulator lanes

The source-packaging script reruns the project validators before creating the final ZIP.

## External runner note

The ChatGPT GitHub integration can read the repository and Actions logs but currently returns `403 Resource not accessible by integration` for repository write operations. Therefore this package could not be pushed from this connection for a post-fix GitHub-hosted rerun. The fixes are grounded in the actual failed job logs and locally revalidated, but the report intentionally does not claim a post-fix all-green GitHub Actions run.
