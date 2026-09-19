// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "LeoMiniGamesDeveloperPlatform/CatalogCompat.h"
#include <QDateTime>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>
#include <QTimeZone>

namespace LeoMiniGames::DeveloperPlatform {
namespace {
QString stringOr(const QJsonObject &o, const char *key, const QString &fallback = {}) {
    const auto v=o.value(QLatin1String(key)); return v.isString() && !v.toString().trimmed().isEmpty() ? v.toString().trimmed() : fallback;
}
qint64 intOr(const QJsonObject &o, const char *key, qint64 fallback=0) {
    const auto v=o.value(QLatin1String(key)); return v.isDouble() ? static_cast<qint64>(v.toDouble()) : fallback;
}
}
QJsonArray CatalogCompat::extractItems(const QJsonDocument &document) {
    if (document.isArray()) return document.array();
    if (!document.isObject()) return QJsonArray();
    const auto root=document.object();
    for (const char *key: {"data","items","results"}) {
        const auto v=root.value(QLatin1String(key)); if (v.isArray()) return v.toArray();
    }
    const auto data=root.value(QStringLiteral("data"));
    if (data.isObject() && data.toObject().value(QStringLiteral("items")).isArray()) return data.toObject().value(QStringLiteral("items")).toArray();
    return QJsonArray();
}

bool CatalogCompat::isExpiredPresignedUrl(const QString &raw) {
    const QUrl u(raw); if (!u.isValid() || u.scheme()!=QStringLiteral("https")) return true;
    const QUrlQuery q(u); const QString date=q.queryItemValue(QStringLiteral("X-Amz-Date")); const QString exp=q.queryItemValue(QStringLiteral("X-Amz-Expires"));
    if (date.isEmpty() || exp.isEmpty()) return false; // normal/stable HTTPS URL
    auto issued=QDateTime::fromString(date, QStringLiteral("yyyyMMdd'T'HHmmss'Z'"));
    bool ok=false; const qint64 seconds=exp.toLongLong(&ok); if (!issued.isValid() || !ok) return true;
    issued.setTimeZone(QTimeZone::UTC);
    return issued.addSecs(seconds) <= QDateTime::currentDateTimeUtc();
}

QJsonObject CatalogCompat::normalizeItem(const QJsonObject &raw, const QString &kind) {
    QJsonObject out=raw;
    const QString id=stringOr(raw,"id",QStringLiteral("unknown"));
    const QString name=stringOr(raw,"name",id);
    out.insert(QStringLiteral("id"),id);
    out.insert(QStringLiteral("name"),name);
    out.insert(QStringLiteral("description"),stringOr(raw,"description",QStringLiteral("No description available.")));
    out.insert(QStringLiteral("author"),stringOr(raw,"author",QStringLiteral("Unknown publisher")));
    out.insert(QStringLiteral("version"),stringOr(raw,"version",QStringLiteral("0.0.0")));
    out.insert(QStringLiteral("category"),stringOr(raw,"category",kind==QStringLiteral("theme")?QStringLiteral("Theme"):QStringLiteral("Other")));
    out.insert(QStringLiteral("icon"),stringOr(raw,"icon",name.left(3).toUpper()));
    const QString iconUrl=stringOr(raw,"icon_url");
    out.insert(QStringLiteral("icon_url"),(!iconUrl.isEmpty() && !isExpiredPresignedUrl(iconUrl)) ? iconUrl : QString());
    out.insert(QStringLiteral("min_app_version"),stringOr(raw,"min_app_version",QStringLiteral("0.0.0")));
    out.insert(QStringLiteral("package_format"),stringOr(raw,"package_format",QStringLiteral("rcc-v1")));
    out.insert(QStringLiteral("entry"),stringOr(raw,"entry",QStringLiteral("Main.qml")));
    out.insert(QStringLiteral("sha256"),stringOr(raw,"sha256"));
    out.insert(QStringLiteral("download_endpoint"),stringOr(raw,"download_endpoint"));
    out.insert(QStringLiteral("download_url"),stringOr(raw,"download_url"));
    out.insert(QStringLiteral("size_bytes"),intOr(raw,"size_bytes",0));
    out.insert(QStringLiteral("downloads"),intOr(raw,"downloads",0));
    // Explicit defaults prevent QML `undefined` from leaking into double/int bindings.
    if (!out.value(QStringLiteral("rating")).isDouble()) out.insert(QStringLiteral("rating"),0.0);
    if (!out.value(QStringLiteral("featured_score")).isDouble()) out.insert(QStringLiteral("featured_score"),0.0);
    return out;
}
}
