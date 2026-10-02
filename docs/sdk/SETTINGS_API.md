# GameSettings and legacy Settings

GameSettings holds preferences under `games/<id>/settings/...`. Progress belongs in GameSave, scores in GameStats. Methods: `value(key[,fallback])`, `setValue(key,value)`, `contains`, `remove`, `clear`; properties: gameId/schema; signals: gameChanged/valueChanged(key,value).

Keys use letters/digits/underscore/dot/hyphen with optional slash-separated segments. Invalid keys do not escape the namespace. `schema` is manifest metadata for tools/UI; it does **not** automatically apply default values, constrain values or create an editor. Supply explicit defaults and validate preferences in your game:

```qml
property int settingsRevision: 0
function difficulty() {
    let revision = settingsRevision
    return GameSettings.value("difficulty", "normal")
}
Connections { target: GameSettings; function onValueChanged(key, value) { settingsRevision += 1 } }
```

Method calls alone are not change-notifying QML properties. Connect valueChanged when the UI must update; `clear()` currently removes the namespace without per-key valueChanged, so refresh/reset your own view after a clear.

Legacy Settings.value/setValue/contains/remove remain scoped through the compatibility facade, including one-time migration of safe legacy keys. Its soundEnabled, soundVolume and animationsEnabled properties remain launcher preferences. They do not expose arbitrary host paths/credentials. Prefer GameAudio.setVolume for group audio settings.
