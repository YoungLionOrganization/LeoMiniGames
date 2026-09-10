// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QJsonObject>
#include <QString>

struct PublisherTrustDecision
{
    QString type = QStringLiteral("unverified"); // official, verified, unverified, suspended
    bool verified = false;
    bool official = false;
    bool legacyAdminFallback = false;
};

// Resolve publisher trust from a catalog row.
//
// Security contract:
//  * Non-canonical/third-party catalogs never grant YoungLion trust.
//  * Modern explicit server trust (publisher object / publisher_type / trust object)
//    always wins over legacy inference.
//  * The historical /api/v1/mods and /api/v1/themes repositories are admin-key-only.
//    When those canonical legacy rows contain no modern trust shape, they are treated
//    as Official Publisher for backward compatibility with pre-v0.7 admin publishing.
PublisherTrustDecision resolvePublisherTrust(const QJsonObject &catalogObject,
                                             bool authoritativeCatalog,
                                             bool legacyCatalogIsAdminOnly = true);
