# LeoMiniGames Theme SDK

Theme packages are data-only Qt RCC bundles, separate from game/mod packages. Build an RCC whose resource root contains `/theme/theme.json` and optional `/theme/assets/*`, then install it through `ThemeRuntime.installThemeRcc(path, sha256)` (future Theme Market will call the same API).

See `docs/THEMING.md`.
