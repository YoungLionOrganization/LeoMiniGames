# ExampleTheme 1.2.0

A minimal Graphite Gold theme for the 0.7.3 runtime, using theme API 1. Its small token overlay deliberately inherits most built-in values. The partial card gradient and relative SVG texture demonstrate fallback and package asset roots.

From the repository root use `python3 tools/sdk/build_package.py theme-sdk/ExampleTheme --rcc /path/to/Qt6/rcc --output build-sdk/example-theme.rcc`, or run build_theme.sh/.bat from any directory. A bare Qt command also works: `rcc --binary --compress-algo zlib theme.qrc -o ExampleTheme.rcc` when run in this example directory.

Install via Theme Market/local host integration; the package is not a game/native plugin. See the [Theme SDK guide](../README.md), [theme format](../../docs/THEMING.md) and [developer licensing](../../docs/DEVELOPER_LICENSING.md). No new license grant/trust approval is implied by copying this example.
