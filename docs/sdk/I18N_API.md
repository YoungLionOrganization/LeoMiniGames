# I18N API

`GameI18n` follows the host language and exposes deterministic fallback. Locale codes are normalized (`az-AZ` → `az`, `zh-Hant` → `zh_tw` where supported). A game should generally not create a second independent language preference.

v0.7 exposes readiness/version/capabilities, translation lookup, placeholder formatting, plural `one`/`other` forms, and runtime language change propagation. Missing keys fall back safely rather than leaving `undefined` in QML.

A package can list `locales` and `default_locale` in its manifest. Locale resources stay inside the package namespace. The host validator checks JSON validity and can report missing/empty entries; long translations still need responsive-layout testing.

Legacy `Lang.language`, `Lang.availableLanguages`, `Lang.text(...)`, and `Lang.setLanguage(...)` remain available through a compatibility facade.
