// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameRuntime.h"
#include <QSet>
#include <QVersionNumber>

GameRuntime::GameRuntime(QObject *parent) : QObject(parent) {}
QStringList GameRuntime::capabilities() const
{
    return {QStringLiteral("theme"), QStringLiteral("i18n"), QStringLiteral("save.atomic"),
            QStringLiteral("audio.capability_probe"), QStringLiteral("input.physical_wasd"),
            QStringLiteral("lifecycle"), QStringLiteral("random"), QStringLiteral("settings"),
            QStringLiteral("stats"), QStringLiteral("achievements"), QStringLiteral("events"),
            QStringLiteral("resources"), QStringLiteral("network.https"),
            QStringLiteral("legacy_rcc_mount"), QStringLiteral("developer_diagnostics")};
}
bool GameRuntime::supports(const QString &capability) const { return capabilities().contains(capability.trimmed()); }
QVariantMap GameRuntime::checkCompatibility(const QString &api, const QString &minApi, const QStringList &required) const
{
    const QVersionNumber host = QVersionNumber::fromString(apiVersion());
    const QVersionNumber requested = QVersionNumber::fromString(api.trimmed().isEmpty() ? QStringLiteral("0.6") : api);
    const QVersionNumber minimum = QVersionNumber::fromString(minApi.trimmed().isEmpty() ? QStringLiteral("0.5") : minApi);
    QStringList missing;
    for (const QString &cap : required) if (!supports(cap)) missing.append(cap);
    const bool versionOk = !requested.isNull() && !minimum.isNull()
        && QVersionNumber::compare(minimum, host) <= 0
        && QVersionNumber::compare(requested, QVersionNumber(0, 7)) <= 0;
    QVariantMap result;
    result.insert(QStringLiteral("ok"), versionOk && missing.isEmpty());
    result.insert(QStringLiteral("legacy"), api.trimmed().isEmpty() || QVersionNumber::compare(requested, QVersionNumber(0, 7)) < 0);
    result.insert(QStringLiteral("hostApiVersion"), apiVersion());
    result.insert(QStringLiteral("missingCapabilities"), missing);
    if (!versionOk) result.insert(QStringLiteral("error"), QStringLiteral("Unsupported API version. Host supports legacy 0.5/0.6 and API 0.7."));
    else if (!missing.isEmpty()) result.insert(QStringLiteral("error"), QStringLiteral("Missing required capabilities: %1").arg(missing.join(QStringLiteral(", "))));
    return result;
}
