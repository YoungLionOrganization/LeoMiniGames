# Pixel asset format groundwork

Reviewed 31 October 2026 against the maintainer's later asset-format discussions. This is an integration plan. LeoMiniGames does not currently decode `.ylpix` or `.ylpxm`, and these formats are outside the v0.7.3 release contract.

## Maintainer direction

| Format | Responsibility |
| --- | --- |
| `.ylpix` | One palette-based RGBA image with packed pixel indices; a small representation of stored color values |
| `.ylpxm` | YoungLion Pixel Model: a general, extensible visual model using reusable image resources, palettes and other representations |

The `.ylpix` direction starts with color depth instead of a magic signature. The palette-count field should scale with color depth. A decoder still needs an explicit format selection and strict validation; the absence of a magic header does not establish file identity or trust.

For `.ylpxm`, common image metadata belongs at the container level instead of being repeated before every embedded image. Resources can then carry the corresponding packed data and references. The architecture must support multiple representation techniques and shared definitions. It must remain useful for scenery, growing trees, varied ore textures and other objects as well as customizable characters.

Compactness should come from shared data, indexed representations and well-defined mathematical construction. A generic compression wrapper alone is not the intended architecture. Optional animation, layers and multiple views belong to the model/container design rather than the minimal still-image format.

## Decisions still required

- Final field widths, byte order, supported color depths and palette-count encoding. Storing `N - 1` and variable-length counts were suggestions, not an approved wire contract.
- Exact index bit order, row layout, one-color handling and trailing-bit requirements. Four colors need two bits per pixel; a 7 by 7 image therefore uses 98 meaningful bits in 13 bytes, leaving six unused bits.
- Whether common container metadata applies globally or through explicitly referenced resource profiles; how unknown optional representations are skipped and unknown required ones are rejected.
- Exact lossless rendering semantics for each representation. Reducing channel precision can change colors and must not be presented as lossless conversion of the original image.
- Portable numerical rules and bounded execution for any mathematical representation. A graphics resource must not become an unrestricted script or native-code entrypoint.

## LeoMiniGames integration boundary

1. Finalize the asset specifications separately from the RCC, YLPlugin and YLPkg containers. Images do not grant plugin permissions or publisher trust.
2. Start with an inert decoder that produces a bounded image result. Enforce limits on dimensions, pixel count, palette entries, referenced resources, dependency depth and output memory before allocating or rendering.
3. Keep asset access inside the active game's validated resource namespace. Reuse `GameResources` ownership and package validation instead of exposing arbitrary filesystem access.
4. Keep existing PNG/SVG and raw RCC games working. Do not replace their assets, change existing associations or require a new engine package merely to install an older game.
5. Add exact pixel fixtures for every supported depth/representation, transparency, packed rows and tail bits. Reject truncation, arithmetic overflow, out-of-range indices, cycles and unsupported required representations.
6. Measure decoding, rendering and memory use on desktop and mobile before choosing a Qt image provider or plugin boundary. Document deployment requirements independently for Flatpak, Android and any future HarmonyOS package.

This work can inform later patches, but no new public decoder API or format compatibility claim is enabled by this plan. Reliability and release qualification remain higher priorities.
