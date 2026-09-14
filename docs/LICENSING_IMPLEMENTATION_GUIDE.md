# LeoMiniGames Licensing Suite — Implementation Guide

**Version:** 1.0  
**Date:** 10 September 2026

## 1. Target legal architecture

```text
LeoMiniGames host/core
    GPL-3.0-or-later
    + separate Plugin / Mod Exception 1.0 additional permission

Independent package
    ├─ standard open-source license
    ├─ standard content license for pure creative assets where appropriate
    └─ LicenseRef-YoungLion-Mod-License-1.0 for proprietary packages

Publisher ↔ YoungLion market/backend
    YoungLion Developer / Publisher Distribution Terms

Names/logos/badges
    YoungLion / LeoMiniGames Trademark and Trust-Mark Policy
```

These layers must remain separate.

## 2. Repository root `LICENSE`

**Do not modify the standard GNU GPL v3 text.**

If the repository currently uses the canonical GPLv3 text and the project notice selects “version 3 or later”, preserve that text exactly.

The `<year>` / `<name of author>` language at the end of GNU’s license file is part of GNU’s “How to Apply These Terms to Your New Programs” guidance and is not an unresolved placeholder that should be filled inside the canonical license text.

Project-specific notices belong outside the GNU GPL text.

Recommended structure:

```text
/LICENSE                                  # canonical GNU GPL v3 text, unchanged
/NOTICE                                   # project-specific notices
/COPYRIGHT                                # copyright holder/contributor information
/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt
/licenses/YOUNGLION_MOD_LICENSE_1.0.txt
/licenses/README.md
/docs/DEVELOPER_LICENSING.md
/docs/OPEN_SOURCE_LICENSE_POLICY.md
/docs/TRADEMARK_POLICY.md
/docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md
```

## 3. `NOTICE`

Recommended project-specific notice:

```text
LeoMiniGames

LeoMiniGames is free software licensed under GPL-3.0-or-later.
The canonical GNU GPL license text is in /LICENSE.

Qualifying independently developed packages that interact with LeoMiniGames
solely through designated public interfaces may use the additional permission
in /licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt.

Third-party packages retain their own licenses. Proprietary packages may use
LicenseRef-YoungLion-Mod-License-1.0 when eligible.

YoungLion/LeoMiniGames names, logos and trust badges are governed separately
from the source-code license. See /docs/TRADEMARK_POLICY.md.
```

Do not put extra restrictions on GPL rights in `NOTICE`.

## 4. `COPYRIGHT`

The file should accurately list current copyright holders rather than claiming all contributor copyrights belong to YoungLion if they do not.

Example structure:

```text
LeoMiniGames
Copyright (c) 2026 Cavanşir Qurbanzadə
Copyright (c) respective contributors for their contributions

See repository history for authorship information.
```

If ownership changes to a company/organization later, update prospectively and document any actual assignments. Do not retroactively state that contributor copyrights were assigned unless there is a valid assignment.

## 5. `CONTRIBUTING.md`

### Recommended current model: no CLA by default

Use:

1. inbound = outbound contribution licensing;
2. explicit contribution notice covering the Plugin Exception; and
3. DCO sign-off for provenance.

Recommended text:

```text
By submitting a contribution to the GPL-covered LeoMiniGames host, you agree
that your contribution is licensed under GPL-3.0-or-later and, for copyright
you control in that contribution, you also grant the additional permission
contained in LeoMiniGames Plugin / Mod Exception 1.0.

You certify the contribution under the Developer Certificate of Origin (DCO)
by adding a Signed-off-by line to each commit.
```

Why not require a CLA immediately?

- GPL + explicit inbound terms are simpler for community contributors.
- DCO records provenance/right-to-contribute without taking ownership.
- A broad CLA can deter contributors and is unnecessary if YoungLion does not need proprietary relicensing of the host.

### When a CLA becomes worth considering

Consider a narrowly drafted CLA if YoungLion later requires the ability to:

- grant materially broader exceptions over contributed code without seeking each contributor’s consent;
- dual-license the contributed host implementation under proprietary terms; or
- centrally enforce/relicense contributor copyrights beyond the inbound GPL + Exception grant.

A future CLA should not be introduced casually or retroactively represented as covering old contributions.

## 6. Changing the Plugin Exception later

The current exception may only be granted over copyright a licensor controls.

If third-party contributors grant Exception v1.0 to their contributions, YoungLion cannot automatically grant a materially broader v2.0 additional permission over those contributions unless the contributor has already granted authority to do so or later consents.

Options for a future broader exception:

- obtain contributor consent;
- apply the new exception only to code for which YoungLion has sufficient rights;
- keep old contributed code under the older exception; or
- adopt a contributor agreement prospectively.

## 7. Public SDK / `src/sdk/`

### `src/sdk/IGamePlugin.h` and similar public interface-only files

Recommended if YoungLion owns the relevant copyright:

```text
SPDX-License-Identifier: MIT OR GPL-3.0-or-later
```

Ship the canonical MIT text in the appropriate license directory if MIT is used.

The public header should contain declarations and minimal interface-only helpers, not copied host implementation.

### Internal headers

Keep private/internal LeoMiniGames headers under the host’s GPL licensing. Do not expose them as the supported proprietary plugin interface.

### Public Native/L3 ABI

Create a clearly separated directory, for example:

```text
src/sdk/public/
  IGamePlugin.h
  LeoPluginAbi.h
  LeoPluginDescriptor.h
```

Only these designated files form the public native boundary.

Private implementation remains outside this directory.

## 8. `mod-sdk/` and `theme-sdk/`

Recommended:

```text
mod-sdk/templates/       MIT
mod-sdk/examples/        MIT (unless an example intentionally demonstrates GPL)
theme-sdk/templates/     MIT
theme-sdk/examples/      MIT for code; assets separately licensed if necessary
```

Each generated project should contain its own license selection placeholder/instructions rather than silently inheriting MIT for the developer’s entire new Package.

Example generated manifest:

```json
{
  "license": "MIT",
  "license_file": "LICENSE",
  "source_available": true,
  "source_url": "https://example.invalid/source"
}
```

The template code can be MIT while a developer replaces the generated project license with another allowed license for their own work, provided all copied components permit it.

## 9. Manifest schema

Add licensing fields **additively** and keep them optional for legacy runtime parsing:

```json
{
  "license": "MIT",
  "license_file": "LICENSE",
  "source_available": true,
  "source_url": "https://example.invalid/source"
}
```

Recommended schema semantics:

### `license`

String. For standard software licenses, accept recognized SPDX expressions according to the market allowlist/parser policy.

For the YoungLion proprietary license use:

`LicenseRef-YoungLion-Mod-License-1.0`

### `license_file`

Relative package path to the applicable license text/notice. Reject path traversal and remote URLs in this field.

### `source_available`

Boolean informational metadata. It does not establish an open-source license.

### `source_url`

Optional HTTPS URL to corresponding/source repository when applicable. For GPL-style submissions, market validation should ensure the actual source-delivery method satisfies the selected license; a boolean alone is not enough.

### optional future fields

Consider additive fields such as:

- `third_party_notices`
- `copyright_file`
- `source_distribution`

but do not block legacy runtime compatibility on their absence.

## 10. Trust/capability fields

Do not treat the following as license data:

```text
official
verified
developer
native
native_l3_allowed
plugin_level
```

Recommended server model:

```text
manifest request:
    desired plugin_level / required capabilities

backend authority:
    publisher_type
    publisher_verified
    can_publish_level3
    can_publish_native
    native_platforms
    review_required

catalog response:
    trusted status rendered by client
```

If legacy packages contain `official` or `verified`, parse them only for compatibility/debug display if necessary; never use them as trust authority.

## 11. New submission policy vs runtime compatibility

### Runtime

LeoMiniGames v0.5/v0.6 packages without new license metadata must continue to load if they otherwise satisfy runtime compatibility requirements.

Suggested runtime classification:

```text
license_state = legacy_unknown
```

Do not fail package mounting merely because `license` is absent.

### Market submission

New submissions and new versions should require current license metadata.

Suggested rule:

```text
legacy installed package + missing license  -> runtime allowed
legacy catalog entry + missing license      -> display legacy/unknown warning
new publish/finalize + missing license      -> submission validation error
new version of legacy package               -> require license metadata
```

This cleanly separates compatibility from publication policy.

## 12. Market/backend enforcement

Backend finalize/review should validate:

- declared license is supported/recognized;
- required LICENSE/notice file exists;
- source metadata is coherent with selected license;
- proprietary license is not used to suppress detected incompatible open-source obligations;
- Package Publisher owns or controls the submission;
- trust status comes from backend records;
- Native/L3 permission is backend-authoritative;
- hash/size/object key are bound to the authenticated upload ticket.

License validation is a compliance aid, not a legal guarantee. Complex dependency trees may require human review.

## 13. Developer portal

Display separately:

```text
License
  MIT / GPL-3.0-or-later / LicenseRef-YoungLion-Mod-License-1.0 / ...

Source
  Available / Not declared / URL

Publisher trust
  Official / Native-L3 Verified / Verified / Unverified

Plugin capability
  L1 / L2 / requested L3 / approved L3
```

Never visually merge “Verified” with “Open Source”. They answer different questions.

Require Publisher Terms acceptance using a versioned acceptance record before first publication and when legally material changes require re-acceptance.

## 14. `docs/DEVELOPER_LICENSING.md`

Document four independent questions for developers:

1. What is LeoMiniGames host licensed under? → GPL-3.0-or-later.
2. May my independent package use another license? → yes if it qualifies under the Plugin Exception and you have the necessary rights.
3. What license should my own Package use? → standard OSS license or YoungLion proprietary license according to intent.
4. Does a license grant Verified/Native/Official access? → no; those are backend trust/capability decisions.

## 15. F-Droid considerations

The host’s GPL license remains FOSS-compatible in principle.

A build that directly promotes/downloads proprietary add-ons may be treated differently by F-Droid policy than a build restricted to FOSS packages. Maintain the ability to produce an F-Droid-oriented catalog/build policy without changing the licensing of the core.

Do not mislabel a proprietary Package as FOSS merely because the host is GPL.

## 16. Existing files to change

### `LICENSE`

Keep canonical GNU GPL v3 text unchanged.

### `COPYRIGHT`

Correctly identify actual copyright holders. Do not claim assignments that do not exist.

### `NOTICE`

Add project-specific GPL selection, Plugin Exception reference, third-party notice pointers and trademark separation. Do not add GPL-incompatible restrictions.

### `CONTRIBUTING.md`

Add explicit inbound GPL + Plugin Exception grant and recommended DCO process.

### `docs/DEVELOPER_LICENSING.md`

Replace placeholder/partial guidance with the four-layer licensing architecture.

### `licenses/README.md`

List:

- GPL host license location;
- Plugin Exception;
- YoungLion proprietary Package license;
- supported standard open-source licenses;
- third-party notice rules.

### `mod-sdk/`

License templates/examples permissively (recommended MIT), add license metadata to generated manifest examples, and include developer license-selection guidance.

### `theme-sdk/`

Same as mod SDK; license code separately from example creative assets where needed.

### `src/sdk/`

Separate designated public interfaces from private/internal headers. Dual-license narrow public interface files `MIT OR GPL-3.0-or-later` where rights permit.

### manifest schema

Add `license`, `license_file`, `source_available`, `source_url` as backward-compatible optional runtime fields; require them through new submission policy as applicable.

### market documentation

Document supported licenses, proprietary license identifier, source obligations, Publisher Terms, trust separation and legacy policy.

### developer portal

Add license picker, LICENSE preview, source metadata, Publisher Terms acceptance/version, third-party notices and separate trust/capability status.

## 17. Files not to conflate

```text
LICENSE
  = LeoMiniGames GPL text

LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt
  = GPL additional permission

YOUNGLION_MOD_LICENSE_1.0.txt
  = proprietary end-user copyright license for eligible Packages

YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md
  = Publisher ↔ YoungLion service/distribution contract

TRADEMARK_POLICY.md
  = names/logos/badges
```

## 18. Release checklist

Before public rollout:

- [ ] confirm current host copyright holders;
- [ ] ensure YoungLion has authority to grant the Plugin Exception over existing host code;
- [ ] verify third-party GPL code has not been imported without equivalent exception rights where proprietary plugin compatibility depends on it;
- [ ] keep GNU GPL text unchanged;
- [ ] add Exception reference to project notices;
- [ ] license public SDK headers intentionally;
- [ ] add contribution licensing notice;
- [ ] implement backend license validation;
- [ ] preserve legacy runtime compatibility;
- [ ] version Publisher Terms acceptance;
- [ ] keep trust data backend-authoritative;
- [ ] perform qualified legal review for Native/L3 linking boundary before commercial reliance;
- [ ] perform qualified legal review before material paid marketplace launch.
