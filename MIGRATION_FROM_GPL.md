# Migration from GPL-labelled Development State

Target baseline reviewed:
`YoungLionOrganization/LeoMiniGames`
main @ `7537efcc40750555121edf607d5fc862a9b50bc4`

## 1. Replace current legal files

Replace:
- `LICENSE`
- `NOTICE`
- `COPYRIGHT`
- `LICENSING.md`
- `CONTRIBUTING.md`

Add:
- `LICENSE_HISTORY.md`
- `CONTRIBUTOR_LICENSE_AGREEMENT.md`
- `LICENSE_METADATA.json`
- `LEGAL_CHANGELOG.md`
- new files under `licenses/` and `docs/`

## 2. Remove obsolete current-host GPL architecture

The following current concepts become obsolete for NEW editions:
- host `GPL-3.0-or-later`;
- `LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` as current host permission;
- statements that LeoMiniGames host is free software;
- current F-Droid-main eligibility statements.

Historical copies can be preserved in repository history or an archival
`licenses/history/` directory with explicit "historical/not current" labels.

## 3. SPDX/header migration

Project-owned host files currently marked:
`SPDX-License-Identifier: GPL-3.0-or-later`

should be changed, where the Project Owner has the required rights, to:
`SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0`

Do NOT mechanically replace headers in:
- third-party code;
- files with different rightsholders;
- files intentionally designated SDK/template material;
- license texts themselves.

Designated Publisher-facing SDK/template files should use:
`SPDX-License-Identifier: LicenseRef-YoungLion-LMG-SDK-1.0`

## 4. Current SDK issue

`src/sdk/IGamePlugin.h` currently says:
`MIT OR GPL-3.0-or-later`

If the goal is a fully YoungLion-custom licensing model, replace it only after
confirming Project Owner rights with:
`LicenseRef-YoungLion-LMG-SDK-1.0`

`src/sdk/GameLocalStats.h` currently identifies itself as GPL-only and as an
internal helper. It should normally become host-licensed
`LicenseRef-LMG-SAPEL-1.0`, not public SDK material.

## 5. Publisher package manifests

For new Publisher Packages choosing the YoungLion proprietary license:

```json
{
  "license": "LicenseRef-YoungLion-Publisher-Package-1.0",
  "license_file": "LICENSE"
}
```

Packages using standard OSS licenses keep their own SPDX license metadata.

## 6. README/docs

Remove statements saying the current host is GPL/open source/free software.

Use:
"LeoMiniGames is source-available under LicenseRef-LMG-SAPEL-1.0."

## 7. F-Droid

Remove official-main-repo readiness claims. See `docs/F_DROID_TRANSITION.md`.

## 8. Qt

Do not remove Qt/LGPL notices. The host relicense does not relicense Qt.

Before a binary release, complete `docs/QT_LGPL_COMPLIANCE.md`.

## 9. Git history

Do not claim that changing today's LICENSE retroactively revokes old GPL grants.

Rewriting Git history may change what is conveniently visible, but it cannot
erase copies or rights already validly received by third parties.

## 10. Release gate

Do not publish the first source-available binary release until:
- project-owned copyright scope is confirmed;
- third-party dependency audit passes;
- source headers are migrated;
- README/site/store metadata matches the new license;
- Publisher Terms acceptance is implemented for publication;
- Qt/LGPL compliance is documented for each binary platform.
