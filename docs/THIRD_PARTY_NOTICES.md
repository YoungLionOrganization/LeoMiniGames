# Third-Party Licensing and Notice Map

LeoMiniGames' custom source-available license covers only project-owned material.

## Qt

The current project links Qt modules including Core, Gui, Qml, Quick,
QuickControls2, Network, Svg, and optionally Multimedia.

An official release using Qt under LGPL must satisfy the exact LGPL obligations
for the shipped Qt version and build configuration.

Do not state that LeoMiniGames' custom license restricts users' LGPL rights in
Qt.

## FFmpeg and Multimedia

Qt Multimedia may ship/use FFmpeg and other third-party components.
The exact release artifact must be inventoried because license configuration and
codec/patent considerations can differ between builds/platforms.

## Release inventory rule

For every Windows/Linux/macOS/Android/iOS artifact:

1. enumerate shipped libraries/frameworks/codecs;
2. record exact versions;
3. record the license basis used;
4. include required license texts and notices;
5. satisfy source/source-offer duties;
6. satisfy relinking/replacement/installation-information duties where needed;
7. preserve modification notices;
8. record build provenance.

## Publisher Packages

Publishers remain responsible for their own third-party compliance even when
YoungLion performs automated scans.
