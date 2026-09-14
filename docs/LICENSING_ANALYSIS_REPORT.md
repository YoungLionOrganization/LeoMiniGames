# LeoMiniGames Complete Licensing Suite — Analysis Report

**Version:** 1.0  
**Date:** 10 September 2026

## Executive conclusion

The recommended YoungLion/LeoMiniGames licensing architecture uses four separate legal layers:

1. **LeoMiniGames host:** `GPL-3.0-or-later`, with the GNU GPL text left unchanged.
2. **Plugin boundary:** LeoMiniGames Plugin / Mod Exception 1.0, a GPLv3 section 7 additional permission for qualifying independently developed Packages using designated public interfaces.
3. **Package license:** standard open-source/content license or `LicenseRef-YoungLion-Mod-License-1.0` for eligible proprietary Packages.
4. **Publisher service contract:** YoungLion Developer / Publisher Distribution Terms 1.0 governing hosting, R2/CDN, validation, scanning, signing, distribution, trust and market operations.

Trademark/trust marks form a fifth, separate policy layer because GPL copyright rights do not equal endorsement/brand rights.

## 1. Why the GNU GPL text is left untouched

The GNU GPL is a standardized license. Project-specific conditions should not be inserted into or used to rewrite the canonical GPL text.

The requested proprietary-plugin accommodation is instead implemented as an additional permission under GPLv3 section 7. This follows the structure GNU itself describes for exceptions/additional permissions.

The familiar `<year>` and `<name of author>` text at the end of GNU’s GPL distribution is part of the “How to Apply These Terms” instructional appendix. It is not a defect in the license file and should not be replaced inside the canonical GPL text.

## 2. Why a Plugin Exception is appropriate

LeoMiniGames is intentionally a plugin/mod host. The current architecture includes RCC/QML packages and a planned/implemented Native/L3 boundary.

Plugin/host copyright analysis can become fact-intensive when components dynamically link, exchange internal structures or depend on implementation details. A documented additional permission reduces uncertainty for the specific combinations the copyright holders intentionally want to permit.

The Exception is therefore deliberately limited to **Independent Packages** using **Public LeoMiniGames Interfaces**.

It does not say “anything called a plugin is proprietary-compatible”.

## 3. Why RCC/QML gets an explicit section

RCC/QML Packages may be loaded into the host process and can use host-provided services. The Exception clarifies that use of the documented RCC mount contract, public QML types and documented services is an intended third-party boundary.

The line is crossed when a Package copies host implementation or relies materially on private/internal objects.

## 4. Why Native/L3 is narrower

Native dynamic linking creates a higher-risk copyright and security boundary than declarative asset/RCC packaging.

The Exception therefore covers only separately built native modules that use a **designated public ABI** and excludes private headers/internal symbols and automatic static incorporation into the core executable.

This is intentionally conservative. Dynamic/native linking remains a point for qualified legal review in relevant jurisdictions.

## 5. Why public ABI headers should be dual-licensed

Recommended public interface-only headers: `MIT OR GPL-3.0-or-later`.

A proprietary plugin may need to copy/include declarations, inline adapters, constants or ABI structs. Permissive licensing of those narrow public interface files removes avoidable ambiguity about the header itself.

However, permissive header licensing does **not** resolve the GPL status of the host/plugin combination. The separate Plugin Exception addresses that question. Using both measures is clearer than relying on either alone.

## 6. Why core implementation stays GPL

The goal is not to make LeoMiniGames open-core in a way that lets proprietary forks absorb GPL core modifications.

Private headers, host services and implementation remain GPL. A core fork or copied host implementation remains subject to the GPL to the extent the GPL applies.

## 7. Why the proprietary Package license was narrowed

The earlier YoungLion proprietary mod draft combined end-user licensing with YoungLion market hosting/distribution rights. The complete suite separates those roles.

`YOUNGLION_MOD_LICENSE_1.0.txt` now governs **Publisher → User** copyright permissions.

`YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md` governs **Publisher → YoungLion Platform** service-operation permissions.

This is cleaner for:

- developer ownership;
- open-source Packages;
- market delisting;
- future commercial schedules; and
- legal interpretation.

## 8. Why the proprietary license covers package categories separately

Game code, themes, pure assets and native binaries create different expectations.

The license therefore distinguishes:

- Game/Mod execution and configuration;
- Theme application/customization;
- Asset extraction/reuse; and
- Native/L3 execution/trust.

One blanket rule would either be too restrictive for ordinary theme use or too permissive for standalone protected assets.

## 9. Reverse engineering clause

The proprietary license does not impose an absolute worldwide ban.

Some jurisdictions provide non-waivable rights or exceptions for interoperability, security research, accessibility or similar purposes. The clause therefore prohibits reverse engineering only to the extent enforceable and expressly preserves Mandatory Law.

This issue is jurisdiction-dependent and should receive legal review before enforcement strategy is finalized.

## 10. Why Publisher Terms use a limited platform license

YoungLion needs legal permission to operate R2/CDN and publishing workflows even when it does not own the content.

The platform license is therefore:

- worldwide;
- non-exclusive;
- royalty-free; and
- limited to service operation.

It covers hosting, copies, cache/CDN, backup, validation, security/malware scans, hashing, signing, metadata/catalog display, distribution, redownload, updates and narrowly necessary platform conversion.

It expressly does not assign IP ownership to YoungLion.

## 11. Open-source submissions and supplemental service rights

YoungLion should not demand a sweeping proprietary platform license over FOSS content when the FOSS license already grants the required rights.

The Publisher Terms therefore rely on the chosen FOSS license where sufficient, with the supplemental platform grant limited to genuinely separate submission materials/operations such as screenshots, pre-release uploads, entitlement records or signing.

## 12. Why license and trust metadata are separate

License answers:

> What may recipients do with this code/content?

Trust/capability answers:

> What does YoungLion authorize this Publisher/Package to do on this platform?

They are independent.

`official`, `verified`, `native_l3_allowed`, `plugin_level` and similar fields must remain backend-authoritative. A manifest is untrusted publisher input and cannot grant itself security status.

## 13. Legacy compatibility

The current LeoMiniGames design prioritizes compatibility with v0.5/v0.6 packages. A missing modern `license` field must therefore not become a runtime mount/load failure.

New **market submission** rules can nevertheless require the metadata.

This results in:

```text
runtime compatibility != market acceptance policy
```

That distinction avoids breaking installed content while improving compliance for future publishing.

## 14. Open-source license selection

The policy intentionally does not invent a “YoungLion Open Source License”.

- MIT/BSD: broad permissive reuse.
- Apache-2.0: permissive plus explicit patent terms.
- MPL-2.0: file-level copyleft.
- LGPL-3.0-or-later: library-oriented weak copyleft.
- GPL-3.0-or-later: strong copyleft.

For pure creative assets, standard Creative Commons licenses may be more appropriate than software licenses.

## 15. Contributions: inbound=outbound, DCO, CLA

### Inbound=outbound

A normal GitHub contribution to a licensed repository is commonly handled as contribution under the repository license. This is sufficient for ordinary GPL contributions if the contribution terms are clear.

### DCO

Recommended now. DCO adds provenance/authority certification without requiring copyright assignment.

### CLA

Not recommended as an automatic requirement at the current stage.

A CLA adds contributor friction and is unnecessary if YoungLion only needs contributions under GPL plus the current Plugin Exception.

A CLA becomes strategically useful if YoungLion later wants unilateral authority to grant materially broader exceptions, dual-license contributed host code on proprietary terms, or centralize relicensing power.

### Important exception issue

Contributors should explicitly grant Plugin Exception 1.0 over their contributions. GPL inbound=outbound alone does not necessarily grant YoungLion permission to add a broader proprietary-plugin exception over someone else’s copyright.

## 16. Trademark separation

GPL gives copyright permissions; it does not automatically authorize deceptive use of project branding or security/trust badges.

The Trademark Policy therefore permits truthful compatibility references while reserving logos and Official/Verified trust marks.

Trademark rights differ by jurisdiction, and YoungLion should not use `®` unless registered.

## 17. Minor/developer contracting capacity

Developer/Publisher Terms include a capacity clause because YoungLion may support teenage developers.

The exact ability of a minor to accept platform, paid seller or commercial terms depends on jurisdiction. Paid marketplace onboarding should therefore require a separate legal-capacity/guardian/business verification design before launch.

## 18. Consumer-law limitations

Warranty disclaimers, liability exclusions, digital entitlement transfer and resale restrictions may be limited by mandatory consumer law.

The drafts use “to the maximum extent permitted by applicable law” and preserve non-waivable rights instead of pretending one clause works identically worldwide.

## 19. Patent considerations

Apache-2.0, GPLv3-family licenses and MPL-2.0 contain express patent provisions; MIT/BSD do not contain comparable express patent-license language.

Dependencies and license combinations can introduce patent termination or compatibility questions. High-value Native/L3 or commercial packages should receive dependency-specific review.

## 20. Qualified legal-review checklist

Before production reliance, get qualified legal review for:

1. GPL plugin boundary under the jurisdictions that matter commercially;
2. dynamic/native linking and any static plugin plan;
3. ability of current copyright holders to grant the Exception over all host code;
4. reverse-engineering restrictions;
5. contracts with minor developers;
6. paid marketplace consumer/seller/tax rules;
7. warranty/liability clauses for commercial Packages;
8. trademark registrations and enforcement strategy; and
9. contributor copyright/relicensing strategy if YoungLion later wants to broaden the Exception.

## 21. Current-source limitation

This suite is aligned to the current YoungLion/LeoMiniGames architecture and v0.7 design contract available for review, including RCC compatibility, L1/L2/L3 trust, backend-authoritative Publisher status and developer publishing architecture.

A final repository patch still requires checking the exact current contents and copyright headers of `LICENSE`, `NOTICE`, `COPYRIGHT`, `src/sdk/`, `mod-sdk/`, `theme-sdk/` and the live manifest schema. Do not automatically replace existing third-party notices or file-level licenses without that review.
