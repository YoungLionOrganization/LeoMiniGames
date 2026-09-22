# LeoMiniGames v0.7.0 Bug Hunter / Security Audit

Date: 2026-09-10
Scope: LeoMiniGames application/client source. Supplied Backend/Account/Admin packages were reference-only and were not modified or bundled.

## Method

The audit targeted prior LeoMiniGames regression classes: QML runtime/binding errors, RCC mount ambiguity, publisher spoofing, audio backend misuse, lifecycle/save loss, keyboard-layout input, network races, package/install consistency, developer credential leakage, resource exhaustion and legacy API regressions.

## Resolved findings

| Finding | Risk | Resolution |
| --- | --- | --- |
| canonical/legacy RCC double-prefix ambiguity | load failure / wrong path | centralized RCC inspection and explicit mount root |
| legacy RCC exposing `/themes` or other host namespace | resource injection | legacy layout accepted only when every resource is under its own `/mods/<id>` namespace |
| manifest self-declared Official/Verified/Native-L3 | privilege/trust spoofing | trust accepted only through canonical catalog boundary; local manifest trust ignored |
| custom catalog returns `official` metadata | badge/trust spoofing | YoungLion verification disabled for non-canonical catalog origin |
| external QML received broad host objects | privilege exposure | separate QQmlEngine and game-scoped compatibility facades |
| external logger could export arbitrary files | filesystem write surface | `GameLoggerFacade` exposes logging only, not export/clear |
| external audio could reach arbitrary sources/global cache controls | filesystem/global mutation | game-scoped audio facade and own-qrc restriction |
| eager startup SFX allocation | latency/memory/backend spam | lazy/cache-bounded audio |
| named effect key treated as file URL | decoder errors | named-effect/file-URL paths separated |
| lifecycle save could be skipped by dirty optimization | progress loss | background/close/quit/gameClosed safety paths use `forceSave()` |
| direct logical WASD | alternate keyboard layouts fail | centralized native/physical `GameInput` mapping; arrows retained |
| mod catalog refresh during active row-index callback | wrong-row mutation/race | refresh blocked during resolving/downloading/installing; uninstall/install re-entry guarded |
| download ticket could cross configured catalog origin | auth/trust boundary drift | ticket endpoint constrained to configured HTTPS origin; redirects constrained |
| developer key redirect/oversized response | credential leakage / DoS | manual redirect refusal, timeout and 512 KiB response bound |
| arbitrary 2xx JSON accepted as developer auth (historical v0.7.0 path) | false authenticated session | originally required the content-list `data.items` shape; superseded in v0.7.1 by the dedicated `/api/v1/developer/auth/verify` contract |
| local Developer RCC source replaced after validation | TOCTOU | hash, session copy and re-hash before mount |
| local RCC collides with installed/built-in ID | namespace shadowing | collision rejected |
| empty stats/achievement game ID opened empty file name | runtime warning | empty path is no-op |
| stats/achievement unbounded CBOR/key growth | memory/disk DoS | 2 MiB file cap, bounded identifiers and entry count |
| native plugin trust snapshot/library unbounded | memory/hash DoS | trust snapshot 2 MiB cap; library 256 MiB cap |
| native dynamic loading on iOS | platform/runtime risk | dynamic native plugins disabled on Android and iOS |
| playing-card Unicode suit font dependency | missing glyph on devices | vector Canvas suit rendering; legacy card value preserved |
| Linux “portable” tar contained executable only | unusable clean-system artifact | archive now created from linuxdeploy AppDir runtime |
| installed index processed through duplicated nested loop | duplicate rows / N×N work | duplicate loop removed; installed index processed once |
| modern `required_capabilities` network permission not propagated to runtime | valid v0.7 network game loaded with network disabled | negotiated required capabilities now feed runtime network policy |
| empty `GameSave` game id could form root/current-directory paths | unsafe path / accidental scan | empty IDs now fail closed across slot/list/save paths |
| `TouchTarget` redeclared inherited `enabled` property | QML override warning/regression | redundant property removed; inherited `Item.enabled` preserved |
| QtIFW could regress to maintenance-only installation | broken installer UX | application component marked `ForcedInstallation` + `Essential` and validated |

## Static scans executed

- `tools/source_guard.py` — PASS
- `tools/validate_project.py` — PASS
- `tools/sanity_check.py` — PASS
- `tools/security_audit.py` — 16 PASS / 0 WARNING / 0 ERROR
- direct scan for `Qt.Key_W/A/S/D` — no gameplay occurrences found
- direct scan for obvious manifest-authoritative trust — no unsafe trust path found; security comments/local-unverified reporting remain
- `tools/validate_distribution.py` — PASS with one intentional screenshot warning
- shell scripts parsed with `bash -n`

## Residual risk / not claimed

- No Qt 6 SDK is installed in this environment, so C++ compilation, qmllint and real RCC CTest execution are not claimed.
- No physical Windows/Linux/Android/iOS/macOS gameplay test was possible here.
- Separate `QQmlEngine` significantly narrows host exposure but QML/JS is not a process-level sandbox. A malicious package can still consume CPU/memory inside the application process. Strong hostile-code isolation would require a process boundary/platform sandbox and is beyond a compatibility-preserving v0.7 patch.
- Native/L3 local trust is an administrator/local-device trust mechanism; a local user able to replace both trusted metadata and executables already has local code-execution capability. Hash/review boundaries prevent a downloaded RCC from escalating into this path.
- Backend native browser-to-app OAuth callback behavior was not modified. The client therefore retains the supplied scoped developer credential session fallback.

## Conclusion

No new concrete static/source defect remained after the final audit pass. Production runtime acceptance still requires Qt-enabled CI/CTest plus physical-device Gamer QA; those are explicitly separate from this source audit.

## v0.7.1 follow-up audit — 2026-09-22

| Finding | Evidence / risk | Resolution |
| --- | --- | --- |
| Developer Lab used `GET /api/v1/developer/contents` as an API-key validity probe | A valid key could be rejected because that route also requires `content:read`, completed developer onboarding and publisher authorization. Authentication and publishing authorization were incorrectly coupled. | Client now calls dedicated `GET /api/v1/developer/auth/verify`; backend verifies active key + enabled developer + active membership without treating content-list permissions as authentication. |
| Authorization header handling assumed only `HTTP_AUTHORIZATION` | Apache/cPanel/FastCGI deployments may expose the forwarded bearer value through `REDIRECT_HTTP_AUTHORIZATION` or request headers, producing false `Developer API key required` failures. | Backend bearer parsing accepts the safe server/header variants while preserving strict single-token `Bearer` syntax. |
| Android CI packaging had no retry for Gradle distribution transport failures | CI #24 Android x86_64 failed only because the Gradle distribution request returned HTTP 500; all source validation and almost every other platform lane succeeded. | Added bounded retry helper that recognizes transport/Gradle-distribution failures only, cleans partial wrapper files and retries. Genuine build failures remain fatal immediately. |
| Publish workflow was reported as failed after CI #24 | Publish #4 preflight could not find a successful exact-SHA `ci.yml` run for `d22534e...`. | No gate bypass was added. Publish must remain blocked until the exact commit has a green CI; the underlying Android transient-failure handling was fixed instead. |

The Developer Lab fix does not persist raw API keys and does not grant publishing/native/L3 authority. Local RCC security boundaries are unchanged.
