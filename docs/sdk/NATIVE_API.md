# Native plugin SDK

Native C++ ABI 1.0 is distinct from QML host API 0.7 and theme API 1. The public interface is [src/sdk/IGamePlugin.h](../../src/sdk/IGamePlugin.h): implement info() and createGame(QObject *parent), Q_INTERFACES(IGamePlugin), and Q_PLUGIN_METADATA with IID xyz.younglion.leominigames.IGamePlugin/1.0. GamePluginInfo.apiVersion is 1. Return a parent-owned QObject and an entryUrl for the packaged QML.

An independently buildable example is [native-sdk/ExampleNativeGame](../../native-sdk/README.md). Use the same Qt major/minor build, compiler ABI/runtime and CPU architecture as the launcher; a native DLL/.so/.dylib is not a portable RCC. The host-created model is App.currentGame for a trusted native session; QML root hooks can use GameSave and lifecycle services. Do not depend on private BuiltinGame/BuiltinState/lmg_core implementation classes as an external SDK contract.

Native loading is desktop-only under the reviewed trusted-native snapshot/hash/Qt/API gates documented in [TRUSTED_NATIVE](../TRUSTED_NATIVE.md). Builtins are linked into the application; this does not authorize arbitrary mobile native plugin loading. Copying a library or claiming plugin_level:3 does not grant review/verification. No deploy/sign/trust automation is supplied by the example.

The include/LeoMiniGamesDeveloperPlatform helpers cover catalog compatibility/trust snapshots, not an in-process native game ABI replacement. Follow existing license/publisher/native addendum requirements separately; no new legal grant is implied by these docs.
