# LeoMiniGames application updates

## v0.7.1 servicing model

LeoMiniGames uses two update paths:

- Windows installations created by the Qt Installer Framework use `LeoMiniGamesMaintenance.exe` and a GitHub-hosted QtIFW repository.
- Portable and non-QtIFW distributions use the in-app GitHub release checker and open the matching public release package or release page.

The application update checker supports `stable` and `preview` channels. Stable ignores GitHub prereleases. Preview accepts SemVer prereleases such as `0.8.0-alpha.1`, `0.8.0-beta.1` and `0.8.0-rc.1`.

## QtIFW repository

Windows packages are built as hybrid installers. `repogen` creates the repository together with the installer. The final build workflow keeps this repository in the internal `update-repositories` artifact; it is intentionally not a public GitHub Release asset.

The guarded `Publish Release` workflow publishes the repository to the repository's `updates` branch:

- `stable/windows/x86_64`
- `stable/windows/ARM64`
- `preview/windows/x86_64`
- `preview/windows/ARM64`

A stable installed client normally uses the stable repository stored by the installer. When the application is switched to Preview, `UpdateService` starts Maintenance Tool with the preview repository as a temporary repository.

## GitHub release safety

`Publish Release` is manual-only. Publish mode additionally requires:

1. current SHA equals `main` HEAD;
2. exact-SHA successful CI;
3. exact-SHA successful Build Release Artifacts workflow;
4. a validated `release-assets` artifact;
5. authorized publisher allowlist membership;
6. approval through the protected GitHub `release` Environment;
7. no pre-existing tag or release.

The release is created as a draft, assets are uploaded and verified, the QtIFW repository is published, and only then is the draft made public.

Configure **Settings → Environments → release** in GitHub with the intended Required Reviewer(s). If the same maintainer must approve their own deployment, GitHub's `Prevent self-review` setting must not conflict with that policy.
