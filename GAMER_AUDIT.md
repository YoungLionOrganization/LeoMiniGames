# LeoMiniGames v0.7.0 Gamer / Multi-Platform QA Audit

Date: 2026-09-10

## Test honesty

No physical device or Qt runtime was available in the delivery environment. Therefore Windows/Linux/Android/iOS/macOS gameplay is **not marked physically tested**. This report distinguishes source/UX review from runtime device validation.

## Source-level player experience review

| Area | Source-level status | Device test |
| --- | --- | --- |
| mouse/tap interaction | host controls and touch-target components present | NOT EXECUTED |
| keyboard | arrows + centralized physical/native WASD path | NOT EXECUTED across layouts |
| responsive UI | adaptive grid/safe-area/device-profile components present | NOT EXECUTED |
| mobile safe area | viewport/safe-area service + Developer Lab profiles present | NOT EXECUTED |
| audio fallback | optional backend, lazy effects, failure-safe paths | NOT EXECUTED |
| haptics | host service present and failure is non-fatal | NOT EXECUTED |
| save on close/background | force-save safety paths present | NOT EXECUTED with process kill |
| theme switching | host-owned runtime and compatibility facade present | NOT EXECUTED live |
| language switching | runtime signal/fallback logic present | NOT EXECUTED across all screens |
| old RCC games | compatibility adapters + fixtures present | RCC CTest NOT EXECUTED locally |

## UX improvements relevant to players

- Old games are not forced through a migration UI just because v0.7 exists.
- Load failures can distinguish invalid manifest/resource/API/capability paths instead of collapsing everything into an unexplained generic load error.
- Alternate desktop keyboard layouts no longer rely exclusively on logical `W/A/S/D` characters.
- Built-in card suits are vector-rendered instead of assuming an emoji/symbol font exists.
- Developer Mode is separated from normal player flows; local RCC content is visibly local/unverified and does not silently gain native permission.
- Audio initialization is deferred so startup is not intentionally burdened by a large eager SFX allocation.

## Required release-candidate physical scenarios

Before calling a binary “multi-platform verified”, execute at minimum:

1. Windows: fresh portable ZIP launch, fresh QtIFW install, update install, uninstall; mouse; arrows; physical WASD under Azerbaijani/Turkish and at least QWERTZ/AZERTY where available; audio/save/DPI/resize.
2. Linux: clean machine/AppImage/archive, X11 and Wayland where available, case-sensitive paths, PulseAudio/PipeWire, keyboard/audio/save.
3. Android: touch, system back, background/foreground, force-stop after important save, audio, haptics, safe areas, orientation, low-memory behavior on a lower-end device.
4. iOS: safe area, lifecycle/background, touch, audio session, storage paths and unsigned/signed packaging distinction.
5. macOS: Retina scaling, keyboard, resize, audio, sandbox/path behavior and unsigned-vs-notarized messaging.
6. Legacy corpus: install/load at least one real v0.5 and v0.6 market RCC in addition to synthetic regression fixtures.

## Gamer conclusion

The source is materially better prepared for desktop/mobile play and legacy content than the pre-v0.7 paths, but the absence of a Qt runtime/physical devices prevents a legitimate “Gamer PASS” for production binaries. Source-level UX review has no remaining concrete code issue; physical QA remains a release gate.
