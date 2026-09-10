# Backward-compatibility regression corpus

These source fixtures model the package/API surfaces that LeoMiniGames v0.5.x and v0.6.x exposed. `v0_5_canonical`, `v0_5_legacy_prefix`, and `v0_6_services` must remain accepted; `v0_7_modern` exercises explicit API negotiation; `malicious_legacy_namespace` must remain rejected because it escapes its `/mods/<id>` namespace.

The static validator runs without Qt. On a Qt-enabled build machine, `tools/build_compat_fixtures.sh` can compile the positive fixtures to RCC and runtime tests can mount them through `RccPackageInspector`.
