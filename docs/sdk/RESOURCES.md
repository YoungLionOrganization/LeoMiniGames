# RCC Resources

Preferred package layout:

```text
RCC internal prefix: /
manifest.json
Main.qml
assets/...
i18n/...
sfx/...
```

The host mounts the RCC at `/mods/<game_id>`, so `Main.qml` becomes `qrc:/mods/<game_id>/Main.qml`.

For backward compatibility, a legacy RCC already built with internal `/mods/<game_id>` is accepted and mounted at `/` only when **every resource** is under that exact namespace. A legacy RCC that also exports `/themes`, `/i18n`, another mod id, or any host namespace is rejected. Ambiguous legacy packages containing manifests for multiple ids are also rejected.

Use `GameResources.url("assets/icon.svg")` and `GameResources.exists(...)` in v0.7 code. Old direct `qrc:/mods/<id>/...` references continue to work.
