#include "core/PublisherTrustResolver.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>

static int fail(const char *message) { qCritical() << message; return 1; }

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QJsonObject legacyAdminMod{
        {QStringLiteral("id"), QStringLiteral("legacy_admin_mod")},
        {QStringLiteral("author"), QStringLiteral("YoungLion")}
    };
    auto legacyMod = resolvePublisherTrust(legacyAdminMod, true, true);
    if (!legacyMod.official || !legacyMod.verified || legacyMod.type != QStringLiteral("official") || !legacyMod.legacyAdminFallback)
        return fail("legacy admin mod did not retain Official Publisher provenance");

    QJsonObject legacyAdminTheme{
        {QStringLiteral("id"), QStringLiteral("legacy_admin_theme")},
        {QStringLiteral("publisher"), QStringLiteral("YoungLion")},
        {QStringLiteral("publisher_status"), QStringLiteral("Unverified Publisher")},
        {QStringLiteral("publisher_verified"), false}
    };
    auto legacyTheme = resolvePublisherTrust(legacyAdminTheme, true, true);
    if (!legacyTheme.official || !legacyTheme.legacyAdminFallback)
        return fail("legacy admin theme did not retain Official Publisher provenance");

    auto thirdParty = resolvePublisherTrust(legacyAdminMod, false, true);
    if (thirdParty.verified || thirdParty.official || thirdParty.type != QStringLiteral("unverified"))
        return fail("third-party legacy catalog elevated to Official Publisher");

    QJsonObject modernUnverified{
        {QStringLiteral("publisher"), QJsonObject{
            {QStringLiteral("display_name"), QStringLiteral("Community Dev")},
            {QStringLiteral("type"), QStringLiteral("unverified")},
            {QStringLiteral("verified"), false},
            {QStringLiteral("official"), false}
        }},
        {QStringLiteral("trust"), QJsonObject{{QStringLiteral("plugin_level"), 1}}}
    };
    auto unverified = resolvePublisherTrust(modernUnverified, true, true);
    if (unverified.verified || unverified.official || unverified.legacyAdminFallback)
        return fail("modern unverified publisher was promoted by legacy fallback");

    QJsonObject modernVerified{
        {QStringLiteral("publisher"), QJsonObject{
            {QStringLiteral("display_name"), QStringLiteral("Verified Dev")},
            {QStringLiteral("type"), QStringLiteral("verified")},
            {QStringLiteral("verified"), true}
        }}
    };
    auto verified = resolvePublisherTrust(modernVerified, true, true);
    if (!verified.verified || verified.official || verified.type != QStringLiteral("verified"))
        return fail("modern verified publisher parse failed");

    QJsonObject officialScalar{
        {QStringLiteral("publisher_type"), QStringLiteral("official")},
        {QStringLiteral("publisher_verified"), true},
        {QStringLiteral("official"), true}
    };
    auto official = resolvePublisherTrust(officialScalar, true, true);
    if (!official.official || !official.verified || official.legacyAdminFallback)
        return fail("server-owned scalar Official provenance parse failed");

    qInfo() << "Publisher trust resolver tests PASS";
    return 0;
}
