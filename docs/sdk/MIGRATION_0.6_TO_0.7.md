# Migration: v0.6 → v0.7

Existing v0.6 RCC packages do **not** need repacking solely for v0.7. Keep `id`, `entry`, package resources, and existing service calls. The host treats a missing `api_version` as legacy compatibility mode.

For new releases, add:

```json
"api_version": "0.7",
"min_api_version": "0.7",
"required_capabilities": ["theme", "resources"]
```

Do not move old `capabilities` blindly into `required_capabilities`; only true hard requirements belong there.

Prefer `GameSettings` for game preferences, `GameSave` for persistent gameplay, `GameResources` for package URLs, `GameAudio` for modern audio, and action-based `GameInput`. Existing `Settings`, `Audio`, `Lang`, `GameTheme`, and `Lifecycle` calls remain supported.

If the old RCC was built with prefix `/mods/<id>`, it remains loadable when it contains nothing outside that namespace. New builds should use internal prefix `/` and let the host mount at `/mods/<id>`.

Manifest claims do not grant verified/official/native/L3 trust. That state is server/host-authoritative.
