# Local stats, achievements, haptics and logging

GameStats: highScore([name]), submitHighScore(score[,name]), counter(name), setCounter(name,value), increment(name[,amount]), gamesPlayed(), totalTimeMs(); changed notifies views. submitHighScore returns whether a higher local score was accepted, not proof of a server submission or disk commit. A default high score starts at 0, so lower-is-better results need your own metric/save rule. The host tracks session count/time; totalTimeMs uses session wall time and is not a gameplay pause-aware stopwatch—use GameClock for active time. Bound metric names/data; local stats are not a leaderboard/cloud sync.

Achievements: unlock(id), isUnlocked(id), progress(id), setProgress(id,value), changed/unlocked(id). Progress is clamped to 0…1; unlock is local, idempotent and marks full progress. Manifest metadata does not define a server achievement program. Stats/achievements persist separately from GameSave, so they are not an atomic transaction with a board snapshot.

Signed counter arithmetic saturates at 64-bit limits. A rejected metric insertion returns the stored counter value. New metrics reserve space for session accounting; full legacy stores preserve existing metrics instead of exceeding the reload limit. Repeated host session starts/ends are idempotent. Achievement progress ignores NaN/infinity; invalid stored numeric progress reads as zero.

Haptics.available, light(), medium(), heavy(), pulse("light"|"medium"|"heavy"). Android has the vibration implementation; unsupported platforms are no-op. Check available when useful, and keep haptics optional rather than blocking gameplay.

GameLogger.log(level,message[,source[,line]]) and lastError. Suggested levels are debug/info/warning/error. QML engine warnings are also captured by the host. The external facade cannot export arbitrary files; use Developer Lab's host diagnostics. Do not log credentials or assume a missing physical output device is a save failure.
