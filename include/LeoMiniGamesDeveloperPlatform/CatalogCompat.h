#pragma once
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace LeoMiniGames::DeveloperPlatform {
class CatalogCompat final {
public:
    static QJsonArray extractItems(const QJsonDocument &document);
    static QJsonObject normalizeItem(const QJsonObject &raw, const QString &kind = QStringLiteral("mod"));
    static bool isExpiredPresignedUrl(const QString &url);
};
}
