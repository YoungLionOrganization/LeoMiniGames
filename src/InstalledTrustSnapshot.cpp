// SPDX-License-Identifier: GPL-3.0-or-later
#include "LeoMiniGamesDeveloperPlatform/InstalledTrustSnapshot.h"

#include <QJsonArray>

namespace LeoMiniGames::DeveloperPlatform {

QJsonObject InstalledTrustSnapshot::serialize(const CatalogTrustInfo &info)
{
    QJsonObject publisher{
        {QStringLiteral("id"), info.publisher.id},
        {QStringLiteral("display_name"), info.publisher.displayName},
        {QStringLiteral("type"), info.publisher.type},
        {QStringLiteral("verified"), info.publisher.verified},
        {QStringLiteral("official"), info.publisher.official},
    };
    QJsonObject trust{
        {QStringLiteral("plugin_level"), info.pluginLevel},
        {QStringLiteral("reviewed"), info.reviewed},
        {QStringLiteral("native_allowed"), info.nativeAllowed},
    };
    QJsonObject license{
        {QStringLiteral("id"), info.licenseId},
        {QStringLiteral("source_url"), info.sourceUrl},
    };
    QJsonArray native;
    for (const auto &a : info.nativeArtifacts) {
        native.append(QJsonObject{
            {QStringLiteral("platform"), a.platform},
            {QStringLiteral("architecture"), a.architecture},
            {QStringLiteral("qt_version"), a.qtVersion},
            {QStringLiteral("abi"), a.abi},
            {QStringLiteral("api_level"), a.apiLevel},
            {QStringLiteral("sha256"), a.sha256},
            {QStringLiteral("size_bytes"), static_cast<double>(a.sizeBytes)},
            {QStringLiteral("review_state"), a.reviewState},
        });
    }
    return QJsonObject{
        {QStringLiteral("schema"), 1},
        {QStringLiteral("publisher"), publisher},
        {QStringLiteral("trust"), trust},
        {QStringLiteral("license"), license},
        {QStringLiteral("native_artifacts"), native},
    };
}

CatalogTrustInfo InstalledTrustSnapshot::deserialize(const QJsonObject &snapshot)
{
    if (snapshot.value(QStringLiteral("schema")).toInt(0) != 1) return CatalogTrustInfo();
    return CatalogTrustInfo::fromCatalogObject(snapshot);
}

} // namespace LeoMiniGames::DeveloperPlatform
