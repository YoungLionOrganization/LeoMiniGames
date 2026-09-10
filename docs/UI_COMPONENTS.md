# LeoMiniGames UI Component Library

LeoMiniGames 0.6.2 exposes reusable QML components backed by the runtime theme/token system. Application code and modern games should prefer these components over ad-hoc rectangles, text fields, buttons, overlays, HUD elements, or virtual controls.

## Core themed surfaces

- `ThemeSurface` — solid, gradient or texture-backed surface resolved from a theme surface specification.
- `ThemePolygonSurface` — theme-aware shaped/polygon surface.
- `TokenPanel` — semantic panel wrapper.
- `PressableSurface` — hover/press/focus state layer with themed motion and geometry.
- `StateLayer` — reusable state overlay.
- `OverlayScrim` — modal/background scrim.
- `FocusRing` — focus indicator using theme focus tokens.

## Text and controls

- `TokenText` — typography role based text.
- `AutoFitText` — token-backed text that safely fits constrained areas.
- `NumberTicker` — animated numeric display.
- `ThemeButton` / `ThemeIconButton` — standard actions.
- `ThemeTextField` — standard themed text field.
- `ThemeBadge` — status/tag badge.
- `ThemeDivider` — semantic separator.
- `ThemeProgressBar` — determinate/indeterminate progress.
- `ThemeToast` — transient in-app notification surface.
- `ThemeEmptyState` — standard empty/error-state presentation.
- `ThemeListItem` — reusable list row.
- `DialogueBubble` — themed dialogue/message bubble.
- `KeyHint` — keyboard/controller action hint.
- `LoadingShimmer` — loading placeholder.

## Layout and interaction

- `SafeAreaItem` — safe viewport wrapper.
- `AspectFrame` — aspect-ratio preserving content frame.
- `TouchTarget` — minimum touch target wrapper.
- `AdaptiveGrid` — theme-token driven responsive grid.

## Game UI

- `GameBoardSurface` — standard board/playfield surface.
- `GameTileSurface` — tile/cell component.
- `GameHudBar` — game HUD container.
- `InventorySlot` — inventory/equipment slot.
- `VirtualActionPad` — digital action buttons backed by `GameInput`.
- `VirtualJoystick` — analog movement input backed by `GameInput` actions.

## Market UI

- `ThemeMarketCard` — Explore/Installed theme card with install, update, reinstall, apply, active and remove states.

## Rules

Presentation values should come from `Constants` semantic properties or `Constants.n()/c()/s()` using `alias.*` keys. Gameplay rules must use game state or `GameRules`, not theme metrics. External themes may override semantic presentation tokens and surfaces but cannot override protected primitive units/ratios or execute QML/JavaScript/native code.

## External plugin developers

The application QML module is intentionally private to avoid coupling RCC v1 games to internal UI implementation details. External games use the same design system through the `GameTheme` context API. The modern SDK example contains standalone `SdkPanel`, `SdkText` and `SdkButton` components that can be included in a game's own RCC and extended into a game-specific component kit. They resolve the same semantic aliases as the application, so installed themes can affect both the shell and modern games without a private QML import.
