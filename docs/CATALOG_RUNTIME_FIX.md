# Catalog runtime compatibility fix

Observed launcher logs contain three separate failure classes:

- model values reaching `double` bindings as `undefined` (`GameCard.qml`, `ModCard.qml`);
- a theme color binding receiving `undefined` (`ThemeEmptyState.qml`);
- `ThemeMarketCard.qml` resolving an image source to `.../undefined`.

`CatalogCompat` normalizes old/new JSON before it is exposed to QML and removes already-expired presigned `icon_url` values. Integrate it at the JSON -> model boundary, not separately in every card.

```cpp
const QJsonDocument doc = QJsonDocument::fromJson(replyBytes);
for (const auto &value : CatalogCompat::extractItems(doc)) {
    if (!value.isObject()) continue;
    const QJsonObject safe = CatalogCompat::normalizeItem(value.toObject(), "mod");
    // append safe to the existing model
}
```

For QML properties that come from the active theme rather than the catalog, use the host's verified alias if it exists; where backwards-compatible themes may omit it, use an existing semantic host fallback. Do not bind `undefined` directly to `real`, `int`, `color`, or `url` properties. `qml/CatalogFallbacks.js` is supplied for transitional guards.

Do not cache presigned R2 `icon_url` as durable metadata. It is a presentation URL, not package identity. Package downloads must continue using the existing download endpoint and SHA-256/size verification flow.
