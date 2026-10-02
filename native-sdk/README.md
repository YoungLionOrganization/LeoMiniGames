# Native SDK — ABI 1.0

Native C++ plugins use [IGamePlugin.h](../src/sdk/IGamePlugin.h), separately from RCC/QML host API 0.7. `info()` returns identity/API/entry URL; `createGame(parent)` returns a parent-owned QObject exposed as `App.currentGame` to the native game's QML. The IID is `xyz.younglion.leominigames.IGamePlugin/1.0` and `GamePluginInfo.apiVersion` is 1. Do not derive external plugins from private host `BuiltinGame` or link `lmg_core` as a public SDK.

`ExampleNativeGame` is an independently buildable compile example, not a pre-approved deployable game. Use the Qt/compiler/architecture kit matching the launcher:

```sh
qt-cmake -S native-sdk/ExampleNativeGame -B build-native-example -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-native-example
```

Desktop builds with BUILD_TESTING=ON also compile this example in CI, without loading or granting trust.

This example targets launcher 0.7.3 (including the additive GameSave.hasStoredSlot helper). It demonstrates a QObject model, Qt metadata, packaged QML and root-level persistence/lifecycle callbacks. It does not provide a publisher trust snapshot, production signature, automatic installer or dynamic mobile deployment. See [Native API](../docs/sdk/NATIVE_API.md) and [Trusted native policy](../docs/TRUSTED_NATIVE.md) before distribution.

Licensing is separate from execution permission. Only files explicitly marked with the SDK license receive that grant; the example's unmarked new source follows the repository's [licensing policy](../LICENSING.md). Choose and supply an accepted license for your independent package. Copying an example never grants Official/Verified/Native-L3 status.
