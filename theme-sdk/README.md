# Theme SDK — theme API 1 / app 0.7.3

Themes are data-only RCCs, separate from games. [ExampleTheme](ExampleTheme/README.md) 1.2.0 is a small Graphite Gold overlay: it overrides a palette/radius and part of a card surface, inherits remaining defaults and packages an SVG texture.

```sh
python3 tools/sdk/build_package.py theme-sdk/ExampleTheme --check
python3 tools/sdk/build_package.py theme-sdk/ExampleTheme --rcc /path/to/Qt6/rcc --output build-sdk/example-theme.rcc
```

The build_theme.sh/.bat wrappers accept an optional Qt 6 rcc path and work from any directory. The shared helper/source validator is in the repository tools/sdk directory; use it by path for copied standalone projects. Output includes SHA-256; no trust/publishing action is performed.

The RCC must contain `/theme/theme.json` and optional `/theme/assets/...`. Either qresource prefix `/theme` with aliases theme.json/assets/... or prefix `/` with theme/theme.json aliases produces this layout. Include every asset. QML/JS/native binaries are not themes. Format documentation: [THEMING](../docs/THEMING.md); authoring schema: [theme-manifest](../schemas/theme-manifest.schema.json).

Theme Market already installs catalog packages through the host ThemeRuntime. Local integration can use `ThemeRuntime.installThemeRcc(path, sha256)`, applyTheme(id), removeTheme(id); a dedicated local-import button is not promised by this SDK. An external game receives a scoped GameTheme/ThemeRuntime facade and cannot install/remove themes.

Use unique id/name/version, theme_api_version:1 and tokens:{}; aliases/surfaces are optional. Relative assets resolve to the owning RCC; partial nested surfaces inherit defaults. Do not override protected metric.unit/ratio/data primitives or gameplay rules. Cycles/types/versions are checked before replacement. Fonts are allowed data but automatic font registration remains future work.

Test apply/change/remove, fallback readability, live bindings, asset resolution and long/large text on targets. The actual example is installed/applied in sdk_examples CTest. It is not a complete physical-device acceptance test. [Licensing](../LICENSING.md) remains unchanged: unmarked example data follows repository terms; only explicitly marked SDK files receive the dedicated grant. Choose an accepted license for an independent publisher theme.
