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
