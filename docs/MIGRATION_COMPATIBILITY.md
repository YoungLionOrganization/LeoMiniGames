# v0.5.2 → v0.6.0 Compatibility Notes

- RCC format remains `rcc-v1`; no repack is required solely for v0.6.0.
- Legacy mods can keep using `Audio.playUrl`, `Audio.preload` and `Settings` exactly as before.
- Installed v0.5.x `installed.json` entries load with defaults for new metadata. They are classified conservatively and are never promoted to verified/Level 3 from local manifest content.
- Main-library behavior changes intentionally: installed external games now appear beside built-ins and open through the common `GameHost`.
- Built-in v0.5.x score/stat keys are migrated from QSettings to CBOR once. App UI/config settings remain in QSettings.
- New API use is opt-in. A plugin without lifecycle/i18n/save/settings-schema fields is still valid.
