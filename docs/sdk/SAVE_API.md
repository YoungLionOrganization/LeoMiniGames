# GameSave: state, slots and migrations

GameSave stores a per-game CBOR map in atomic, checksummed `.lmgsave` files. Preferences belong in GameSettings. Signatures/signals: [reference](API_REFERENCE.md#gamesave); binary format: [Save System](../SAVE_SYSTEM.md).

| Call | Behavior |
| --- | --- |
| `get(key[,fallback])`, `set(key,value)`, `contains`, `remove` | Read/update the in-memory snapshot; set does not commit by itself. Use plain maps/lists/scalars, not QObject/functions/cyclic objects. |
| `load([slot])` | Default autosave; validates primary/backup and applies registered migrations before replacing memory. Returns bool and emits loaded(slot) on success. |
| `save([slot])` | Default autosave; commits the current map/schema. Returns bool; unchanged data can skip disk rewriting. |
| `autosave()` | Writes dirty data to **autosave**, not currentSlot. Returns true for a clean/no-op snapshot. |
| `forceSave([slot])` | Default autosave; host invokes it after lifecycle save on background/close. It refuses protected failed-load slots. |
| `createSlot(slot)` | Creates an **empty** slot without replacing an existing primary/backup; does not save current payload or select/load that slot. |
| `deleteSlot(slot)` | Explicit removal; check return. Do not delete failed-load saves to suppress an error. |
| `listSlots()` | Primary `.lmgsave` names; backup-only slots are not listed. |
| `hasStoredSlot(slot)` | App 0.7.3 additive QML helper: true when a primary **or backup** exists, regardless of validity. |
| `restoreBackup(slot)` | Copies validated backup bytes to primary preserving schema; call load again to expose/migrate it. |
| `clearMemory()` | Clears the in-memory map and marks it dirty; this is not disk deletion. |

Slot names match `[A-Za-z0-9_.-]{1,64}` except `.` and `..`. Use autosave for the host-owned default path. Manual slots need explicit `save("slot")`/`load("slot")`; a loaded currentSlot does not redirect host autosave. Properties are `gameId`, `currentSlot`, `schemaVersion`, `loadedSchemaVersion`, `lastError`. Observe lastErrorChanged/saved/loaded/backupRestored.

## Lifecycle and safe error handling

```qml
function save() {
    GameSave.set("progress", { level: level, coins: coins })
    if (!GameSave.autosave()) GameLogger.log("error", GameSave.lastError, "Main.qml")
}
```

The host calls the root save hook synchronously, then forceSave. Do not queue an asynchronous save after close; the engine is retired. Failed/corrupt/future-schema loads preserve the original file and block overwrite. Do not treat false as permission to replace it with defaults. Missing first-run data is distinct from a damaged existing slot; Use hasStoredSlot to distinguish missing first-run data from an existing primary or backup; existence does not mean validity. Always check subsequent writes and display lastError. ExampleModernMod requires min_app_version 0.7.3 for this additive helper; older RCC call shapes are unchanged. App-specific payload validation still belongs to your game.

## Schema migration

Declare save_version in the manifest; omitted defaults to 1. Register migrations in Component.onCompleted, which runs before the host load hook:

```qml
Component.onCompleted: {
    GameSave.registerMigration(1, 2, function(oldSave) {
        oldSave.taps = oldSave.score === undefined ? 0 : oldSave.score
        delete oldSave.score
        return oldSave
    })
}
function load() {
    if (GameSave.load()) taps = GameSave.get("taps", 0)
    else status = GameSave.lastError
}
```

Callbacks must return a plain object; registration requires increasing versions. Missing chains, failed callbacks, payloads over 32 MiB/deeper than 64 and newer schemas fail rather than silently resetting progress. Register every supported path, test old fixtures and reopen after commit. Settings-key compatibility migration is a different process; it does not migrate gameplay snapshots.

Portable Save Transfer/cloud synchronization and a general slot editor are not provided in 0.7.3. Six builtins now save actual boards/hands/state through the host; external games still define their own snapshots/migrations.
