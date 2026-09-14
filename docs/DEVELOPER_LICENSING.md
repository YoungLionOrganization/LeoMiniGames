# Developer and package licensing

## Host

LeoMiniGames host code is **GPL-3.0-or-later**. `/LICENSE` is the canonical GNU GPLv3 text and remains unchanged. Project ownership/application information is in `/COPYRIGHT`, `/NOTICE` and `/LICENSING.md`.

## Plugin / Mod Exception

`/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` is the adopted GPLv3 section 7 additional permission for qualifying independently developed packages that interact only through designated public LeoMiniGames interfaces. It covers the documented RCC/QML/data boundary and, subject to its exact conditions, a designated public Native/L3 ABI. Private/internal headers, copied host implementation and proprietary core forks are outside the automatic permission.

## Public SDK headers

Narrow public interface-only SDK headers may use `MIT OR GPL-3.0-or-later`. Host implementation remains GPL-3.0-or-later unless a file explicitly says otherwise.

## Proprietary packages

Eligible proprietary packages may select `LicenseRef-YoungLion-Mod-License-1.0` and ship `/licenses/YOUNGLION_MOD_LICENSE_1.0.txt` (or an exact copy in the package). This package license is distinct from the host Plugin/Mod Exception.

## Open-source packages

Prefer standard licenses and SPDX identifiers. See `/docs/OPEN_SOURCE_LICENSE_POLICY.md`.

## Publisher/service terms

Marketplace submission/distribution rights are governed separately by `/docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`. A publisher keeps ownership subject to those terms and the selected package license.

## Trust is not licensing

Official Publisher, Verified Publisher, Native/L3 capability, developer authentication and signing authority are backend-authoritative platform state. No manifest license field grants those statuses.

## Legacy packages

Runtime compatibility is intentionally separate from submission policy. v0.5/v0.6 packages that omit modern license metadata remain loadable. New submissions can require modern license/source metadata without making old installed content unloadable.

## Legal review

The repository includes implementation-ready policy drafts, but jurisdiction-sensitive matters such as Native/L3 linking, consumer limitations, minors, paid marketplace operation and trademark enforcement should be reviewed by qualified counsel before commercial reliance.
