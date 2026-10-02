# Manifest Contract

The package manifest is `manifest.json`. The legacy minimum remains `id`, `name`, `version`, and `entry`; `package_format` defaults to `rcc-v1` when absent in old metadata paths.

```json
{
  "id": "example_game",
  "name": "Example Game",
  "version": "1.0.0",
  "entry": "Main.qml",
  "package_format": "rcc-v1",
  "api_version": "0.7",
  "min_api_version": "0.7",
  "required_capabilities": ["theme", "save.atomic", "resources"]
}
```

`capabilities` is retained for v0.5/v0.6 compatibility and is informational for old packages. v0.7 hard requirements belong in `required_capabilities`. If `api_version` is omitted, the host negotiates legacy v0.5/v0.6 behavior.

Paths such as `entry`, `icon_path`, and `license_file` must be relative, normalized, and must not contain traversal. Package ids match `^[a-z0-9][a-z0-9_.-]{1,63}$`.

Fields such as `official`, `verified`, `reviewed`, `native`, or `plugin_level` in a local manifest are never authoritative publisher permission. Catalog/account policy owns trust.

## Field groups

| Fields | Meaning |
| --- | --- |
| id, name, version, entry | Stable game identity/display/version and relative QML entry. Changing id creates a new save/settings namespace. |
| package_format, min_app_version | rcc-v1; minimum application SemVer (separate from API version). |
| api_version, min_api_version, required_capabilities | Requested/minimum host API and true hard feature requirements. |
| icon_path, license, license_file, source_url, source_available | Resource/publishing metadata; include referenced files in RCC. |
| save_version | Positive integer snapshot schema, default 1; register migrations before load. |
| settings_schema | Object metadata; not automatic preference validation/defaults or an implemented shared editor. |
| locales, default_locale | Flat i18n JSON files and fallback locale; package every declared locale. |
| orientation, theme_support, theme_surfaces, tags | Presentation/discovery declarations, not game rules or trust. |
| plugin_level | Requested content level; does not grant native/verified permissions. |

[JSON schema](../../schemas/plugin-manifest.schema.json) permits additional fields for forward compatibility. The runtime remains authoritative for compatibility/trust; the SDK build check is an authoring aid, not a full host/publisher validator. See [ExampleModernMod manifest](../../mod-sdk/ExampleModernMod/manifest.json) for a tested API 0.7 package.

GameRuntime's exact host capability names are theme, i18n, save.atomic, audio.capability_probe, input.physical_wasd, lifecycle, random, settings, stats, achievements, events, resources, network.https, legacy_rcc_mount, developer_diagnostics. GameAudio.capabilities uses a different service-specific vocabulary; do not put its music/preload strings into required_capabilities.

## Network capability

Installed v0.7 packages that need HTTPS runtime access should declare `network.https` in `capabilities` and, when it is a hard requirement, in `required_capabilities`. Legacy v0.5/v0.6 installed packages keep the historical HTTPS compatibility path. Developer-local RCC network access remains disabled.
