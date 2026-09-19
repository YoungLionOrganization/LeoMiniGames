// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "PublisherTrustResolver.h"

namespace {
QString normalizedPublisherType(const QString &raw)
{
    const QString type = raw.trimmed().toLower();
    if (type == QStringLiteral("official") || type == QStringLiteral("verified") ||
        type == QStringLiteral("unverified") || type == QStringLiteral("suspended")) {
        return type;
    }
    if (type == QStringLiteral("verified_native") || type == QStringLiteral("verified-native"))
        return QStringLiteral("verified");
    return QString{};
}

bool strictTrue(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    return value.isBool() && value.toBool();
}
}

PublisherTrustDecision resolvePublisherTrust(const QJsonObject &catalogObject,
                                             bool authoritativeCatalog,
                                             bool legacyCatalogIsAdminOnly)
{
    PublisherTrustDecision result;
    if (!authoritativeCatalog)
        return result;

    const QJsonObject publisherObject = catalogObject.value(QStringLiteral("publisher")).toObject();
    const QJsonObject trustObject = catalogObject.value(QStringLiteral("trust")).toObject();

    QString type = normalizedPublisherType(publisherObject.value(QStringLiteral("type")).toString());
    if (type.isEmpty())
        type = normalizedPublisherType(catalogObject.value(QStringLiteral("publisher_type")).toString());

    const bool hasModernTrustShape = !publisherObject.isEmpty()
        || catalogObject.contains(QStringLiteral("publisher_type"))
        || !trustObject.isEmpty();

    // Explicit modern types are authoritative, including explicit unverified/suspended.
    if (type.isEmpty() && hasModernTrustShape) {
        if (strictTrue(publisherObject, QStringLiteral("official")) ||
            strictTrue(catalogObject, QStringLiteral("official"))) {
            type = QStringLiteral("official");
        } else if (strictTrue(publisherObject, QStringLiteral("verified")) ||
                   strictTrue(catalogObject, QStringLiteral("publisher_verified")) ||
                   strictTrue(catalogObject, QStringLiteral("verified"))) {
            type = QStringLiteral("verified");
        } else {
            type = QStringLiteral("unverified");
        }
    }

    // Legacy compatibility: current historical public repositories are writable only
    // through authenticated admin endpoints. Their lack of modern trust fields means
    // official admin provenance, not an unverified community publisher.
    if (type.isEmpty() && legacyCatalogIsAdminOnly) {
        type = QStringLiteral("official");
        result.legacyAdminFallback = true;
    }

    if (type.isEmpty())
        type = QStringLiteral("unverified");

    result.type = type;
    result.official = type == QStringLiteral("official");
    result.verified = result.official || type == QStringLiteral("verified");
    return result;
}
