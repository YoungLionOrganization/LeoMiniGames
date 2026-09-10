# LeoMiniGames v0.7.0 F-Droid Readiness

Date: 2026-09-10

## Current readiness

The source tree contains an F-Droid-oriented CMake option, upstream Fastlane metadata, a clean-build helper, a reproducibility comparison helper and an fdroiddata YAML **template**. No proprietary SDK is intentionally required by the F-Droid variant.

## Verified source-tree conditions

- Version name/code: 0.7.0 / 700.
- License: GPL-3.0-or-later.
- `tools/audit_prebuilt_binaries.py`: PASS; no committed executable/library binary artifacts found outside excluded build/dist paths.
- English title/short/full description, changelog 700 and a real application icon exist under upstream Fastlane metadata.
- No fake screenshots are supplied.
- fdroiddata recipe remains a template with `REPO_URL` and `FULL_COMMIT_SHA`; it does not pretend that an immutable public source commit exists.

## Current F-Droid documentation checked

F-Droid's current Build Metadata Reference says the `commit` field should use the **full commit hash**, not a branch/tag name. The current submission guide also recommends upstream Fastlane-style metadata and release tags, while screenshots are sourced from upstream metadata.

References checked 2026-09-10:

- https://f-droid.org/docs/Build_Metadata_Reference/
- https://f-droid.org/docs/Submitting_to_F-Droid_Quick_Start_Guide/
- https://f-droid.org/docs/All_About_Descriptions_Graphics_and_Screenshots/

## Blocking items before an actual fdroiddata MR

1. Publish/identify the canonical public source repository.
2. Replace `REPO_URL` and `FULL_COMMIT_SHA` with real immutable values.
3. Confirm/pin a fully FOSS Qt-for-Android source build strategy acceptable to fdroiddata; do not use an opaque Qt online-installer binary as a build dependency.
4. Execute the clean F-Droid Android build in the actual F-Droid-like Linux environment and confirm output APK path/version.
5. Add genuine application screenshots from a real build under upstream Fastlane metadata.
6. Review anti-features against the final production network/backend behavior.

## Reproducibility

`tools/compare_reproducible.sh` is provided to compare two build outputs. Signing credentials are not embedded in the repository. Determinism still needs to be measured using two real Android builds; it is not claimed from static inspection alone.

## Result

**SOURCE PREPARATION: PASS WITH EXTERNAL BLOCKERS.** The repository-side preparation is in place, but an official F-Droid submission is not honestly possible until the canonical public commit, FOSS Qt build recipe, clean build result and real screenshots exist.
