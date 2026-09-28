# LeoMiniGames v0.7.1 — Windows Installer Audit

Date: 2026-09-24

## Reported regressions

1. QtIFW visual layout was cramped and duplicated branding.
2. Windows setup exposed too few install choices.
3. Release artifacts did not distinguish a modern x86_64 optimized CPU profile.
4. Running Setup over an existing installation did not present maintenance actions.
5. The Ready page could show an empty “You are installing” area.

## Implemented architecture

- Classic QtIFW wizard + dark QSS + one page list; no `Logo` or `PageListPixmap` in config.
- Controller script detects installed LeoMiniGames and dispatches Update / Modify / Uninstall to the installed Maintenance Tool; Repair/Reinstall stays in Setup.
- Component custom pages provide existing-install actions and optional Desktop/Start Menu/Maintenance shortcuts.
- Ready page explicitly selects the application component and populates `InstallMsgLabel`.
- Windows artifacts: x86_64 baseline, x86_64 AVX2 (`/arch:AVX2`), ARM64.
- AVX2 update identity is preserved end-to-end (`x86_64-avx2` client contract, `x86_64-AVX2` repository directory).
- Win32/x86 is rejected as unsupported by this Qt 6 release setup instead of being mislabeled as supported.

## Validation boundary

XML, scripts, workflow structure, package policy, update routing and source validators can be checked here. A real QtIFW Windows GUI was not executed in this environment; final layout/DPI/repair behavior requires the next Windows-generated Setup EXE smoke test.
