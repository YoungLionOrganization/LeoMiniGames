# LeoMiniGames Licensing Architecture

## 1. Host

LeoMiniGames host/core is source-available under:

`LicenseRef-LMG-SAPEL-1.0`

Canonical text: `/LICENSE`.

It is not OSI Open Source and not Free Software.

The license permits inspection, private builds/modifications, contribution
forks, normal use of Official Releases, and independent Publisher Packages via
designated public interfaces. It prohibits unauthorized host redistribution,
alternative builds, rebranded distributions, mirrors, and unofficial releases.

## 2. SDK / Templates

Only files explicitly carrying:

`LicenseRef-YoungLion-LMG-SDK-1.0`

receive the developer-oriented SDK grant.

Directory location alone is not enough.

The SDK license permits Publisher developers to incorporate designated SDK
material into their own compatible Packages.

## 3. Publisher Packages

A Publisher owns its own original Package material.

Publishers may choose a recognized open-source license, their own legally valid
license accepted by Platform policy, or:

`LicenseRef-YoungLion-Publisher-Package-1.0`

for eligible proprietary Packages.

## 4. Publisher ↔ YoungLion

Publishing through YoungLion/LeoMiniGames is not governed solely by copyright
license metadata.

The service relationship is governed by:

`docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`

Native/L3 requires:

`docs/NATIVE_L3_PUBLISHER_ADDENDUM_1.0.md`

## 5. Trust is not licensing

Official, Verified, Native/L3, signing, review, and privileged capability state
is backend-authoritative. A manifest cannot self-grant it.

## 6. Third-party software

Qt and other dependencies remain under their original terms.

LeoMiniGames' source-available license must never be represented as restricting
LGPL rights in Qt libraries.

## 7. Historical GPL state

Earlier public repository revisions contained GPL notices. The transition to
the source-available license does not retroactively revoke rights already
validly granted to recipients of earlier copies.

See `LICENSE_HISTORY.md`.

## 8. F-Droid

The new host license is not FLOSS. Therefore current/future source-available
LeoMiniGames versions are not eligible for the official F-Droid main repository.

A YoungLion-controlled Android repository may be operated separately if desired,
subject to all third-party licensing and package-signing requirements.
