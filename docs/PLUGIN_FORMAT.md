# RCC Plugin Format

## Compatibility contract

**The package format is still `rcc-v1`.** v0.6.0 does not introduce a second package format for normal mods.

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

## Optional v0.6 fields

`license`, `license_file`, `source_url`, `source_available`, `locales`, `default_locale`, `tags`, `capabilities`, `save_version`, `settings_schema`, `plugin_level`, `orientation`.

The canonical machine-readable schema is `schemas/plugin-manifest.schema.json`. It intentionally permits additional fields for forwards compatibility.

## Security fields

A manifest may describe a requested `plugin_level`, but it cannot prove publisher verification, review, native trust or Level 3 authorization. RCC metadata is clamped to Level 1/2. Level 3 is established only by trusted catalog/client metadata and native trust policy.
