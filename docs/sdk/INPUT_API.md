# Input API

`GameInput` centralizes action state and physical keyboard mapping. Arrow keys remain logical keys. Physical WASD uses host/native scan-code mapping where available so layouts such as AZERTY, QWERTZ, Azerbaijani, Turkish, and Russian do not force every game to maintain platform scan-code tables.

Games should consume actions (`up`, `down`, `move_left`, `move_right`, `fire`, `pause`) rather than comparing only `Qt.Key_W/A/S/D`. Logical-key fallback remains for synthetic events and standard layouts.

Touch, drag, swipe, controller-like on-screen controls, focus, and animation-state gating remain game-level concerns. Minimum touch target and safe-area behavior must be checked on mobile.
