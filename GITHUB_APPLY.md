# Applying LeoMiniGames source updates

This file describes how to apply a prepared LeoMiniGames source package or patch without reviving obsolete files from older releases.

## Recommended Git flow

```bash
git switch main
git pull --ff-only
git switch -c maintenance/v0.7.1-docs
# overlay the prepared files, or apply the supplied patch
git add -A
git status
git diff --cached
git commit -m "docs: finalize v0.7.1 release guidance"
git push -u origin maintenance/v0.7.1-docs
```

Merge only after CI passes. If the change affects release metadata or build outputs, rerun **Build Release Artifacts** for the new exact `main` SHA before publishing.

## License state

The current LeoMiniGames host/core license is `LicenseRef-LMG-SAPEL-1.0` in `LICENSE`. Do **not** restore the historical GPL application license, Plugin Exception or old F-Droid submission material as current policy. Historical valid grants remain documented in `LICENSE_HISTORY.md` and `MIGRATION_FROM_GPL.md`.

## Generated archives

Do not commit generated source/build ZIPs into the repository root. Generated binaries and source archives belong in GitHub Actions artifacts and GitHub Releases.

## Release after applying a patch

For a release-affecting patch:

1. merge the patch to `main`;
2. wait for exact-SHA CI success;
3. rerun **Build Release Artifacts**;
4. run **Publish Release** with `mode=validate`;
5. run **Publish Release** with `mode=publish`;
6. approve the protected `release` Environment if GitHub asks for approval.

See `docs/GITHUB_RELEASES.md` for the canonical operator procedure.

## Connected GitHub tooling

A ChatGPT/GitHub connector may be able to inspect the repository and workflow logs while still lacking repository-content write permission. A `403 Resource not accessible by integration` on branch/file creation is an authorization limitation of that connection, not a Git error in this repository.
