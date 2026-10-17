# Save System

Save state is separate from QSettings. Each game is sandboxed under `saves/<gameId>/` and slot names are restricted tokens.

`.lmgsave` layout:

1. 8-byte `LMGSAVE\0` magic
2. 32-bit big-endian CBOR-header length
3. CBOR header: format version, game ID, game version, save schema version, UTC timestamp, SHA-256 checksum
4. CBOR map payload

`QSaveFile` provides atomic primary writes. Before replacing a valid primary file, an atomic `.bak` copy is created. `restoreBackup()` validates the backup and required migrations before copying its bytes back, then loads the prepared state into memory. A rejected migration leaves both files and session memory unchanged. The restored file retains its schema metadata until a successful save commits the migrated state, preserving the original backup. Loading verifies magic, game namespace, format, CBOR structure and SHA-256 before exposing data.

Schema migrations are an ordered chain registered by the game. Missing migrations fail closed; an old save is not silently relabeled as a newer schema.

Migration/snapshot callbacks and completion notifications are scoped to the current game and JavaScript engine. Retiring either cancels the old request, preserving replacement memory and suppressing stale completion events. A disk commit completed before a notification remains committed to its original game.
