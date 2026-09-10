# Plugin Developer Guide

1. Start from `mod-sdk/ExampleHelloMod` for a legacy/minimal RCC package or `ExampleModernMod` for v0.6 APIs.
2. Keep `package_format: rcc-v1` and a safe relative `entry` such as `Main.qml`.
3. Do not import LeoMiniGames' private QML module. Use QtQuick/QtQuick.Controls and context services.
4. Use `GameSettings` only for preferences. Use `GameSave` for state/progress and `GameStats` for counters/high scores.
5. Use `Viewport` rather than assuming valid width/height during `Component.onCompleted`.
6. Treat lifecycle callbacks as optional: old mods still run, modern mods can autosave or release resources on lifecycle events.
7. Put translations under `i18n/<locale>.json` and use `GameI18n.text()`.
8. Ship a license file and manifest license metadata. For closed-source YoungLion licensing, `YoungLion-Mod-License` is reserved but the actual license text is not part of this release.

### Save migration example

```qml
GameSave.registerMigration(1, 2, function(oldSave) {
    oldSave.newField = oldSave.oldField || 0
    delete oldSave.oldField
    return oldSave
})
```

Register migrations before loading an old slot.
