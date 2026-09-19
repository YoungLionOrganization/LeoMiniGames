# Qt / LGPL Compliance for Source-Available LeoMiniGames

LeoMiniGames may use a restrictive source-available license for its own code
while using Qt components available under LGPL, but the LGPL obligations for Qt
must remain fully intact.

## Current module check

Current CMake configuration uses:
- Qt Core
- Qt Gui
- Qt Qml
- Qt Quick
- Qt Quick Controls 2
- Qt Network
- Qt SVG
- optional Qt Multimedia

At the time of this licensing transition these runtime modules are not listed
among Qt 6.11's GPL-only module list, but the exact Qt version and transitive
components must be verified for every release.

## Required release controls when using open-source Qt

At minimum, verify the applicable LGPL text and exact Qt licensing terms and:

- provide prominent notice that Qt LGPL components are used;
- provide the complete LGPL license text;
- make the corresponding Qt library source, including modifications, available
  through a compliant method under YoungLion's control;
- preserve third-party notices;
- permit the user rights required by LGPL, including replacement/relinking of
  the LGPL library and reverse engineering for debugging such modifications to
  the extent the LGPL requires;
- provide required installation information;
- do not use LeoMiniGames terms to contractually remove Qt LGPL rights.

## Linking

Dynamic linking is normally the simpler path for proprietary/source-available
applications using LGPL Qt.

Static linking requires a separate, careful compliance design, including the
materials/mechanism necessary for user relinking and any other LGPL obligations.

## Mobile/store warning

Android/iOS packaging and application-store terms require separate review.
Qt's own licensing guidance warns that some store/distribution models can
conflict with LGPL obligations.

Do not publish a restrictive LeoMiniGames binary to a store until the release
engineer has documented either:

1. full LGPL compliance for that exact artifact/distribution channel; or
2. a valid Qt commercial license covering that development/distribution path.

## GPL-only Qt modules

Do not add a GPL-only Qt module to a source-available LeoMiniGames build under
the open-source Qt licensing path without a legal/architectural decision.
Such a dependency can make the proposed proprietary/source-available
distribution incompatible.

## CI gate

Release CI should fail if:
- an unexpected Qt module is linked;
- required Qt/LGPL notice files are absent;
- Qt source/source-offer metadata is absent;
- third-party inventory is missing.
