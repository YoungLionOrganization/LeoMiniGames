# Trusted Native / Level 3 Plugins

Level 3 is a runtime trust decision, not a manifest permission. A normal RCC package cannot become native or trusted by setting `plugin_level: 3`, `native: true`, `verified: true`, or similar fields.

The current desktop loader accepts a native Qt plugin only when it is explicitly listed in the application-data `plugins/trusted-native.json` snapshot and all of these checks succeed:

- `reviewed` is `true`;
- `publisher_verified` is `true`;
- `sha256` is a 64-character SHA-256 digest and exactly matches the library bytes;
- `file` is a plain filename inside the trusted plugin directory (no path traversal);
- `api_version` is compatible with the current native plugin API (currently `1`);
- `qt_major` matches the running Qt major version;
- Qt recognizes the file as a loadable library and the normal `IGamePlugin` metadata/API validation succeeds.

Example trust snapshot:

```json
{
  "plugins": [
    {
      "file": "ExampleTrustedPlugin.dll",
      "sha256": "<64 lowercase hex characters>",
      "reviewed": true,
      "publisher_verified": true,
      "api_version": 1,
      "qt_major": 6,
      "signature": null
    }
  ]
}
```

`signature` is reserved for a later signed publishing flow. v0.6.0 does not pretend that a signature has been verified when no signature infrastructure exists. Android does not dynamically load Level 3 libraries in this release.
