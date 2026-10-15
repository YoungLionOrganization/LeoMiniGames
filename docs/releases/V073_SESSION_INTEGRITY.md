# v0.7.3 session and save integrity follow-up

15 October 2026. Continues [the earlier runtime fixes](V073_RUNTIME_FOLLOWUPS.md) on PR #7. The earlier commit `9382f02` completed both GitHub CI and all-platform artifact validation successfully. The changes below require fresh results for their new commit.

## Save slot removal

Explicit deletion clears failed-load/recovery protection only for the removed slot. Primary and backup paths must be regular files; unrelated directories and links are rejected before changing either file. Both files are staged in a private directory before the live slot is retired. A failed rename restores staged files; if rollback itself fails, recovery files are retained and their location is reported. Cleanup failure after retirement is reported separately.

Regression evidence includes deleting a protected corrupt slot and recreating it, a directory at the backup path, repeated deletion of a missing slot, and a Windows primary opened without delete sharing. The locked-primary test verifies that the already staged backup is restored byte-for-byte.

## Numeric services and statistics

NaN/infinity no longer unlock achievements, accelerate the game clock or create phantom input. Invalid achievement/clock assignments retain valid state; invalid axis input releases the action. Signed counter arithmetic saturates rather than overflowing. Repeated session starts preserve the original timer and count once. New metrics reserve space for host session fields; old full stores retain existing metrics instead of growing beyond the reload limit. Rejected counter insertion returns the actual stored value.

The original service regression reported 11 failures out of 12 assertions, including counter overflow and duplicate session accounting. Additional tests cover a 4096-entry legacy stats file across session accounting and reopening.

## Reentrant lifecycle and game opening

Lifecycle notifications belong to a session generation. A hook that closes, unloads or replaces the session cannot emit a stale completion signal afterward. Recursive invocation of the same hook is suppressed. Unloaded observers see cleared state and can attach the next session safely. Nested pause/resume notifications reflect the resulting state.

Game opening checks its generation after notifying observers: an observer that closes or replaces a game cannot leave a late `gameOpened` event or a misleading success result. The host clears GameStats context when a game closes, and the six-game smoke test checks it.

The original reentrancy regression reported 10 failures out of 10 assertions. Both external-session fixtures and builtin smoke tests remain part of the full native suite.

## Windows test isolation

SettingsManager accepts an explicit INI filename for host tests. Audio, catalog, external-session and SDK tests now use their temporary settings file. This fixes repeated Windows runs reading settings from the previous run's registry profile; XDG variables alone do not isolate QSettings there. Normal application construction retains native settings storage.

Native verification uses the existing Qt 6.10.2 Windows x86_64 Debug kit with Multimedia. Full native suite and fresh GitHub run outcomes are recorded in PR #7. Physical device, keyring and release-signing checks remain release QA requirements.

The expanded native CTest suite passed **21/21**. Source/project/sanity/documentation validators passed; distribution reported 300 PASS, SDK tooling 10/10 and package validators 16/16. The Windows audio test's stale-registry failure was reproduced before explicit INI isolation and passed afterward. i18n retains 22 coverage warnings with zero errors.
