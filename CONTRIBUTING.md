# Contributing

1. Keep RCC v1 backwards compatible. Do not make a new manifest field mandatory without a major compatibility decision.
2. Do not store progress, score, inventory, save slots, campaign state or achievements in QSettings.
3. Treat publisher verification, review status, native trust and Level 3 as security metadata; never trust a plugin manifest for these decisions.
4. Keep QML safe for transient zero/invalid geometry. Prefer `Viewport.safeWidth`, `Viewport.safeHeight` and positive font/radius bounds.
5. Run `python tools/validate_v060.py`, `python tools/source_guard.py`, `python tools/sanity_check.py` and `python tools/validate_project.py` before submitting changes.
6. New dependencies must be FLOSS-compatible and documented, with Android/F-Droid implications considered.
