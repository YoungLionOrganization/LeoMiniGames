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

## Network capability

Installed v0.7 packages that need HTTPS runtime access should declare `network.https` in `capabilities` and, when it is a hard requirement, in `required_capabilities`. Legacy v0.5/v0.6 installed packages keep the historical HTTPS compatibility path. Developer-local RCC network access remains disabled.
