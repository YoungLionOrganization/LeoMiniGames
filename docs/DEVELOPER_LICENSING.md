# Developer and package licensing policy

## 1. Host application

LeoMiniGames itself is distributed under **GPL-3.0-or-later**. The file
`LICENSE` is the unmodified GNU GPL v3 text. Project-specific notices belong in
`COPYRIGHT`, `NOTICE` and this document, not inside the GNU license text.

## 2. Package license and developer permission are different

A game's/mod's/theme's copyright license answers **what recipients may do with
the package**. Developer/account capability answers **what the LeoMiniGames
service allows that publisher to do**. Do not merge the two. In particular:

- `verified` is not Native/L3 permission;
- Official/Verified status is backend-authoritative and cannot be self-declared;
- Developer Mode or a scoped `lmg_...` key does not change copyright rights;
- Native/L3 approval does not make a package open source;
- an open-source license does not automatically grant Native/L3 approval.

## 3. Open-source games, mods and themes

Use a recognized license and keep its official text intact. The manifest should
use the canonical SPDX identifier where possible, ship a `LICENSE`/license file,
and provide a source URL when source disclosure or project policy requires it.

Selection guidance:

- **MIT / BSD-3-Clause** — permissive; downstream proprietary reuse is allowed.
- **Apache-2.0** — permissive with an explicit patent grant and NOTICE duties.
- **MPL-2.0** — file-level copyleft; useful when modified covered files should
  stay open but larger combined works may use different terms.
- **LGPL-3.0-or-later** — library-oriented weak copyleft; use only when the
  package architecture actually fits LGPL linking/compliance requirements.
- **GPL-3.0-or-later** — strong copyleft; suitable when redistributed derivatives
  should remain GPL-compatible and corresponding source must be provided.

Do not invent a custom "open-source" license if an OSI/FSF-standard license
already expresses the intended rights.

## 4. Closed-source packages

The repository's `YOUNGLION_MOD_LICENSE.txt` is a placeholder, not final terms.
The intended proprietary package license should be written and legally reviewed
before use. At minimum it should define:

- the licensor/publisher and covered package;
- end-user install/run and reasonable personal backup rights;
- redistribution, mirroring, resale and sublicensing rules;
- modification/reverse-engineering restrictions only to the extent permitted by
  mandatory law;
- ownership of the package and third-party materials;
- update/service termination rules;
- warranty and liability limitations appropriate to applicable law;
- governing-law/jurisdiction choices after legal review;
- a limited license to YoungLion to host, copy, cache, validate, security-scan,
  hash/sign, package, display metadata, distribute, update and enable redownload
  of the submitted package through LeoMiniGames services.

That YoungLion service license should be non-exclusive and limited to operating,
securing and distributing the submitted package; it should not silently transfer
ownership of a developer's work.

## 5. GPL boundary / future Plugin & Mod Exception

A dedicated additional permission should be reviewed before promising that every
closed-source native plugin can link against GPL-covered core code. The intended
boundary is:

- independent RCC/QML/data packages use documented public host services;
- package authors retain ownership and may select their package license;
- a future exception may expressly allow separately distributed independent
  packages to use designated public SDK interfaces without inheriting the host's
  GPL solely from that interface use;
- Native/L3 coverage, if desired, must be limited to a clearly designated public
  ABI/API. Private/internal headers and copied core implementation code should not
  receive an accidental proprietary exception;
- distribution of the LeoMiniGames core itself remains subject to GPL obligations.

Have the final exception reviewed together with the proprietary mod license; the
two documents solve different problems.

## 6. Manifest policy

Modern manifests should distinguish licensing metadata from trust/capability
metadata. Recommended licensing concepts are:

```json
{
  "license": "MIT",
  "license_file": "LICENSE",
  "source_available": true,
  "source_url": "https://example.invalid/source"
}
```

For a future YoungLion proprietary license, use its final canonical identifier
only after the placeholder has been replaced. Do not use `source_available=false`
as a permission flag; it is metadata only.

## 7. Contribution licensing

Contributors to the GPL host should understand the inbound contribution terms.
Before accepting substantial third-party contributions, decide whether YoungLion
will use GPL-only inbound terms, a contributor agreement, or another documented
policy if relicensing/plugin-exception authority is important. Do not assume a
GitHub pull request automatically assigns copyright to YoungLion.

> This policy is project engineering guidance, not jurisdiction-specific legal
> advice. Final proprietary licenses, trademark terms and GPL additional
> permissions should be reviewed by qualified counsel before commercial reliance.
