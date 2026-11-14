# v0.7.4 Linux runtime investigation

## Host responsibilities

Games and themes use the host contract across platforms. Package developers should not select Qt image plugins, media backends, distribution packages or a window-system workaround. Native package/runtime differences are handled by the host and its distribution pipeline.

## Reproduced problems

1. Theme numeric sanitization used substring matching. `duration` contains `ratio`, so durations such as 110/180/300 ms became 1 ms. Progress height/radius also became at most 1. The new regression fails against the prior code and passes after semantic segment/suffix classification.
2. The published Ubuntu portable-package log deployed SVG/JPEG/GIF/ICO plugins but no WebP plugin. Online kits requested Multimedia without the image-formats add-on. Desktop bundles now require SVG and WebP plugins; actual encode/decode and QML import probes run through the runtime being tested.
3. Debian package dependencies omitted Shapes, Dialogs and separately packaged image plugins. Tests installed the application in the same container that compiled it, where development dependencies could hide missing runtime packages. Additional runtime jobs now install the built package in a fresh distro image, without a Qt development kit.
4. Earlier Linux package logs explicitly reported no audio device. Launching all six games therefore did not establish audible output. A new private PulseAudio null sink and monitor record actual signed 16-bit PCM from named effects and media-player music; silent, truncated and spike-only recordings fail validation.

The Debian runtime splits are documented by the distribution: [Shapes](https://packages.debian.org/trixie/qml6-module-qtquick-shapes), [Dialogs](https://packages.debian.org/trixie/qml6-module-qtquick-dialogs), [SVG plugins](https://packages.debian.org/trixie/qt6-svg-plugins) and [WebP/image-formats plugins](https://packages.debian.org/trixie/qt6-image-formats-plugins).

5. The live Liquid Glass 1.0.1 package uses the legacy `/themes/<id>` layout and contains QML companions. A strict JSON compatibility reader adapts its literal palette/radii to current tokens without evaluating those files. Canonical packages retain the no-QML rule. Fixtures reject namespace escapes, mismatched IDs, executable-looking color strings and canonical QML payloads.

6. Actual Chess Arena loading reproduced `ThemeRuntime is not defined` in imported host singletons. The restricted facades were installed in a child context, while QML singletons use the per-engine root context. Facades now live in that isolated engine's root; a regression fails before the fix and also checks that raw Developer/Paths/Mods objects stay absent.

## Actual catalog investigation

The 2 November catalog contained 15 themes (542,406 bytes total) and 18 games (4,915,734 bytes total). Downloads were checked against catalog and ticket hashes. Before the legacy bridge, 14 themes and the default opened all six builtins; Liquid Glass failed at validation. Initial Windows offscreen runs opened 14 of 18 games through the installed-package runtime. Targeted single-game retests passed for Orbital Commander, Sudoku Master and Brick Forge. Chess Arena reproduced the singleton-context defect; after the root-context fix its actual catalog package also opened, rendered and closed without ThemeRuntime warnings. The corresponding external-session regression passes 49 assertions. Liquid Glass subsequently passed all six builtin sessions through the legacy JSON reader; the theme suite passes 39 assertions. A dedicated Linux Xvfb workflow tests every current theme/game at desktop and compact sizes and preserves frame/log evidence.

`LMG_SMOKE_MOD_RCC` stages a validated RCC only inside `--smoke-test`'s temporary storage. The normal installed-package loader applies compatibility, namespace and restricted-service checks. The diagnostic does not authenticate a developer, trust native code or alter a user's installation. `LMG_SMOKE_ONLY_MOD=1` selects only that test session.

## Verification boundaries

Local reproduction uses the existing Qt 6.11.1 kit, with outputs on the maintainer's D drive. Linux package installation, plugin loading and PulseAudio output checks run on GitHub, without installing a Linux SDK on the maintainer's computer. Live catalog themes are downloaded only to private test storage after checking their catalog digests; they are not redistributed as repository fixtures.

These changes do not alter the published v0.7.3 tag or its assets. v0.7.4 remains unreleased. A simulated audio sink proves that PCM reaches an audio server; it does not qualify every physical sound device, Bluetooth route, desktop compositor or graphics driver. New workflow results must be reviewed before release.
