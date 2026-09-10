# LeoMiniGames Theme System (0.6.2)

LeoMiniGames uses one runtime design-token contract for the application shell and theme-aware games. A theme can replace presentation values and surface specifications without replacing application/game code.

## Core rule

New QML should use semantic `Constants` properties or semantic `alias.*` lookups instead of literal presentation values. The theme system covers substantially more than colors:

- color palettes and state colors;
- spacing, padding, margins and gaps;
- widths, heights and responsive breakpoints;
- radii, borders and focus rings;
- typography size/weight/line-height/letter-spacing roles;
- opacity, scale and motion durations/easing;
- elevation/shadow values;
- touch targets and control geometry;
- page/card/dialog/market/layout metrics;
- game board, tile, HUD, inventory and virtual-control presentation metrics;
- solid, gradient and texture-backed surface specifications.

The built-in Bronze Espresso theme contains 300+ base color tokens, 1,600+ total tokens and 5,000+ aliases. Aliases are the stable semantic contract; raw primitive tokens are implementation details.

## Runtime architecture

- `ThemeManager` is exposed as `ThemeRuntime` and `GameTheme`.
- `Constants.qml` is the Qt Design Studio-style singleton API used by QML.
- `Theme.qml` remains only as a backwards-compatibility facade.
- `ThemeSurface` resolves solid/gradient/texture surface specifications.
- `ThemePolygonSurface` handles theme-aware shaped surfaces.
- `ThemeCatalogManager` provides the online Theme Market catalog and verified download/install pipeline.
- `ThemesExplore` and `ThemesInstalled` are filtered/sorted Qt models used by the Theme Market UI.

Changing the active theme increments the runtime revision. QML bindings resolve the new theme without an application restart.

## Exactly one active theme

`ThemeManager` stores one `activeThemeId`. `applyTheme(id)` replaces that id; therefore two themes cannot be active at the same time. The built-in theme is always retained as the fallback. The Installed UI prevents removal of the active theme, while the runtime also has a safe fallback if removal is requested programmatically.

Themes are not games and are never inserted into `GameRegistry` or the main game list.

## Theme Market UI

The Theme Market is separate from Mods and has two tabs:

- **Explore** — online catalog with search, category filtering, sorting, install/update/reinstall and apply actions.
- **Installed** — built-in, market-installed and local/sideloaded themes with active state, update/reinstall when a catalog version exists, apply, and remove for non-active external themes.

The market is accessible from the Library header and from Settings.

## Package format

External themes are data-only Qt RCC packages. They are separate from game/mod RCC packages.

```text
/theme/theme.json
/theme/assets/...
```

Allowed resource types are limited to JSON, images and supported font data. QML, JavaScript, native libraries and executable files are rejected. The Theme Market download path additionally requires HTTPS, validates catalog/ticket metadata, enforces package-size limits and verifies SHA-256 before installation.

## Protected primitives and gameplay isolation

External themes cannot override protected primitive families such as `metric.unit.*`, `metric.ratio.*` and `metric.data.*`. Themes should override semantic tokens/aliases such as:

```text
alias.button.height
alias.card.radius
alias.dialog.padding
alias.themeMarket.grid.cellHeight
alias.gameTile.gap
alias.typography.body.lineHeight
```

Gameplay rules do not belong in the theme document. Board sizes, scoring rules, RNG behavior, difficulty definitions, economy values and similar mechanics remain in game state/code or `GameRules`.

This prevents a visual theme from silently changing game balance or application behavior.

## Surface specifications

Standard surfaces include app, card, control and game presentation roles. A surface may define:

- solid fill;
- gradient fill and stops;
- texture asset;
- texture opacity/mode;
- radius;
- border color/width;
- optional shape-related metadata understood by reusable components.

If an external theme omits a surface or token, the built-in theme provides the fallback. Fallback assets are resolved against the built-in asset root rather than the external theme root.

## Game developer API

Prefer reusable components from `docs/UI_COMPONENTS.md`:

```qml
GameTileSurface {
    // geometry, border, surface and state presentation come from aliases
}
```

For direct semantic queries:

```qml
property color warningColor: GameTheme.color("color.danger")
property real hudGap: GameTheme.number("alias.gameHud.gap")
property var tileSurface: GameTheme.surface("surface.gameTile")
```

Legacy RCC games are not required to become theme-aware. Level 2 games should generally use the shared contract so Theme Market packages can restyle them automatically.
