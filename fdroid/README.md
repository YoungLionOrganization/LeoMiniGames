# F-Droid packaging notes

`metadata/xyz.younglion.leominigames.yml.example` is intentionally a template, not an active fdroiddata recipe.

F-Droid's current build metadata guidance requires the exact source commit; a branch/tag placeholder must not be submitted. Before an fdroiddata merge request:

1. Publish the canonical GPL-3.0-or-later source repository.
2. Replace `REPO_URL` and `FULL_COMMIT_SHA` with the real values.
3. Pin/build the required Qt 6 Android modules from FOSS source in the fdroiddata recipe; do not bundle Qt online-installer binaries.
4. Run `tools/audit_prebuilt_binaries.py`, `tools/validate_v070.py`, and a clean Android build.
5. Add real application screenshots under upstream Fastlane metadata. Do not use mock screenshots.

The source tree already contains a tracker/ads-free `LEOMINIGAMES_FDROID` CMake option. It does not enable proprietary SDKs.
