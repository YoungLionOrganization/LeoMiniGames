#include "ExampleNativePlugin.h"
GamePluginInfo ExampleNativePlugin::info() const {
    return {QStringLiteral("example_native"),QStringLiteral("Native SDK example"),
        QStringLiteral("Compile-only native ABI example; deployment requires host review/trust."),
        QStringLiteral("Example"),QStringLiteral("1.0.0"),QStringLiteral("Example publisher"),
        QStringLiteral("N"),QUrl(QStringLiteral("qrc:/native/example_native/Main.qml")),1};
}
QObject *ExampleNativePlugin::createGame(QObject *parent) { return new ExampleNativeState(parent); }
