#pragma once

#include "CatalogTrustInfo.h"
#include <QJsonObject>

namespace LeoMiniGames::DeveloperPlatform {

class InstalledTrustSnapshot
{
public:
    static QJsonObject serialize(const CatalogTrustInfo &info);
    static CatalogTrustInfo deserialize(const QJsonObject &snapshot);
};

} // namespace LeoMiniGames::DeveloperPlatform
