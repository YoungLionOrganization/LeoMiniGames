#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace LeoMiniGames::DeveloperPlatform {

struct PublisherTrust {
    QString id;
    QString displayName;
    QString type;              // unverified, verified, official, suspended
    bool verified = false;
    bool official = false;
};

struct NativeArtifactTrust {
    QString platform;
    QString architecture;
    QString qtVersion;
    QString abi;
    QString apiLevel;
    QString sha256;
    qint64 sizeBytes = 0;
    QString reviewState;
};

struct CatalogTrustInfo {
    PublisherTrust publisher;
    int pluginLevel = 1;
    bool reviewed = false;
    bool nativeAllowed = false;
    QString licenseId;
    QString sourceUrl;
    QList<NativeArtifactTrust> nativeArtifacts;

    bool isOfficial() const noexcept { return publisher.official && publisher.type == QStringLiteral("official"); }
    bool isVerified() const noexcept { return publisher.verified || isOfficial(); }

    static CatalogTrustInfo fromCatalogObject(const QJsonObject &catalogObject);
};

} // namespace LeoMiniGames::DeveloperPlatform
