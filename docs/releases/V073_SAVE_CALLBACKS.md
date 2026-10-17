# v0.7.3 save callback isolation

17 October 2026. Follows [session/save integrity](V073_SESSION_INTEGRITY.md), [installer recovery](V073_INSTALLER_RECOVERY.md) and [audio callback fixes](V073_AUDIO_CALLBACKS.md).

Save migrations and value snapshot getters execute JavaScript. If a callback closed/replaced the game or retired its engine, an older load/restore could still apply the returned payload to the replacement session. A save completion observer could also change games before the old operation cleared state or published its remaining notifications.

Save operations now capture the game/engine generation. A change during migration, snapshot conversion, error clearing or completion notification cancels further work and returns false. Migration targets remain fixed for the request. Backup restoration protects its slot before invoking migrations so a close-triggered forceSave cannot overwrite recovery data during the callback. Interrupted operations do not clear the replacement session's dirty state or publish old completion events.

Cancellation does not undo a filesystem commit that completed before a notification changed sessions. The completed write belongs to its original game; the host stops publishing its retired result and leaves the replacement session intact.

Native regressions reproduced `session switch cancels migration operation` and `session switch during save notification retires the request` failures before the fix. Coverage includes load and backup restoration across a session change, a close-triggered save during migration, engine retirement, snapshot getters changing games, stale saved notifications, and replacement dirty state surviving an error-clear callback. Source-file assertions compare both recovery files byte-for-byte.

The private Qt 6.10.2 Windows x86_64 Debug kit with Multimedia passed the full native suite, 22/22. Source/project/sanity/documentation validators passed; distribution reported 300 PASS. Fresh GitHub outcomes are tracked in PR #7. No production backend deployment or public release is included.
