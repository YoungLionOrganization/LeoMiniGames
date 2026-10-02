# Runtime HTTPS

GameRuntime.supports("network.https") describes host support, not whether the current session is allowed online. Installed API 0.7 games opt in through capabilities:["network.https"]; add it to required_capabilities only if gameplay truly requires it. Legacy installed v0.5/v0.6 games keep the historical HTTPS path. Developer-local RCCs remain network-disabled even when they declare the capability.

The external QQmlEngine network manager accepts HTTPS only when the host grants permission. Use Qt/QML HTTPS requests with success/error/timeout/offline handling. There is no separate GameNetwork context API in 0.7.3, and the launcher does not give mods Developer Lab keys or publisher credentials. Certificate verification remains enabled.

Mod-provided requests still need their own response-size/time budgets and cancellation; the launcher's catalog limits do not automatically implement your game's protocol. Do not present a developer-local failure as evidence that an installed network-enabled package is broken. Existing backend/reference sources in this repository are not proof of a production deployment. Local unit/SDK tests run offline; connected integration tests require the actual configured backend.
