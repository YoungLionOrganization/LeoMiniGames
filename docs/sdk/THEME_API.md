# GameTheme

External games read tokens through value, color, number, stringValue, hasValue, keys(prefix), values(prefix), surface(key). Properties are activeThemeId/name, revision, themes/themeCount, ready/version. isInstalled, installedVersion, applyTheme(id), lastError and capabilities remain compatible. applyTheme selects an already installed theme; it does not install a package and changes launcher-wide selection.

Do not import the private application Constants/ThemeSurface modules. The SDK's SdkPanel/SdkText/SdkButton show package-local Qt Quick components using semantic keys. Missing tokens/surface fields fall back to the built-in theme; nested partial surfaces inherit defaults. Relative asset tokens/aliases resolve to their owning RCC URL. Alias cycles/invalid manifests fail installation rather than blanking the UI.

```qml
function themeColor(key) {
    let revision = GameTheme.revision
    return GameTheme.color(key)
}
Rectangle { color: themeColor("alias.page.background.normal") }
```

Read revision in **every theme-producing binding** that must update; invokable lookups are not independently change-notifying. The same rule applies to number/surface/text/image asset queries. Surface returns a data map, not a rendered component; external games implement their own shape/gradient/image drawing.

External ThemeRuntime is the same scoped facade alias as GameTheme. The host ThemeRuntime has install/remove APIs, but the external facade does not. [Theme SDK](../../theme-sdk/README.md) describes data-only packages/theme API 1. Protected metric.unit/ratio/data primitives keep gameplay constants separate. Automatic packaged-font registration is still future work; allowed font data alone does not change font families.
