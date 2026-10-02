# Lifecycle

The host attaches the external root Item and invokes optional zero-argument root methods. It also emits matching Lifecycle signals. Choose one place for a side effect: implementing `function save()` and also saving in `onSaveRequested` causes duplicate work.

| Root callback | Signal | Host use |
| --- | --- | --- |
| load | loaded | After QML creation / Component.onCompleted; restore state here. |
| start | started | Start once after load. |
| pause | paused | Inactive/manual pause; gate input/timers. |
| background | backgrounded | Host goes inactive/hidden/suspended. |
| save | saveRequested | Serialize synchronously; host subsequently force-saves. |
| foreground | foregrounded | Active again, before resume. |
| resume | resumed | Resume active gameplay. |
| close | closed | Once during coordinated close. |
| unload | unloaded | Release game resources; host clears root/engine. |

Typical startup is create → Component.onCompleted → load → start. Background is pause → background → save → forceSave; foreground is foreground → resume. Close is save → forceSave → close → unload → host engine/resource retirement. Multiple OS state notifications can emit background/save more than once, so make hooks idempotent. Pause/resume/start/close are guarded by the host.

```qml
property bool sessionPaused: false
function pause() { sessionPaused = true }
function resume() { sessionPaused = false }
Timer { interval: 1000; repeat: true; running: !sessionPaused; onTriggered: tick() }
```

Host GameClock and GameAudio follow pause/resume. Your QML Timer/animations/requests still need game-specific gating. Read clocks for elapsed active time instead of wall-clock differences. GameInput resets on focus/background; do not replay stale held actions.

Lifecycle.pause/resume can request a manual pause/resume. Do not call them recursively from their own callbacks. Use **App.closeGame()** to navigate out and run the host teardown; calling Lifecycle.close/unload alone is not navigation. External games do not receive attach(). Missing hooks are legal in legacy content.
