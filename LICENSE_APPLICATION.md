# LeoMiniGames — License Application Notice

Program: **LeoMiniGames**  
Copyright: **Copyright (c) 2026 Cavanşir Qurbanzadə and respective contributors**  
Host license SPDX expression: **GPL-3.0-or-later**

This file states how the repository's licenses apply to LeoMiniGames. It does **not**
replace, edit, shorten, or append terms to the GNU General Public License text in
`/LICENSE`.

## 1. LeoMiniGames host

Unless an individual file carries a different explicit license notice, the
LeoMiniGames host application and host implementation are licensed under
**GNU GPL version 3 or, at your option, any later version
(`GPL-3.0-or-later`)**.

The complete canonical GNU GPL version 3 text is `/LICENSE`.

The `<year>` and `<name of author>` placeholders near the end of `/LICENSE`
belong to GNU's own “How to Apply These Terms to Your New Programs” example.
They are intentionally left exactly as published by GNU and are **not**
unfinished LeoMiniGames metadata. The real LeoMiniGames copyright statement is
above and in `/COPYRIGHT`.

## 2. Plugin / Mod additional permission

`/licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` is a separate GPLv3
section 7 additional permission. It applies only when its own definitions and
conditions are satisfied.

The exception is not inserted into `/LICENSE` because `/LICENSE` is kept as the
canonical GPLv3 license text. Distributors should ship both files when the
exception is relevant.

## 3. Public SDK/interface files

A narrow public SDK/interface file may carry an explicit license such as
`MIT OR GPL-3.0-or-later`. That file-specific SPDX notice controls that file.
It does not relicense the rest of the host.

## 4. Independent open-source packages

Games, mods, themes and other independent packages may use their own compatible
open-source licenses. The repository policy is documented in
`/docs/OPEN_SOURCE_LICENSE_POLICY.md`.

## 5. Eligible proprietary packages

An eligible independent proprietary package may explicitly select
`LicenseRef-YoungLion-Mod-License-1.0` and must then ship the applicable
`/licenses/YOUNGLION_MOD_LICENSE_1.0.txt` terms as required by that license.

This proprietary package license is separate from the GPL license of the host
and separate from the Plugin/Mod Exception.

## 6. Publisher/service terms and trust state

Marketplace submission/distribution terms are in
`/docs/YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md`.

Official/Verified publisher status, Native/L3 authorization, signing authority,
developer authentication and similar trust/capability state are platform policy
or backend-authoritative state. A package cannot obtain those permissions merely
by choosing a license string.

## 7. Trademarks

Copyright permission does not automatically grant trademark, endorsement, badge,
or branding rights. See `/docs/TRADEMARK_POLICY.md`.

## 8. Distribution checklist

Official source and binary distributions should include, at minimum:

- `LICENSE`
- `LICENSE_APPLICATION.md`
- `COPYRIGHT`
- `NOTICE`
- `LICENSING.md`
- `licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt`
- `licenses/YOUNGLION_MOD_LICENSE_1.0.txt`

Third-party notices and source-offer obligations must also be included whenever
applicable.

This notice is repository licensing metadata, not a substitute for
jurisdiction-specific legal review.
