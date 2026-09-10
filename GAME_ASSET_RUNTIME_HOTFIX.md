# LeoMiniGames v0.6.2 Game Asset Runtime Hotfix

This source hotfix accompanies YoungLion Official Games Vol.6 v6.4.

## Changes

- Adds the Qt Svg module to the application CMake dependency/link set so external RCC game packs can reliably render SVG assets on Windows and Android deployments.
- Fixes `ModCard.qml` progress bar radius binding. A child `ThemeSurface` is parented to the internal content layer, so `parent.radius` could be undefined at runtime. It now uses the semantic theme alias directly.
- Avoids constructing `Image` objects with empty source URLs in ModCard, Theme Market cards and game icons. The images are created through `Loader` only when a non-empty URL exists, reducing `QFSFileEngine::open: No file name specified` noise.
- Keeps the fallback package/game icon visible until an external image has actually reached `Image.Ready`.

No plugin/package format changes are introduced.
