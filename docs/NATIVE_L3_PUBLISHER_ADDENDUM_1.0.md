# LeoMiniGames Native/L3 Publisher Addendum

**Version:** 1.0  
**Effective date:** 19 September 2026

This Addendum supplements the current YoungLion Developer / Publisher
Distribution Terms.

## 1. Security capability, not copyright right

Native/L3 is a revocable Platform security capability.

A package license, SDK header, local manifest, signature artifact, previous
approval, or source-code availability does not itself create Native/L3 status.

## 2. Scope of approval

YoungLion may scope approval by:
- Publisher;
- Package ID;
- exact version;
- artifact SHA-256;
- operating system;
- architecture;
- Qt/host API version;
- capability;
- review state;
- expiration/review date.

Anything outside that scope requires new authorization.

## 3. Review material

YoungLion may require proportionate:
- confidential Source Code;
- build scripts/toolchain versions;
- SBOM;
- compiler/linker flags;
- symbols;
- reproducible-build instructions;
- source/binary hashes;
- dependency licenses;
- vulnerability results;
- network endpoints/data-flow documents;
- privacy/security threat model;
- signing provenance;
- update mechanism documentation.

## 4. Artifact integrity

Publisher must not:
- substitute an unreviewed binary after approval;
- materially alter an artifact without new review;
- forge or reuse another Publisher's signatures;
- use packers/obfuscation primarily to defeat review;
- hide downloaded executable/native code;
- use a reviewed wrapper to load unreviewed native modules.

Legitimate obfuscation must be disclosed when it materially affects review.

## 5. Privilege minimization

Native/L3 code must request/use only capabilities reasonably required for
disclosed functionality.

Without separate approval, it must not:
- install kernel drivers or privileged system services;
- establish covert persistence;
- tamper with YoungLion security controls;
- inspect unrelated process memory;
- scrape credentials/tokens;
- bypass platform permission systems;
- access unrelated user files;
- deploy a hidden self-update channel that bypasses review.

## 6. Remote code/configuration

Material remote-code, executable-update, plugin-download, or script-update
mechanisms must be declared.

Remote configuration must not turn reviewed benign behavior into unreviewed
privileged behavior.

## 7. Incidents

Publisher must notify YoungLion without unreasonable delay after confirming a
material security compromise affecting Users or Platform integrity.

For an actively exploited critical issue, Publisher should, where practicable,
notify YoungLion within 24 hours of confirming material impact.

## 8. Remediation

YoungLion may require patching, mitigations, a remediation plan, hash revocation,
or temporary disablement before distribution continues.

## 9. Signing keys

Private signing keys must not be committed to repositories or embedded in
Packages.

Suspected key compromise must be reported promptly.

YoungLion may revoke trust records, signatures, credentials, or approvals
associated with compromise.

## 10. Audit

For a credible supply-chain/security concern, YoungLion may request
proportionate evidence that the distributed artifact corresponds to the
reviewed source/build.

This does not transfer Publisher source ownership.

## 11. Emergency suspension

YoungLion may immediately disable Native/L3 loading or distribution for a
credible risk involving malware, credential theft, key compromise, supply-chain
substitution, severe vulnerability, unlawful surveillance, or material review
evasion.

## 12. ABI/API compatibility

Approval does not guarantee permanent Native/L3 ABI compatibility.

Security or platform changes may require migration.

END OF NATIVE/L3 ADDENDUM 1.0
