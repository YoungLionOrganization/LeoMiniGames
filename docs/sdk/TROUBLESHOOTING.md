# SDK troubleshooting and acceptance

| Symptom | Check |
| --- | --- |
| rcc missing / Qt 5 selected | Pass --rcc with the actual Qt 6 executable; wrappers accept it as first argument. |
| Package mounts but entry/icon/translation is missing | Every file must be in qrc, not merely on disk. New internal prefix is `/`. |
| Host rejects API/capability | Distinguish api_version 0.7, app min_app_version 0.7.3, native ABI 1 and theme API 1. GameRuntime.capabilities is a property. |
| Unknown private QML module | Remove import LeoMiniGames; use package-local components and context services. |
| Save fails / future schema | Display lastError, retain original data, register complete migrations before load. Do not silently reset/delete. |
| Save twice on background | Use either root save() or onSaveRequested for serialization, not both. |
| UI does not update after theme/language change | Read GameTheme.revision / GameI18n.language inside the producing binding. |
| Keyboard increments twice | A focused Qt button also handles Space/Enter; gate global fire or route it to the same action once. |
| Short landscape screen clips content | Use a bounded layout and Flickable/ScrollView; test long locale/font scaling. |
| Audio fails on one platform | Use the host API and packaged local URL; examine audioError and deployed media backend/codec/device. Prefer PCM WAV effects. |
| HTTPS denied in Developer Lab | Local sessions are network-disabled; this is not an installed opt-in permission grant. |
| Native library not loaded | Build the matching kit; review/hash/trust policy is separate from compilation. |

## Offline validation

```sh
python3 tools/sdk/build_package.py mod-sdk/ExampleModernMod --check
python3 tools/sdk/generate_api_reference.py --check
python3 tools/validate_documentation.py
python3 tests/test_sdk_tools.py
ctest --test-dir build -R sdk_examples --output-on-failure
```

The SDK CTest builds/mounts the actual examples, invokes their lifecycle/action/save paths and checks legacy loading/theme application. Configure with BUILD_TESTING=ON and build first. Source validation is not publisher approval or an on-device smoke test.

Before submitting a package, verify fresh start, save/reopen, previous schema fixtures, pause/background/resume/close, keyboard/touch parity, theme/language live update, missing optional audio/haptics, offline behavior, portrait/short landscape, large fonts and actual target devices. Supply icon/license/source metadata, package SHA-256 and a truthful capability/locale list. Theme packages need separate id/version/theme API, data-only resources and fallback/readability checks. Keep production release/platform QA records separate from these example tests.
