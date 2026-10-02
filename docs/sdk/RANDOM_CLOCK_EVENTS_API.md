# Random, clock and events

## GameRandom

seed(text), nextInt(minInclusive,maxInclusive), nextReal() in [0,1), chance(probability), pickIndex(count) and shuffled(list). state is a writable hexadecimal string. For deterministic continuation, save GameRandom.state with your board and restore it before drawing another random value. Do not seed again after restoring. The service is shared in the host process; seed a new game explicitly or restore its state. It is not a cryptographic random API.

## GameClock

reset(), pause(), resume(), elapsedMs(), elapsedSeconds(), paused, timeScale. The host resets on lifecycle start and connects pause/resume. reset restores normal timeScale/paused state. elapsed getters are methods, not tick signals; use a Timer/render tick for UI refresh. timeScale is bounded 0…8; 0 freezes accumulation. The clock is in-memory active-session timing, not a persisted timestamp; store accumulated game time in GameSave for reopen.

## GameEvents

emitEvent(name,payload) emits eventEmitted(name,payload). Use plain data and namespace event names for game components (HUD, quests, achievements). Events are not persisted/replayed and are not a public cross-plugin network/event transport. Consume them only during your active session.
