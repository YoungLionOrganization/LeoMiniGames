# v0.7.4 Linux runtime investigation

## Host responsibilities

Games and themes use the host contract across platforms. Package developers should not select Qt image plugins, media backends, distribution packages or a window-system workaround. Native package/runtime differences are handled by the host and its distribution pipeline.

## Reproduced problems

1. Theme numeric sanitization used substring matching. `duration` contains `ratio`, so durations such as 110/180/300 ms became 1 ms. Progress height/radius also became at most 1. The new regression fails against the prior code and passes after semantic segment/suffix classification.
2. The published Ubuntu portable-package log deployed SVG/JPEG/GIF/ICO plugins but no WebP plugin. Online kits requested Multimedia without the image-formats add-on. Desktop bundles now require SVG and WebP plugins; actual encode/decode and QML import probes run through the runtime being tested.
3. Debian package dependencies omitted Shapes, Dialogs and separately packaged image plugins. Tests installed the application in the same container that compiled it, where development dependencies could hide missing runtime packages. Additional runtime jobs now install the built package in a fresh distro image, without a Qt development kit.
4. Earlier Linux package logs explicitly reported no audio device. Launching all six games therefore did not establish audible output. A new private PulseAudio null sink and monitor record actual signed 16-bit PCM from named effects and media-player music; silent, truncated and spike-only recordings fail validation.

The Debian runtime splits are documented by the distribution: [Shapes](https://packages.debian.org/trixie/qml6-module-qtquick-shapes), [Dialogs](https://packages.debian.org/trixie/qml6-module-qtquick-dialogs), [SVG plugins](https://packages.debian.org/trixie/qt6-svg-plugins) and [WebP/image-formats plugins](https://packages.debian.org/trixie/qt6-image-formats-plugins).

## Verification boundaries

Local reproduction uses the existing Qt 6.11.1 kit, with outputs on the maintainer's D drive. Linux package installation, plugin loading and PulseAudio output checks run on GitHub, without installing a Linux SDK on the maintainer's computer. Live catalog themes are downloaded only to private test storage after checking their catalog digests; they are not redistributed as repository fixtures.

These changes do not alter the published v0.7.3 tag or its assets. v0.7.4 remains unreleased. A simulated audio sink proves that PCM reaches an audio server; it does not qualify every physical sound device, Bluetooth route, desktop compositor or graphics driver. New workflow results must be reviewed before release.
