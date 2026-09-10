# Apply this GitHub patch

Overlay this directory on the LeoMiniGames repository root.

Recommended flow:

```bash
git switch main
git pull --ff-only
git switch -c github/release-tooling-v070
# copy this patch directory's contents into the repository root
git rm --ignore-unmatch LeoMiniGames.zip
git add .github .gitattributes .gitignore COPYRIGHT NOTICE README.md CONTRIBUTING.md CHANGELOG.md docs licenses mod-sdk tools/package tools/validate_distribution.py
git commit -m "Fix CI and add release artifact tooling"
git push -u origin github/release-tooling-v070
```

Then open a PR and merge only after CI passes.

`LICENSE` must remain the verbatim GNU GPL v3 text. Project-specific legal text is intentionally stored in separate files.

The historical root `LeoMiniGames.zip` should be removed from Git tracking. Generated source/build archives belong in Actions/Release assets.

The ChatGPT GitHub connection could read this public repository, but its GitHub App installation was not authorized for `YoungLionOrganization`; branch creation returned HTTP 403. Give the GitHub app access to `YoungLionOrganization/LeoMiniGames` before asking ChatGPT to push this patch directly.
