# LeoMiniGames v0.7.2 source validation

This archive is the **application source**, including Android/desktop build
instructions and tests. It is not a signed installer, APK or AAB.

## Changes

- Developer Lab can remember an API key only when opted in, using QtKeychain and
  the operating-system credential store. Restored keys are revalidated; logout
  cancels pending verification. A failed keychain operation does not fall back
  to plaintext storage.
- Save-slot creation no longer clobbers a previous slot. Recursive snapshot
  normalization prevents nested JavaScript references from escaping into JSON.
- Metadata and update responses are bounded in bytes and time, and catalog
  endpoints reject malformed or unsafe URL forms.
- Android disables cleartext traffic, excludes credentials from device backup,
  and packages pinned, SHA-256-verified OpenSSL 3.5.8 libraries for each ABI.
  Artifact validation checks ELF architecture, TLS libraries and 16 KiB page
  alignment. Release packaging requires an existing signing identity.

## Verification performed

- Qt 6.8.3 Linux build: success.
- CTest: 8/8 passed (catalog, compatibility, save, network and developer
  authentication races).
- Source, distribution, localization and security validators: run by
  `python3 tools/package/package_source.py`; all must pass before this ZIP is
  emitted.

The local environment did not have an Android SDK/NDK or signing identity. On
2026-09-28, CI and Build Release Artifacts completed successfully at commit
`a020647bfd7dacb49727b4683bede6807eec7d67` and the release preflight
validated 27 public assets. The subsequent publish job failed on the missing
`release/notes/v0.7.2.md`; this source adds that file and validates its presence.
The updated source needs fresh exact-commit CI/build runs. Device HTTPS checks
and deployment of the separate backend remain production integration steps.
