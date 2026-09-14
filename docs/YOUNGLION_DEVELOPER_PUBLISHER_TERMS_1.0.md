# YoungLion Developer / Publisher Distribution Terms

**Version:** 1.0  
**Effective date:** 10 September 2026  
**Applies to:** LeoMiniGames developer/publisher submission and distribution services

> These Terms govern the relationship between a Publisher and YoungLion when content is submitted to or distributed through the LeoMiniGames/YoungLion platform. They are **not** an end-user software license and do not replace the license selected for a Package.

## 1. Relationship to other YoungLion terms

These Developer / Publisher Distribution Terms (“Publisher Terms”) supplement the general YoungLion Terms of Service, Privacy Policy, Security/Responsible Disclosure rules, applicable marketplace rules and service-specific policies.

If a Package has its own copyright license, that license governs the rights of recipients in the Package. These Publisher Terms govern the Publisher’s use of the YoungLion publishing and distribution service.

## 2. Eligibility and contracting capacity

A Publisher must have legal capacity to accept these Publisher Terms.

If a Publisher is below the age of legal majority or otherwise lacks full contractual capacity, the Publisher must use the service only through a parent, legal guardian or other legally authorized representative where required by applicable law.

Paid selling, revenue sharing, tax onboarding, payouts or other commercial Publisher functions may require additional age, identity, tax, business-registration or legal-capacity checks before activation.

YoungLion may require re-acceptance when these Terms materially change.

## 3. YoungLion Account and canonical identity

Where the developer platform uses YoungLion Account, the canonical user identity is the YoungLion account identifier issued by the central identity system.

A Publisher must not spoof, forge or misrepresent another Publisher identity.

Developer API keys, OAuth sessions, signing credentials and other authentication material are personal or organization-scoped security credentials and must be protected according to the applicable developer security requirements.

## 4. Publisher ownership

The Publisher retains ownership of intellectual-property rights the Publisher owns in submitted Packages.

Submission does **not** assign copyright ownership to YoungLion.

No provision of these Publisher Terms transfers ownership of the Publisher’s Source Code, artwork, audio, trademarks or other protected material to YoungLion except where the parties separately sign an explicit assignment.

## 5. Package license selection

Each new submission must identify the Package’s applicable copyright/content license using the current manifest and market requirements.

A Package may use:

- an accepted standard open-source/free-software license;
- a standard content license where appropriate for standalone creative assets; or
- `LicenseRef-YoungLion-Mod-License-1.0` for qualifying proprietary Packages.

A Publisher must not use a custom proprietary license label to conceal obligations imposed by incorporated open-source components.

## 6. Limited service-operation license to YoungLion

By submitting a Package for hosting or distribution, the Publisher grants YoungLion a **worldwide, non-exclusive, royalty-free, service-operation-limited license** to the submitted Package and associated submission materials, only to the extent reasonably necessary to operate, secure and improve the publishing/distribution service.

This license permits YoungLion to:

1. receive and store the Package;
2. make technical and operational copies;
3. cache the Package and deliver it through CDN/storage infrastructure;
4. create backups and disaster-recovery copies;
5. unpack or inspect the Package for validation;
6. validate package structure, manifests, declared interfaces and compatibility metadata;
7. conduct security, malware, integrity and policy scans;
8. compute hashes, signatures and other integrity metadata;
9. digitally sign or countersign a Package where the Platform’s security architecture requires it and the Publisher is eligible;
10. display Package metadata, descriptions, icons, screenshots, changelogs, license information and Publisher information submitted for public display;
11. list and index the Package in catalogs, search results and recommendation surfaces;
12. reproduce and distribute the Package to Users who are authorized to acquire it;
13. deliver updates and patches submitted by the Publisher;
14. permit reasonable redownload/recovery by Users who previously acquired the Package;
15. retain limited archival copies and records for security, fraud prevention, incident response, legal compliance and dispute resolution; and
16. perform a narrowly necessary technical platform conversion when distribution to a supported platform requires format normalization, compression, metadata generation, signing, containerization or equivalent non-substantive processing.

YoungLion may not use this Section as a general license to create unrelated products from the Publisher’s proprietary content.

## 7. Open-source Packages

For a Package already distributed under an open-source license, YoungLion will rely on the rights granted by that license wherever reasonably sufficient.

The Section 6 service-operation grant applies only to additional submission materials or platform operations not clearly covered by the selected license, such as Publisher-provided promotional metadata, screenshots, private pre-release upload handling, signing, entitlement-based delivery, redownload records or limited platform conversion.

Nothing in these Publisher Terms reduces rights recipients already have under the Package’s open-source license.

## 8. Publisher representations

By submitting a Package, the Publisher represents that, to the Publisher’s knowledge and to the extent legally required:

1. the Publisher owns the submitted material or has sufficient rights to submit, license and distribute it;
2. all third-party licenses and attribution requirements are satisfied;
3. the selected license metadata is materially accurate;
4. the Package does not knowingly contain material the Publisher is not authorized to distribute;
5. the Publisher has not intentionally included malware, credential theft, destructive payloads, unauthorized surveillance, covert persistence, secret backdoors or other prohibited malicious functionality;
6. material capabilities have not been intentionally concealed to bypass review;
7. Publisher identity and ownership information supplied to YoungLion is materially accurate; and
8. the Package does not falsely claim Official or Verified status.

## 9. Third-party components

The Publisher is responsible for identifying third-party dependencies and complying with their licenses.

Where a third-party license requires Source Code, source offers, relinkability, copyright notices, a NOTICE file, patent conditions or other obligations, the Publisher must satisfy them before publication.

YoungLion may reject a submission whose declared proprietary terms conflict with an incorporated open-source license.

## 10. Security and prohibited functionality

Publishers must not use the Platform to distribute malware, credential stealers, unauthorized spyware, destructive code or packages designed primarily to bypass YoungLion security controls.

Security-sensitive capabilities must be declared according to the current manifest and review requirements.

YoungLion may quarantine or disable a Package immediately where a credible security threat exists.

## 11. Privacy and user data

A Package receives no right to personal data merely by being published.

A Package that collects telemetry, accesses YoungLion Account data, uses OAuth, accesses external accounts, communicates with external services, uses advertising/analytics, or otherwise processes personal data must comply with applicable law and YoungLion privacy/capability requirements.

A Publisher must not request broader permissions than reasonably required for the Package’s disclosed functionality.

## 12. Backend-authoritative trust and capability

Copyright license metadata and trust/capability metadata are separate.

The following values, or equivalent successor values, are **not copyright licenses and are not controlled by a Package manifest**:

- `official`
- `verified`
- `developer`
- `native`
- `native_l3_allowed`
- `plugin_level`
- Official Publisher status
- Verified Publisher status
- native platform allowlists
- review bypass privileges
- signing authority

YoungLion/LeoMiniGames backend records are authoritative for these trust and capability decisions.

A manifest field attempting to self-assert a privileged value does not grant that privilege.

## 13. Official and Verified badges

A Publisher may not self-assign, reproduce deceptively, spoof or imply entitlement to YoungLion trust badges.

Current trust presentation may include:

- Gold Official badge — official YoungLion publication;
- Green Native/L3 Verified badge — verified Publisher with designated native/L3 permission; and
- Blue Verified badge — verified Publisher.

YoungLion may change badge designs, names or qualification criteria without changing the copyright license of previously published Packages.

## 14. Native / L3 publishing

Native/L3 publication requires separate backend authorization and may require enhanced review, code inspection, signing, platform-specific artifacts, security documentation and additional contractual terms.

Possession of a native SDK header, use of the Plugin Exception or selection of the YoungLion proprietary license does not create Native/L3 eligibility.

YoungLion may restrict native publishing by Publisher, Package, version, platform, architecture or review state.

## 15. Validation and review

YoungLion may perform automated and manual validation, including:

- manifest/schema validation;
- package/resource validation;
- license checks;
- compatibility checks;
- malware/security scanning;
- native binary inspection where applicable;
- metadata review; and
- moderation review.

Passing automated validation does not guarantee publication or Official/Verified status.

## 16. Publishing decisions

YoungLion may approve, reject, delay, quarantine, suspend, delist or unpublish a Package for legitimate platform reasons including:

- security risk;
- malware;
- license conflict;
- copyright/trademark claim;
- privacy violation;
- material misrepresentation;
- technical instability;
- prohibited content;
- fraud;
- policy violation;
- legal requirement; or
- abuse of privileged capabilities.

Where circumstances permit, YoungLion should provide a reason and an appeal/review channel.

## 17. Publisher removal and prior acquisitions

A Publisher may request that a Package stop being offered to new Users, subject to outstanding contractual, marketplace or legal obligations.

Removal from new distribution does not necessarily require YoungLion to:

- remotely delete already installed copies;
- revoke rights already granted by an open-source or end-user license;
- immediately destroy security/audit backups;
- erase legally required transaction records; or
- prevent reasonable redownload by Users who previously acquired the Package where the applicable entitlement model promises redownload.

The service-operation license in Section 6 survives termination only to the limited extent reasonably necessary for these purposes.

## 18. Updates

Each submitted version is separately attributable to the Publisher and may have version-specific hashes, signatures, license metadata and review state.

A Publisher may change the license of a future version only where the Publisher has authority to do so. A later license selection does not retroactively revoke rights granted under an earlier version’s license.

## 19. Legacy Packages

The runtime compatibility policy for legacy LeoMiniGames v0.5/v0.6 Packages is separate from new market submission requirements.

Missing modern license metadata in a legacy Package must not, by itself, make an already compatible Package fail to load at runtime.

YoungLion may classify such a Package as `legacy/unknown-license` for display or review purposes.

A new submission or new version may be required to provide current license metadata before publication.

## 20. Paid content and commercial schedules

These Publisher Terms do not by themselves establish revenue sharing, payout timing, marketplace commission, taxes, refunds, subscription rules or seller-of-record arrangements.

Before paid third-party publishing is enabled, YoungLion should provide a separate commercial/marketplace schedule covering those matters and any additional identity, age, tax and business verification requirements.

## 21. API keys and publishing credentials

Developer API keys and other secrets must not be embedded in public Packages or manifests.

A Publisher is responsible for safeguarding credentials issued to the Publisher and must promptly rotate/revoke a credential believed to be compromised.

YoungLion may revoke credentials independently of a Package’s copyright license.

## 22. Feedback and diagnostics

Publisher-provided bug reports, compatibility reports and suggestions may be used by YoungLion to operate and improve the Platform. Submission of feedback does not transfer ownership of unrelated Publisher code.

## 23. Term and termination

These Publisher Terms remain effective while the Publisher uses the publishing/distribution service.

Either side may end the service relationship subject to surviving obligations. YoungLion may suspend access immediately for serious security, fraud or legal risk.

Sections concerning ownership, prior license grants, archival/security retention, existing end-user rights, dispute resolution and provisions that by their nature should survive remain effective to the necessary extent.

## 24. Warranty disclaimer

TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE PUBLISHING SERVICE IS PROVIDED “AS IS” AND “AS AVAILABLE”. YOUNGLION DOES NOT GUARANTEE ACCEPTANCE, CONTINUOUS MARKET AVAILABILITY, A PARTICULAR REVIEW TIME, DOWNLOAD VOLUME OR REVENUE.

MANDATORY RIGHTS ARE NOT EXCLUDED.

## 25. Limitation of liability

To the maximum extent permitted by applicable law, neither party is liable under these Publisher Terms for indirect or consequential loss that applicable law permits the parties to exclude.

This clause does not exclude liability that cannot legally be excluded or limited.

Commercial seller terms may contain additional lawful allocation of financial risk when paid publishing is introduced.

## 26. Intellectual-property complaints

YoungLion may maintain a notice-and-review process for copyright, trademark and other rights complaints. A Publisher must reasonably cooperate with substantiated claims and may provide counter-information where applicable law permits.

## 27. Trademark reservation

These Publisher Terms do not grant general rights in YoungLion, LeoMiniGames, logos or trust badges. Limited truthful compatibility use is governed by the YoungLion Trademark Policy and applicable law.

## 28. Changes

YoungLion may update these Publisher Terms for legal, security, technical or service changes. Material changes should be versioned and communicated through the developer platform or other appropriate channel.

Where legally required or where changes materially alter the Publisher relationship, re-acceptance may be required.

## 29. Governing terms and mandatory law

These Publisher Terms follow the governing-law/dispute framework of the applicable YoungLion Terms of Service unless a separate signed agreement lawfully provides otherwise.

Nothing in these Terms waives non-waivable rights.

## 30. Legal review note

Before enabling material third-party revenue, paid seller onboarding, broad Native/L3 publishing or contracting with minors, YoungLion should obtain jurisdiction-specific legal review for contracting capacity, consumer law, tax, payment, liability and marketplace obligations.
