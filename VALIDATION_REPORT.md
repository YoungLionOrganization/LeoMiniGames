# LeoMiniGames v0.7.1 — Validation Report

Date: 2026-09-22
Release candidate commit: `9943b5c1824b81fad87f2d67741fefd714c869f6`

## Scope

This report records the v0.7.1 release/update servicing state: cross-platform CI, release artifact generation, guarded release preflight, Windows QtIFW update infrastructure and the Apple-mobile workflow regression fixed in `UpdateService`. It does not replace the historical v0.7.0 compatibility/security audits, which remain versioned evidence for that baseline.

## GitHub Actions evidence

For the exact release-candidate SHA above:

- **LeoMiniGames CI #21** — completed successfully.
- **Build Release Artifacts #9** — completed successfully.
- **Publish Release #1 / Release preflight** — completed successfully.
- **Publish Release #1 / Publish approved release** — skipped intentionally because that run used validation mode rather than publish mode.

The successful Build Release Artifacts run produced the source artifact, Windows x86_64/ARM64 artifacts, Ubuntu/Debian/Arch Linux artifacts, macOS arm64/x86_64/universal artifacts, all configured Android artifacts, iOS/iPadOS device/simulator unsigned artifacts, `release-assets`, and `update-repositories`.

## Resolved Apple-mobile workflow failure

The previous v0.7.1 attempt compiled the Windows Maintenance Tool launch path on iOS/iPadOS. Apple mobile builds failed at `UpdateService.cpp` because `QProcess::startDetached()` is not available there. The fix makes the `QProcess` include and Maintenance Tool process launch Windows-only and also uses `qsizetype` for the prerelease-list comparison index/count.

The current exact-SHA CI and artifact workflows pass after that correction. `validate_v071.py` includes a regression guard so Windows-only process-launch code is caught during static validation rather than first appearing in Apple build jobs.

## Release pipeline validation

`Publish Release` preflight verified:

- workflow SHA equals current `main` HEAD;
- required successful CI and Build Release Artifacts runs exist for the exact SHA;
- canonical release metadata is valid;
- `release-assets` passes the public allowlist validator;
- Windows x86_64 and ARM64 QtIFW repositories contain `Updates.xml`;
- no conflicting release/tag existed at preflight time.

Actual public release publication is a separate destructive step and was not performed by Publish Release #1 because `mode=validate` was selected.

## QA honesty

A green GitHub workflow proves compilation/packaging on its configured hosted runner and validates the release contracts. It does not prove real-device UX, audio/haptics, background/foreground behavior, platform signing/notarization or store acceptance. Historical `GAMER_AUDIT.md` and related reports therefore remain explicit about physical-device limitations.

## Next release action

Run **Publish Release** from `main` with `mode=publish`. If GitHub places **Publish approved release** in a waiting state, approve the `release` Environment through **Review deployments**. See `docs/GITHUB_RELEASES.md`.


## 2026-09-22 follow-up: workflow and Developer Lab

Latest inspected baseline commit before this package: `d22534e2170948f2ab6c0f8b333cec5380e88f6a`.

- CI #24 (`35752864666`): failed only in Android x86_64 packaging because the external Gradle distribution request returned HTTP 500. Static validation, Windows, Linux, macOS, Apple-mobile, the other Android ABIs and Android universal lanes succeeded.
- Build Release Artifacts #11 (`35753216167`): successful for the same exact SHA.
- Publish Release #4 (`35755442043`): preflight correctly failed because there was no successful exact-SHA CI run; publication was not allowed to bypass the release gate.
- Developer Lab audit found that `/api/v1/developer/contents` was incorrectly used as an authentication probe. The client now uses `/api/v1/developer/auth/verify`, and the backend supplies a dedicated no-store verification contract with robust bearer-header extraction for cPanel/FastCGI.
- Android jobs now use a bounded, signature-based retry helper for transient Gradle/network failures. Non-network build failures are not retried or suppressed.

Post-fix local/static validation results are recorded by this package's final validation run. A new GitHub-hosted exact-SHA CI result requires the patched source to be pushed; no such post-fix runner result is claimed in this report.

## Final local regression validation for this delivery

The final working tree was revalidated after the workflow and Developer Lab fixes:

- `tools/source_guard.py`: PASS
- `tools/validate_project.py`: PASS
- `tools/sanity_check.py`: PASS
- `tools/security_audit.py`: **16 PASS / 0 WARNING / 0 ERROR**
- `tools/validate_i18n.py`: **0 ERROR / 22 coverage warnings**; source-text fallback remains intentional
- `tools/validate_distribution.py`: **245 PASS / 0 WARNING / 0 ERROR**
- `tools/audit_prebuilt_binaries.py`: PASS
- `tools/validate_v071.py`: PASS
- all client shell scripts: `bash -n` PASS
- `ci.yml`, `build-artifacts.yml`, `publish-release.yml`: YAML parse PASS
- Android transient-retry controlled test: two simulated Gradle-distribution HTTP 500 failures followed by success on attempt 3: PASS
- Android non-network build-error controlled test: simulated C++ compile failure exited on attempt 1 without retry: PASS

Backend validation for the matching delivery:

- all backend PHP sources: `php -l` PASS
- `tools/test_developer_lab_auth.php`: PASS
- `tools/validate_developer_lab.php`: PASS
- `tools/test_update_gateway.php`: PASS
- `tools/validate_update_gateway.php`: PASS

No production Developer API key was used and no physical-device QA is claimed. A post-fix GitHub-hosted CI result can only exist after this working tree is committed and pushed.

## 2026-09-22 follow-up: Build Release Artifacts #12

Latest inspected source commit: `2fbd4b7ffc9259c5d3171ff5247d9393e1fa2bbb`.

- LeoMiniGames CI #25 (`35757266231`) completed successfully for the exact SHA.
- Build Release Artifacts #12 (`35757409242`) reached successful validation and successful Windows, Linux, macOS and most Apple-mobile packaging lanes, but the Android release jobs failed before Gradle execution.
- The common Android failure was not C++/QML, Gradle, signing or NDK compilation. `tools/package/package_android.sh` required `tools/ci/android_build_with_retry.sh` to have executable permission (`-x`). The exact Git tree stores that helper as a normal `100644` file, so all release Android jobs exited with code 65 despite the helper being present.

The packaging fix uses a normal-file check (`-f`) and invokes the helper through `bash` for both `apk` and `aab`. `validate_v071.py` now enforces this contract so a file-mode regression is caught during the source-validation job instead of in five parallel Android packaging jobs.

A new GitHub-hosted Build Release Artifacts result requires this patched tree to be pushed; this report does not claim an unobserved post-fix hosted-run result.
