# LeoMiniGames application updates

## v0.7.1 update boundary

LeoMiniGames clients do **not** consume GitHub Releases or `raw.githubusercontent.com` directly.
The only public update authority used by the application is:

```text
https://leominigames.younglion.xyz
```

The backend currently uses GitHub as its upstream release/update provider. That is an implementation detail behind the server contract and can later be replaced by R2, a YoungLion CDN, or another package service without changing the application-side update API.

## Client metadata request

The application calls:

```text
GET /api/v1/updates/v1/check
    ?channel=stable|preview
    &platform=windows|linux|android|macos
    &arch=...
    &install=portable|installer
    &current=<application-version>
```

The server selects the release and recommended package and returns only `leominigames.younglion.xyz` URLs. The client validates that release-page, download and maintenance-repository URLs are HTTPS URLs on this exact host before using them.

The response contract is versioned as `leominigames-update-v1`. A provider migration must preserve this public contract or introduce a separately versioned contract.

## Package downloads

Portable/non-QtIFW downloads use a backend URL of the form:

```text
/api/v1/updates/v1/releases/<tag>/assets/<asset-name>
```

The backend resolves the requested exact release asset against its configured provider and streams it to the client/browser. The application never receives or opens the provider's direct asset URL.

The current GitHub provider consumes GitHub's release metadata internally and can use GitHub's asset digest as SHA-256 metadata when present. Binary responses are streamed rather than buffered into PHP memory.

## Windows QtIFW repository

Windows Qt Installer Framework installations use only backend repository URLs:

```text
/updates/qtifw/stable/windows/x86_64
/updates/qtifw/stable/windows/ARM64
/updates/qtifw/preview/windows/x86_64
/updates/qtifw/preview/windows/ARM64
```

`Updates.xml`, `.7z` payloads and `.sha1` files are proxied by the backend from the configured upstream provider. The installer and Maintenance Tool therefore do not embed a GitHub repository URL.

The guarded `Publish Release` workflow still publishes the generated QtIFW repository to the repository's internal `updates` branch while GitHub remains the configured upstream. Moving that storage later does not require a LeoMiniGames client update as long as the backend routes above remain stable.

## Stable and Preview

- **Stable** resolves the latest published non-prerelease release.
- **Preview** resolves the highest valid SemVer release visible to the backend, including `alpha`, `beta` and `rc` prereleases.

The application still performs a local SemVer comparison before presenting an update so a malformed or older server response cannot trigger a downgrade.

## Backend resilience

The backend metadata layer uses a short cache and can serve a bounded stale cache if the upstream provider is temporarily unavailable. Update metadata has a strict size cap. Release tags, asset names and QtIFW relative paths are validated before provider access, and the public routes cannot be used as an arbitrary URL proxy.

## Release pipeline

`Publish Release` remains manual-only. Publish mode requires the exact `main` SHA to have successful CI and Build Release Artifacts runs, validates the public asset set, publishes the QtIFW repository, and only then makes the draft release public.

## Release workflow recovery

`Publish Release` identifies draft releases by numeric release ID, not by the published-tag endpoint. This is intentional: a GitHub draft may use an `untagged-*` page and return 404 from `releases/tags/{tag}` until it is published. A retry for the same exact source SHA reuses the matching draft, deletes partial assets, uploads the validated 25-file public asset set again, publishes the QtIFW repository, and only then flips the draft public. If the matching draft belongs to an older SHA, the authorized workflow replaces that stale draft before creating the current release transaction.
