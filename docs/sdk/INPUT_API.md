# GameInput

Consume actions instead of platform keycodes/scancodes. Methods are `press(action[,value])`, `release`, `setValue`, `isPressed`, `value`, `reset`; values are clamped to −1…1. Signals are actionPressed(action,value), actionReleased(action), actionValueChanged(action,value). A press is a state transition, not a repeating key timer; continuous motion should read isPressed/value during game ticks.

| Physical/logical input | Action |
| --- | --- |
| Left / physical A | move_left |
| Right / physical D | move_right |
| Up / physical W | up |
| Down / physical S | down |
| Space / Return / Enter | fire |
| Escape | pause |

Physical WASD is mapped by the launcher for keyboard layouts; logical fallback supports synthetic events. The host installs one event filter scoped to the game focus subtree and avoids TextInput/TextEdit. It tracks multiple held physical keys for one action and resets on focus/background. `jump` is a usable custom action name, not an automatic default key mapping.

```qml
Connections {
    target: GameInput
    function onActionPressed(action, value) {
        if (action === "fire" && !sessionPaused) performAction()
    }
}
```

Touch/swipe/controller mapping belongs to the game. On-screen held controls can press/release a named action; release on cancellation as well as pointer release. Do not call handleKey/setFocusRoot or add per-platform scan tables: those are host integration operations. Qt controls also process Space/Return; avoid executing a global fire action and a focused button's click for the same key. The modern SDK example routes both paths once.
