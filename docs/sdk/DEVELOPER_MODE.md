# Developer Mode / Developer Lab

Developer Lab is intentionally separate from normal installation. A local RCC is accepted only after a developer session is verified, copied into a session cache, hashed, structurally inspected, checked for id collisions/API compatibility, and mounted with native/L3 disabled.

The current backend contract available to this source tree supports scoped `lmg_...` developer API-key verification. The raw key is used only for the verification request, is not persisted in QSettings/disk, redirects are rejected, and authentication responses are size-bounded. A browser OAuth native-app callback is not fabricated when the server does not provide that contract.

Local Developer RCC network access is disabled. Diagnostics can expose QML/runtime problems but external `GameLogger` cannot export arbitrary files. Device profiles simulate common viewport/safe-area sizes; they do not replace physical-device testing.
