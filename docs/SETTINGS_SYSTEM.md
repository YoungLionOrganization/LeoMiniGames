# Settings System

Global application preferences continue to use `SettingsManager`/QSettings. Plugin preferences use `GameSettings`, which automatically prefixes keys with `games/<gameId>/settings/`.

Good settings: difficulty, music/SFX preference, sensitivity, hints, control layout, particles, camera shake.

Not settings: level, coins, inventory, campaign/world/quest state, save slots, character/factory progress, high scores or achievements. These belong to `GameSave`, `GameStats` or `Achievements`.

An optional manifest `settings_schema` is cached with installed metadata and exposed as `GameSettings.schema` for future shared UI, Editor, CLI and VS Code tooling.
