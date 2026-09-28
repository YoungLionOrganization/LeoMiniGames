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

## QtKeychain

QtKeychain 0.15.0 is included as source under `third_party/qtkeychain` and
statically linked into the application for optional Developer Lab credential
storage in the operating-system keychain. Its 3-clause BSD license is in
`third_party/qtkeychain/COPYING`; binary distributions reproduce the
following copyright notice and disclaimer: Project-local CMake changes are described in
`third_party/qtkeychain/LMG_VENDOR.md`.

Copyright (C) 2011-2015 Frank Osterfeld. Additional contributors are
identified in the bundled source headers.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to
   endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## OpenSSL on Android

The Android build obtains OpenSSL 3.5.8 from the pinned, checksum-verified
source archive and bundles ABI-specific shared libraries. Include its
`LICENSE-OpenSSL.txt` from the build output in binary distribution materials,
alongside an inventory of the exact binaries and their origins.

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
