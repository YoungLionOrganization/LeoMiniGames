# Next patch priorities

Reviewed 30 October 2026 against the current source, the earlier release plans and the maintainer's later YLPlugin/HarmonyOS architecture discussions. These are staged proposals, not features already supported by v0.7.3.

## Complete v0.7.3 first

- Exact-source native CI, signed package builds and the strict 39-file public asset validation.
- Real deployed Linux package and Windows installer servicing evidence; keep incomplete device checks visible.
- Matching platform QA for the immutable candidate, followed by release preflight and publication through the protected release workflow.
- User installation documentation included in the desktop payloads and accessible from the repository.

The public mods/themes services returned 200 and schema health reported ready during this review. That resolves the earlier observed HTTP 500 blocker as an external-state change; this application branch did not deploy a backend repair.

## v0.7.4: reliability before new formats

| Priority | Work | Acceptance |
| --- | --- | --- |
| High | Investigate failures found by the new deployed-package tests and Qt 6.12 lane | Reproduce each failure and keep the existing API 0.7/legacy fixture suite passing |
| High | Make candidate platform evidence easier to collect and review | Source SHA and each package digest are bound to real tests; missing results remain missing |
| High | Keep package selection, update/recovery and diagnostics clear | Native/Flatpak data boundaries and install format remain visible to users |
| Medium | Evaluate moving release kits to Qt 6.12 after the separate desktop lane is stable | Windows/Linux/macOS and later Android packaging all pass before changing the baseline |
| Lower | HarmonyOS native-port experiment | Follow the [platform checklist](HARMONYOS.md); advertise support only after HAP/device evidence |

v0.7.4 must not turn a package-format experiment into a forced migration for existing games. Save Transfer remains a separately planned feature for the v0.8 line unless its scope is explicitly changed.

## YLSpec / YLPlugin / YLPkg boundaries

The maintainer's later design separates three responsibilities:

- **YLSpec** carries inspectable metadata, profiles and required-feature declarations. Its fixed frame header is separate from the variable-length document. A valid document does not authorize code execution.
- **YLPlugin** is the future plugin container. It can carry an RCC payload, expose metadata without running the plugin and select different host entrypoints. Compression/encryption and native payloads require their own reviewed container contract.
- **YLPkg** manages installation/dependencies and can deliver optional engine/API/SDK components. It must not be required just to run a YLPlugin and must not grant arbitrary host privileges.

The [YLSpec profile specification](https://github.com/YoungLionOrganization/YLSpec/blob/main/docs/profiles.md) was checked during this review. Profiles define field meaning without changing the document envelope. Unknown profiles may be inspected, while validation requires an explicitly registered matching profile.

### Groundwork for a later integration

1. Define an inert package-inspection result between container parsing and host policy. Do not load code merely to inspect it.
2. Keep raw RCC as a supported input. A YLPlugin RCC adapter should reuse `RccPackageInspector`, `PackageCompatibility` and existing scoped runtime services rather than duplicating them.
3. Match the LeoMiniGames host target/API before choosing an entrypoint from a multi-host plugin. Unknown required runtimes/features fail with a clear diagnostic.
4. Interpret permissions in the selected host's versioned registry. The later design uses names or decimal numeric values such as `37` / `[37, 8]`; metadata cannot mint Official/Verified/L3 trust.
5. Preserve namespace, size, digest, rollback and active-package ownership checks. Encrypted/paid content does not bypass those checks.
6. Keep native/L3 review separate from container acceptance. Mobile/platform restrictions continue to apply.
7. Add compatibility fixtures proving old raw RCC and wrapped RCC behavior before exposing a new public file association or catalog format.

No YLPlugin parser, encryption scheme, YLPkg installer or new permission encoding is enabled by this document. The current runtime and market continue to use the documented RCC contract.
