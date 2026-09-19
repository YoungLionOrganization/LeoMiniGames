# Recommended GitHub repository settings

These settings are repository metadata/administration settings rather than source files.

## About

**Description**

> Modular cross-platform Qt 6 mini-game platform with RCC games/mods, themes, developer SDK, backward compatibility and an offline-first runtime.

**Website**

`https://leominigames.younglion.xyz`

**Topics**

`qt6`, `qml`, `cpp`, `minigames`, `gaming`, `modding`, `rcc`, `cross-platform`, `windows`, `linux`, `android`, `ios`, `macos`, `source-available`

Use the LeoMiniGames square icon or logo as the repository social preview image.

## Actions

Keep the default workflow token at read-only unless a specific workflow needs write access. The current CI and manual artifact builder need only `contents: read`.

Configure Android signing values as **Actions repository secrets**, never Variables and never committed files. See `GITHUB_RELEASES.md`.

## Main-branch protection / ruleset

Recommended baseline:

- block force-pushes and branch deletion on `main`;
- require the `Static validation` status and the platform build statuses before merging substantial changes;
- keep administrators able to perform documented emergency fixes if desired;
- signed commits are optional, but release tags/commits should ideally be signed once the project has a stable signing process;
- do not require a PR for every solo-maintainer edit unless that workflow suits the project, but never bypass required CI for release candidates.

## Releases

The repository intentionally uses a manual build-artifact workflow rather than automatic tag-triggered release publication. After a successful artifact run, attach the resulting files to a GitHub Release manually.

## Source archives

Do not commit generated release/source ZIP files to the repository. Git already stores the source history, and GitHub Actions/Release assets are the appropriate place for generated archives. The historical root `LeoMiniGames.zip` should be removed from the tracked tree once this patch is applied.
