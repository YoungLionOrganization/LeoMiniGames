#include "LeoMiniGamesDeveloperPlatform/CatalogTrustInfo.h"
#include "LeoMiniGamesDeveloperPlatform/InstalledTrustSnapshot.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>

using namespace LeoMiniGames::DeveloperPlatform;

static int fail(const char *msg) { qCritical() << msg; return 1; }

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    // Legacy response: no additive trust metadata => safe unverified Level 1 fallback.
    QJsonObject legacy{{QStringLiteral("id"), QStringLiteral("old_mod")},
                       {QStringLiteral("verified"), true}, // manifest/top-level spoof must not grant trust
                       {QStringLiteral("official"), true}};
    auto a = CatalogTrustInfo::fromCatalogObject(legacy);
    if (a.isVerified() || a.isOfficial() || a.pluginLevel != 1 || a.nativeAllowed) return fail("legacy spoof elevated trust");

    QJsonObject modern{
        {QStringLiteral("publisher"), QJsonObject{
            {QStringLiteral("id"), QStringLiteral("pub-1")},
            {QStringLiteral("display_name"), QStringLiteral("YoungLion")},
            {QStringLiteral("type"), QStringLiteral("official")},
            {QStringLiteral("verified"), true},
            {QStringLiteral("official"), true}}},
        {QStringLiteral("trust"), QJsonObject{
            {QStringLiteral("plugin_level"), 3},
            {QStringLiteral("reviewed"), true},
            {QStringLiteral("native_allowed"), true}}},
        {QStringLiteral("license"), QJsonObject{
            {QStringLiteral("id"), QStringLiteral("MIT")},
            {QStringLiteral("source_url"), QStringLiteral("https://example.invalid/source")}}}
    };
    auto b = CatalogTrustInfo::fromCatalogObject(modern);
    if (!b.isOfficial() || !b.isVerified() || b.pluginLevel != 3 || !b.nativeAllowed) return fail("official trust parse failed");

    auto roundtrip = InstalledTrustSnapshot::deserialize(InstalledTrustSnapshot::serialize(b));
    if (!roundtrip.isOfficial() || roundtrip.licenseId != QStringLiteral("MIT")) return fail("trust snapshot roundtrip failed");

    QJsonObject suspended = modern;
    QJsonObject p = suspended.value(QStringLiteral("publisher")).toObject();
    p.insert(QStringLiteral("type"), QStringLiteral("suspended"));
    suspended.insert(QStringLiteral("publisher"), p);
    auto c = CatalogTrustInfo::fromCatalogObject(suspended);
    if (c.isVerified() || c.isOfficial() || c.nativeAllowed) return fail("suspended publisher retained trust");

    qInfo() << "Catalog trust compatibility tests PASS";
    return 0;
}
