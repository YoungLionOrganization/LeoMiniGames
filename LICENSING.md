# LeoMiniGames licensing application

The LeoMiniGames host application is licensed under **GNU GPL version 3 or, at
your option, any later version (`GPL-3.0-or-later`)**.

The repository deliberately separates the canonical GPL text from the
project-specific application statement:

- `/LICENSE` — canonical GNU GPLv3 text, kept verbatim.
- `/LICENSE_APPLICATION.md` — the actual LeoMiniGames program/copyright/license
  application statement and distribution checklist.
- `/COPYRIGHT` — project copyright holder/contributor notice.
- `/NOTICE` — concise distribution notice and cross-references.

The `<year>` and `<name of author>` text near the end of `/LICENSE` is part of
GNU's own “How to Apply” example. It is not an unfinished LeoMiniGames field and
must not be replaced inside the canonical GPL text.

## Plugin / Mod Exception

LeoMiniGames grants the separate GPLv3 section 7 permission in
`/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` to packages that satisfy that
document's definition of an Independent Package and use only designated public
LeoMiniGames interfaces. The exception does not turn internal/private host APIs
into a proprietary SDK and does not grant Official/Verified/Native/L3,
trademark, signing, marketplace, or platform rights.

## Package licensing

Eligible proprietary packages can separately select
`LicenseRef-YoungLion-Mod-License-1.0`; the complete terms are in
`/licenses/YOUNGLION_MOD_LICENSE_1.0.txt`.

Open-source packages should normally use standard SPDX licenses under
`/docs/OPEN_SOURCE_LICENSE_POLICY.md`.

Publisher distribution-service terms and trademark policy are separate from
copyright licensing. See
`/docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md` and
`/docs/TRADEMARK_POLICY.md`.

Official release packaging must carry the legal files listed by
`/LICENSE_APPLICATION.md`; CI validation treats omission of those files as a
distribution defect.
