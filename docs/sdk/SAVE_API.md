# Save API

Gameplay persistence uses `GameSave`; application preferences use `Settings`/`GameSettings`. Do not put world/progression/inventory snapshots in QSettings.

The v0.7 save contract is:

```text
state → serialize → atomic write → disk commit → restart → restore
```

The host calls `forceSave()` for background/close/quit paths. Routine autosave may still coalesce writes. `save_version` remains optional and defaults to 1 for old packages; schema evolution should migrate older slots instead of discarding them.

Legacy settings stored as safe unnamespaced QSettings keys can be read once by the v0.7 compatibility facade and copied into the per-game `compat/mods/<id>/...` namespace. Host-reserved settings prefixes are not exposed through this migration path.
