# Contributing

1. Keep RCC v1 backwards compatible. Do not make a new manifest field mandatory without a major compatibility decision.
2. Do not store progress, score, inventory, save slots, campaign state or achievements in QSettings.
3. Treat publisher verification, review status, native trust and Level 3 as security metadata; never trust a plugin manifest for these decisions.
4. Keep QML safe for transient zero/invalid geometry. Prefer `Viewport.safeWidth`, `Viewport.safeHeight` and positive font/radius bounds.
5. Run `python tools/validate_v060.py`, `python tools/source_guard.py`, `python tools/sanity_check.py` and `python tools/validate_project.py` before submitting changes.
6. New dependencies must be FLOSS-compatible and documented, with Android/F-Droid implications considered.


## Contribution licensing

Unless a file clearly states otherwise, contributions to the LeoMiniGames host
are accepted under the host's outbound license, **GPL-3.0-or-later**. Submitting a
pull request does not by itself transfer copyright ownership to YoungLion. Do not
submit code you do not have the right to contribute.

Developer account permissions, package publishing permissions, Official/Verified
status and Native/L3 approval are separate from copyright licensing. The adopted `LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt` is a GPLv3 section 7 additional permission for qualifying public-interface interactions. Contributors must have authority to grant the project license and that additional permission for their contribution. The proprietary package license is `LicenseRef-YoungLion-Mod-License-1.0` and is separate from contribution licensing.
