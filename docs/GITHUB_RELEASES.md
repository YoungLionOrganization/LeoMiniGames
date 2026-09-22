# GitHub Releases and release operator guide

This document describes the v0.7.1 release pipeline. The pipeline is intentionally manual at the publication boundary, but asset collection, validation and GitHub Release publication are automated after the maintainer selects publish mode and approves the protected environment.

## Pipeline overview

| Workflow | Trigger | Purpose | Publishes a Release? |
| --- | --- | --- | --- |
| `ci.yml` | push / PR / manual | Static validation plus cross-platform compile/test coverage | No |
| `build-artifacts.yml` | manual | Builds the exact v0.7.1 artifacts, `release-assets` and QtIFW `update-repositories` | No |
| `publish-release.yml` | manual | Validates release gates; in publish mode creates and publishes the GitHub Release | Yes, only in `publish` mode |

Release metadata comes from `release/release.json`. The public asset contract comes from `release/assets.json`. Authorized release publishers come from `release/authorized_publishers.json`.

## Before starting

Confirm all of the following:

- the intended release commit is the current `main` HEAD;
- `CMakeLists.txt`, `release/release.json`, installer package/config metadata and release notes agree on the version;
- `release/release.json` points to the intended tag and release notes;
- the `release` GitHub Environment exists if approval is required;
- the operator is listed in `release/authorized_publishers.json`;
- Android signing secrets are configured when signed Android artifacts are required.

For v0.7.1 the authorized publisher is `Cavanshirpro`.

## Step 1 — get the exact `main` SHA green

Push the final release source to `main` and wait for **LeoMiniGames CI** to finish successfully. Do not publish from an older green run: `Publish Release` checks the exact workflow SHA against the current `main` HEAD.

If `main` changes later, the release gates must be rerun for the new SHA.

## Step 2 — run Build Release Artifacts

Open:

**GitHub → LeoMiniGames → Actions → Build Release Artifacts → Run workflow**

Select branch **main** and run it. v0.7.1's workflow does not ask for a version input; `LMG_VERSION`, CMake and `release/release.json` are checked against one another.

Wait until the whole workflow is green. In addition to per-platform artifacts it must produce:

- `release-assets`
- `update-repositories`

`release-assets` is the only artifact set used for public GitHub Release upload. The current allowlist expects 25 public files. Files such as `*.aab`, `*.sha256`, `*-unsigned.zip`, `*-Legal.zip`, `Updates.xml` and repository metadata are intentionally not public Release assets.

## Step 3 — validate the release gates

Open:

**Actions → Publish Release → Run workflow**

Choose:

- Branch: **main**
- `mode`: **validate**

Run the workflow. `Release preflight` must pass. In this mode the job named **Publish approved release** is expected to show **Skipped**. That is deliberate because the workflow contains:

```yaml
if: ${{ inputs.mode == 'publish' }}
```

Validation mode does not create a tag, draft or public release. It proves that the exact-SHA CI/build runs, release assets, update repositories and duplicate guards are ready.

## Step 4 — publish

Run **Publish Release** again, but this time choose:

- Branch: **main**
- `mode`: **publish**

The preflight reruns. After it passes, **Publish approved release** starts. If the `release` Environment has required reviewers, GitHub changes that job to a waiting state.

On the workflow run page:

1. click **Review deployments**;
2. select the `release` environment;
3. optionally add a comment;
4. click **Approve and deploy**.

If the same maintainer is supposed to approve their own release, repository environment policy must not block self-review. Check **Settings → Environments → release** and ensure the required-reviewer/self-review configuration matches the intended policy.

## What publish mode does

After approval, the workflow:

1. verifies that `main` still points to the exact preflight SHA;
2. rechecks the authorized publisher and duplicate tag/release guards;
3. downloads the already validated release assets;
4. creates the GitHub Release as a **draft**;
5. uploads the entire validated public asset set;
6. compares remote Release asset names against the validated local set;
7. publishes the QtIFW Windows repository to the `updates` branch under the configured track;
8. makes the draft public only after those checks succeed.

For stable v0.7.1, the Windows update repository is published below:

```text
stable/windows/x86_64
stable/windows/ARM64
```

Preview releases use the corresponding `preview/windows/...` paths.

If the GitHub Release publication step fails after the update repository was pushed, the workflow attempts to roll the `updates` branch back to its previous state.

## Why "Publish approved release" may look stuck or skipped

### It says Skipped

You launched `Publish Release` with `mode=validate`. Nothing is wrong. Start a new run with `mode=publish`.

This is exactly what happened in **Publish Release #1** on 2026-09-22: `Release preflight` succeeded, while `Publish approved release` was skipped because the run was validation-only.

### It says Waiting / Awaiting approval

The protected `release` Environment is waiting for a reviewer. Use **Review deployments → release → Approve and deploy**.

### It fails with "main advanced after preflight"

Someone pushed to `main` while the release waited for approval. This is fail-closed behavior. Run CI and Build Release Artifacts for the new `main` SHA, then rerun Publish Release.

### It fails because a tag or public release already exists

A real tag or already-public release remains immutable and is a hard stop. A **draft** with the same tag is different: the workflow now recovers it when it targets the current exact SHA, or deletes/replaces it when it is a stale draft from an older pre-release SHA. Do not manually overwrite an already-public release.

### No successful exact-SHA workflow is found

The current `main` SHA does not have a successful required CI or Build Release Artifacts run. Run the missing workflow against current `main` and wait for success.

## Android signing secrets

Never commit a `.jks`/`.keystore` file or passwords. Configure repository Actions secrets with these exact names:

```text
ANDROID_KEYSTORE_BASE64
ANDROID_KEY_ALIAS
ANDROID_KEYSTORE_PASSWORD
ANDROID_KEY_PASSWORD
```

On Windows PowerShell, encode an existing keystore as one-line Base64:

```powershell
$bytes = [IO.File]::ReadAllBytes("C:\path\to\your-upload-key.jks")
[Convert]::ToBase64String($bytes) | Set-Content -NoNewline .\keystore.base64.txt
```

Retain the correct signing/upload key for application update continuity. If Google Play App Signing is enabled, CI normally uses the registered upload key rather than Google's protected app-signing key.

## Current v0.7.1 state

The most recent inspected release candidate before this workflow fix was commit `bd9bd54833850a0a74cf5d9fd504c6833a15f522` on 2026-09-22:

- LeoMiniGames CI #23: **success**
- Build Release Artifacts #10: **success**
- Publish Release #3 preflight: **success**
- Publish Release #3 publish job: **failed after draft creation/upload** because the workflow tried to resolve the still-draft release through `releases/tags/v0.7.1`, which returned HTTP 404.

The corrected workflow carries the numeric draft release ID instead. After this fix is committed, the commit SHA changes, so CI and **Build Release Artifacts** must be run again for the new exact SHA before `Publish Release` is run in `publish` mode.

## v0.7.1 draft-release retry behavior

The publish transaction is retry-safe. GitHub does not expose a draft release through the normal `releases/tags/{tag}` REST lookup used for published releases. A failed publish can therefore leave an `untagged-*` draft even though the final tag does not exist yet. The workflow resolves drafts from the release collection, recovers a draft that targets the exact current `main` SHA, removes any partial asset set, re-uploads the validated assets, and carries the numeric release ID into the final publish step. If a draft with the same tag targets an older SHA, the authorized publish transaction deletes that stale draft and creates a fresh draft for the current exact SHA.

If `Prepare draft and upload exact asset set` fails, fix the source/workflow issue, obtain successful CI and **Build Release Artifacts** for the resulting exact SHA, and rerun `Publish Release` in `publish` mode.

