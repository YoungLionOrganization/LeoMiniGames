# RCC Plugin Format

## Compatibility contract

**The package format is still `rcc-v1`.** v0.7.3 does not introduce a second package format for normal mods.

Legacy minimum fields such as `id`, `name`, `version`, `entry` and optional `icon`/`icon_path` remain valid. New fields are additive and optional. A missing v0.6 field must not invalidate an otherwise valid legacy plugin.

Typical package:

```text
Main.qml
manifest.json
assets/icon.png
LICENSE                 # recommended/required by publishing policy
sfx/...
i18n/en.json            # optional
i18n/tr.json            # optional
```

Build with Qt `rcc -binary` and mount is still `qrc:/mods/<id>/`.

## Additive fields (0.6/0.7)

`license`, `license_file`, `source_url`, `source_available`, `locales`, `default_locale`, `tags`, `capabilities`, `save_version`, `settings_schema`, `plugin_level`, `orientation`.

The canonical machine-readable schema is `schemas/plugin-manifest.schema.json`. It intentionally permits additional fields for forwards compatibility.

Modern API 0.7 also uses api_version, min_api_version and required_capabilities; see [Manifest](sdk/MANIFEST.md). The shared [SDK builder](sdk/QUICKSTART.md) validates canonical resource inclusion and compiles with Qt 6 rcc. Missing API fields retain the legacy path.

## Security fields

A manifest may describe a requested `plugin_level`, but it cannot prove publisher verification, review, native trust or Level 3 authorization. RCC metadata is clamped to Level 1/2. Level 3 is established only by trusted catalog/client metadata and native trust policy.
