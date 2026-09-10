// SPDX-License-Identifier: GPL-3.0-or-later
#include "LeoMiniGamesDeveloperPlatform/CatalogTrustInfo.h"

#include <QJsonArray>
#include <QJsonValue>

namespace LeoMiniGames::DeveloperPlatform {
namespace {
QString boundedString(const QJsonObject &o, const char *key, int maxLen = 2048)
{
    const auto v = o.value(QLatin1StringView(key));
    if (!v.isString()) return QString();
    const QString s = v.toString().trimmed();
    return s.size() <= maxLen ? s : s.left(maxLen);
}

bool strictBool(const QJsonObject &o, const char *key)
{
    const auto v = o.value(QLatin1StringView(key));
    return v.isBool() && v.toBool();
}
}

CatalogTrustInfo CatalogTrustInfo::fromCatalogObject(const QJsonObject &catalogObject)
{
    CatalogTrustInfo out;

    // Backwards compatibility: absence of the additive server trust fields means
    // legacy/unverified. Never infer trust from manifest fields.
    const QJsonObject publisher = catalogObject.value(QStringLiteral("publisher")).toObject();
    if (!publisher.isEmpty()) {
        out.publisher.id = boundedString(publisher, "id", 128);
        out.publisher.displayName = boundedString(publisher, "display_name", 160);
        const QString type = boundedString(publisher, "type", 32).toLower();
        if (type == QStringLiteral("official") || type == QStringLiteral("verified") ||
            type == QStringLiteral("unverified") || type == QStringLiteral("suspended")) {
            out.publisher.type = type;
        } else {
            out.publisher.type = QStringLiteral("unverified");
        }
        out.publisher.official = strictBool(publisher, "official") && out.publisher.type == QStringLiteral("official");
        out.publisher.verified = strictBool(publisher, "verified") &&
                                 (out.publisher.type == QStringLiteral("verified") || out.publisher.type == QStringLiteral("official"));
    } else {
        out.publisher.type = QStringLiteral("unverified");
    }

    const QJsonObject trust = catalogObject.value(QStringLiteral("trust")).toObject();
    if (!trust.isEmpty()) {
        const int level = trust.value(QStringLiteral("plugin_level")).toInt(1);
        out.pluginLevel = qBound(1, level, 3);
        out.reviewed = strictBool(trust, "reviewed");
        out.nativeAllowed = strictBool(trust, "native_allowed") && out.pluginLevel == 3;
    }

    const QJsonObject license = catalogObject.value(QStringLiteral("license")).toObject();
    if (!license.isEmpty()) {
        out.licenseId = boundedString(license, "id", 120);
        out.sourceUrl = boundedString(license, "source_url", 2048);
    } else {
        // Additive compatibility for deployments that expose scalar fields first.
        out.licenseId = boundedString(catalogObject, "license", 120);
        out.sourceUrl = boundedString(catalogObject, "source_url", 2048);
    }

    const QJsonArray native = catalogObject.value(QStringLiteral("native_artifacts")).toArray();
    out.nativeArtifacts.reserve(native.size());
    for (const auto &value : native) {
        if (!value.isObject()) continue;
        const auto obj = value.toObject();
        NativeArtifactTrust a;
        a.platform = boundedString(obj, "platform", 32).toLower();
        a.architecture = boundedString(obj, "architecture", 64).toLower();
        a.qtVersion = boundedString(obj, "qt_version", 64);
        a.abi = boundedString(obj, "abi", 128);
        a.apiLevel = boundedString(obj, "api_level", 64);
        a.sha256 = boundedString(obj, "sha256", 64).toLower();
        a.sizeBytes = static_cast<qint64>(obj.value(QStringLiteral("size_bytes")).toDouble(0));
        a.reviewState = boundedString(obj, "review_state", 32).toLower();
        if (a.platform.isEmpty() || a.architecture.isEmpty() || a.sha256.size() != 64 || a.sizeBytes <= 0) continue;
        out.nativeArtifacts.push_back(a);
    }

    // Suspended publishers are never trusted, regardless of stale cached booleans.
    if (out.publisher.type == QStringLiteral("suspended")) {
        out.publisher.official = false;
        out.publisher.verified = false;
        out.nativeAllowed = false;
    }

    return out;
}

} // namespace LeoMiniGames::DeveloperPlatform
