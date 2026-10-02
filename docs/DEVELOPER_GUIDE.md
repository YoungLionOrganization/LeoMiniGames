# Developer guide — 0.7.3

Start with [SDK Quickstart](sdk/QUICKSTART.md) and the [API index](sdk/OVERVIEW.md). ExampleHelloMod preserves the legacy API; ExampleModernMod 1.1.0 targets API 0.7 and application 0.7.3. Theme SDK and native ABI examples are separate projects.

Build game/theme RCCs with `python3 tools/sdk/build_package.py <project> --rcc <Qt6-rcc> --output <output.rcc>`. The scripts work independently of the current directory, validate packaged resources and emit checksums. Build the launcher with BUILD_TESTING=ON to execute real SDK RCC/engine tests.

A development loop is edit → source check → RCC build → Developer Lab verification/import → diagnostics → save/reopen and target-device testing. Developer Lab local RCCs are session-only and network-disabled; normal market installation/publishing is separate. [Troubleshooting](sdk/TROUBLESHOOTING.md) lists acceptance checks and known limits.

Use GameSave for state, GameSettings for preferences, GameAudio for platform playback, GameInput actions for keyboard/touch parity, and GameResources for package URLs. Register schema migrations before load. Serialize once per lifecycle save; do not both implement save() and duplicate it in onSaveRequested. Make theme/language bindings depend on revision/language and gate input/timers while paused.

Supply license/source metadata under the existing [developer licensing policy](DEVELOPER_LICENSING.md), [publisher terms](YOUNGLION_DEVELOPER_PUBLISHER_TERMS_1.0.md) and, for native/L3, [native addendum](NATIVE_L3_PUBLISHER_ADDENDUM_1.0.md). `LicenseRef-YoungLion-Publisher-Package-1.0` terms exist in licenses; the old claim that closed-source terms were only reserved is obsolete. Compilation and manifest trust claims never grant publisher review/native access.
