# LeoMiniGames Open-Source License Policy

**Version:** 1.0  
**Date:** 10 September 2026

## 1. Core rule

YoungLion should not create a custom “YoungLion Open Source License”. For open-source software, use established standard licenses and preserve their canonical text.

The LeoMiniGames host remains `GPL-3.0-or-later`. The GNU GPL text in the repository `LICENSE` file should remain unmodified.

The separate `LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` is an additional permission under GPLv3 section 7. It should not be pasted into or used to rewrite the GNU GPL text.

## 2. Decision matrix

| License | Typical choice | Proprietary reuse | Copyleft | Source disclosure on distribution | Express patent grant | NOTICE / attribution | RCC package fit | Native/L3 caution |
|---|---|---|---|---|---|---|---|---|
| MIT | Small SDKs, templates, examples, permissive mods | Yes | None | No | No explicit patent license | Preserve copyright/license notice | Excellent | Good for public ABI headers; does not itself solve GPL host/plugin boundary |
| BSD-3-Clause | Permissive code where non-endorsement clause is useful | Yes | None | No | No explicit patent license | Preserve license; no endorsement using names | Excellent | Similar to MIT; protect names separately through trademark policy |
| Apache-2.0 | Larger reusable SDK/library where explicit patent terms matter | Yes | None | No general source requirement | Yes, express patent grant with termination provisions | Preserve required notices; NOTICE handling if present | Excellent | Strong choice for independent SDK libraries; verify GPL compatibility for exact combination/version |
| MPL-2.0 | File-level copyleft desired | Yes, around MPL files | File-level / weak | Modified MPL-covered files generally remain source-available when distributed | Yes | Preserve MPL notices; no Apache-style mandatory NOTICE file | Good | Good for separable modules; keep MPL-covered files cleanly separated from proprietary native implementation |
| LGPL-3.0-or-later | Reusable library where modifications to library should stay free | Generally yes if LGPL conditions are met | Library-level / weak | LGPL library/modifications source and user relinking/replacement rights as applicable | GPLv3-family patent provisions | Preserve license/notices | Possible but often awkward for self-contained RCC mods | Native linking obligations matter; design for replacement/relinking and lawful reverse engineering for debugging modifications |
| GPL-3.0-or-later | Fully free/open mod/game where strong copyleft is intended | Generally not for a combined proprietary derivative | Strong | Corresponding Source required when conveying covered object code, subject to GPL terms | Yes | Preserve GPL notices/license/source obligations | Excellent for intentionally GPL mods | Strong-copyleft analysis becomes important when combined/linked; Plugin Exception affects host boundary, not third-party GPL obligations |

## 3. Practical recommendations

### MIT

Use for:

- mod SDK templates;
- theme SDK templates;
- minimal examples;
- small helper libraries intended for the broadest reuse;
- public C/C++ interface declarations when YoungLion wants proprietary plugins to include them.

Do not use MIT as a substitute for a content license for standalone artwork/audio if a Creative Commons license is more appropriate.

### BSD-3-Clause

Use where MIT-like permissiveness is desired but an explicit non-endorsement condition is useful.

Do not rely on BSD’s non-endorsement language as the entire YoungLion trademark strategy; keep the separate Trademark Policy.

### Apache-2.0

Use for larger independent SDK/tooling components when an explicit patent license is desirable.

If an Apache-licensed component has a NOTICE file, preserve notices as the license requires.

### MPL-2.0

Use where YoungLion or a Publisher wants modifications to particular files to remain open while permitting those files to coexist with proprietary files.

Keep file boundaries meaningful. Avoid copying MPL code into proprietary files in a way that defeats clear compliance.

### LGPL-3.0-or-later

Use primarily for actual libraries, not as a default license for ordinary RCC games/themes.

A proprietary program may be able to use an LGPL library subject to LGPL conditions, including the relevant user modification/relinking rights. Native packaging must be designed accordingly.

### GPL-3.0-or-later

Use when the Publisher intentionally wants strong copyleft and Source Code availability for redistributed derivatives/covered combinations.

A GPL mod should not additionally be subjected to the proprietary YoungLion Mod License restrictions.

## 4. Pure creative assets

For standalone artwork, sound, music, documentation or other primarily creative assets, software licenses are often not the best fit.

Recommended standard options should be evaluated by content type, for example:

- CC0-1.0 for near-public-domain dedication;
- CC-BY-4.0 for attribution-based reuse; or
- CC-BY-SA-4.0 for share-alike creative content.

Do not automatically put code and creative assets under the same license unless that is intentional.

## 5. LeoMiniGames SDK recommendation

### Public interface-only headers

For narrowly scoped headers expressly designed to be included by third-party plugins, such as a designated public `IGamePlugin.h`, recommend:

`MIT OR GPL-3.0-or-later`

provided YoungLion/the relevant copyright holders own the necessary rights.

Why both?

- MIT gives proprietary plugins a clean right to copy/include declarations and small inline/interface-only material.
- GPL-3.0-or-later preserves a natural option for GPL projects.
- The separate LeoMiniGames Plugin Exception addresses the GPL host/plugin combination question.

The recommended architecture therefore uses **both** permissive public interface licensing **and** the GPL Plugin Exception.

### Public native/L3 ABI

Use the same `MIT OR GPL-3.0-or-later` rule for narrowly scoped ABI headers, descriptors and sample loader declarations intended for third-party inclusion.

Do **not** move LeoMiniGames core implementation, private headers or internal classes into the permissive SDK merely to allow proprietary native plugins.

### Mod SDK templates/examples

Recommend `MIT` for starter templates and minimal example code.

### Theme SDK templates/examples

Recommend `MIT` for code/templates. License sample art/assets separately if necessary.

### Schemas

JSON schemas and validator metadata may use MIT for simplicity when YoungLion wants broad tooling adoption.

## 6. GPL host + exception policy

The repository root `LICENSE` remains the unchanged GPLv3 license text.

Project-specific additional permission lives in:

`licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt`

Recommended project notices should say, in substance:

> LeoMiniGames is licensed under GPL-3.0-or-later. Qualifying independent packages may use the additional permission in the LeoMiniGames Plugin / Mod Exception 1.0.

Do not insert custom proprietary restrictions into the GNU GPL text.

## 7. Package license metadata

New submissions should use a standard SPDX identifier/expression where possible.

Examples:

```json
{
  "license": "MIT",
  "license_file": "LICENSE",
  "source_available": true,
  "source_url": "https://example.invalid/source"
}
```

```json
{
  "license": "GPL-3.0-or-later",
  "license_file": "LICENSE",
  "source_available": true,
  "source_url": "https://example.invalid/source"
}
```

For the YoungLion proprietary license:

```json
{
  "license": "LicenseRef-YoungLion-Mod-License-1.0",
  "license_file": "LICENSE",
  "source_available": false
}
```

`LicenseRef-...` is intentionally a project-defined reference, not an SPDX License List identifier.

## 8. Do not mix trust metadata with licensing

None of these fields establish copyright terms:

- `official`
- `verified`
- `developer`
- `native`
- `native_l3_allowed`
- `plugin_level`

They are backend-authoritative trust/capability data.

A package can be GPL and Official, GPL and Unverified, proprietary and Verified, or proprietary and Unverified. These are independent axes.

## 9. License validation rules

For **new market submissions**:

1. `license` required.
2. `license_file` required unless the platform can reliably provide the exact canonical standard license text by identifier and the submission rules expressly allow that model.
3. Open-source submissions must satisfy their source obligations independently of `source_available` metadata.
4. `source_available=true` does not itself make a Package open source.
5. `source_url` should be required when the selected license/source-delivery model requires public source and source is not bundled in the Package.
6. Third-party notices must be supplied when required.

For **runtime loading**, missing metadata in a legacy package is not a runtime compatibility failure by itself.

## 10. Qualified legal-review points

Seek jurisdiction-specific legal review before relying on a conclusion about:

- whether a particular native/dynamic plugin is a derivative/combined work under local copyright law;
- enforceability of reverse-engineering restrictions;
- patent consequences of a particular dependency combination;
- relicensing contributed code;
- consumer rights in paid content; or
- contributor contracting capacity for minors.
