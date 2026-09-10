# Theme API

External games use `GameTheme`. v0.7 deliberately keeps the v0.6 token lookup surface: `value`, `color`, `number`, `stringValue`, `hasValue`, `keys`, `values`, and `surface`. Read-only compatibility properties include `activeThemeId`, `activeThemeName`, `revision`, `themes`, and `themeCount`; `isInstalled`, `installedVersion`, and selection of an already installed theme through `applyTheme` are retained.

External packages do not receive raw theme installation/removal APIs. Theme packages and trust are managed by the host.

Prefer semantic aliases over literal UI colors. v0.7 adds background/surface/text/accent/semantic/button/game aliases plus spacing, radius, touch-target, icon-size, typography, animation, shadow/glow, and reduced-motion tokens. Existing aliases remain valid and are not removed for v0.7.
