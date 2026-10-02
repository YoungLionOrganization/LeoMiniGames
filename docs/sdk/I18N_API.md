# GameI18n

Methods: text(key[,fallback]), format(key,arguments[,fallback]), plural(key,count[,fallback]), hasKey(key), capabilities(). Properties: gameId, language, defaultLocale, locales, ready, version. Signals: changed, languageChanged, missingKey(key,locale).

Locale names are normalized to lowercase with underscores (`az-AZ` becomes `az_az`, with base `az` fallback; Chinese variants map to zh_cn/zh_tw). Resolution is requested locale → requested base → default locale → default base → English → explicit fallback/key. List locales/default_locale in the manifest and package every JSON file in the RCC under i18n/. Keys are flat strings; plurals use suffix `.one` / `.other` (not a full CLDR plural-rule engine).

```json
{"welcome":"Hello %{name}","items.one":"%{count} item","items.other":"%{count} items"}
```

```qml
function tr(key, fallback) {
    let language = GameI18n.language // makes the QML binding depend on language
    return GameI18n.text(key, fallback)
}
Text { text: tr("welcome", "Hello") }
// Formatting: GameI18n.format("welcome", {name: "Ada"})
// Plural: GameI18n.plural("items", itemCount)
```

Placeholders support `%{name}`, `{%name}` and positional `%1` etc.; named placeholders avoid map-order ambiguity. Reading language in a text-producing binding is required for live updates; an invokable lookup alone has no NOTIFY dependency. Imperative status strings should be recomputed when language changes if they must change after assignment. Do not create an independent language preference by default; the host follows its selected language.

Legacy Lang.language/availableLanguages/text/setLanguage remain available. Lang.text translates **launcher source strings**, whereas GameI18n.text looks up **package keys**. JSON/coverage checks do not replace layout/RTL testing.
