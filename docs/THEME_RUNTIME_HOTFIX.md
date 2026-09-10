# v0.6.2 Theme Runtime Hotfix

This hotfix closes issues observed in the real Qt 6.11.1 desktop runtime log.

## Fixed runtime warnings

- `GameCard.qml`: missing `ratioFull` now maps to the semantic full ratio.
- `ThemeEmptyState.qml`: missing icon color alias now has a defined semantic color.
- `ThemeMarketCard.qml`: optional icon URLs default to an empty string and never resolve `undefined` as a qrc path.
- `ModCard.qml`: operation progress is converted to a finite, clamped ratio before geometry uses it.
- Optional `GameIcon`, inventory, button and dialogue image sources are guarded against empty paths.
- `PressableSurface` no longer redeclares `Item.enabled`.

## Full-theme compatibility

Application QML no longer binds presentation directly to Bronze/Espresso palette properties. It uses semantic colors/surfaces/metrics. The legacy `Theme.qml` names remain compatible but resolve through active semantic values. Official full-contract themes can therefore affect the same visual scope as the built-in Bronze Espresso theme.

Gameplay constants are intentionally not theme-controlled.
