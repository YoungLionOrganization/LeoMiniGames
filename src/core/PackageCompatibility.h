// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include "GameRuntime.h"
#include "SemVer.h"
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>

namespace PackageCompatibility {
inline QString error(const QJsonObject &manifest) {
    QStringList required;
    const QJsonValue requirements = manifest.value(QStringLiteral("required_capabilities"));
    if (!requirements.isUndefined() && !requirements.isArray()) return QStringLiteral("Required capabilities must be an array.");
    for (const QJsonValue &value : requirements.toArray()) {
        if (!value.isString() || value.toString().isEmpty()) return QStringLiteral("Required capability must be a nonempty string.");
        required.append(value.toString());
    }
    GameRuntime runtime;
    const auto compatibility = runtime.checkCompatibility(manifest.value(QStringLiteral("api_version")).toString(), manifest.value(QStringLiteral("min_api_version")).toString(), required);
    if (!compatibility.value(QStringLiteral("ok")).toBool()) return compatibility.value(QStringLiteral("error")).toString();
    const QString minimum = manifest.value(QStringLiteral("min_app_version")).toString();
    if (!minimum.isEmpty()) {
        const auto requested = LmgSemVer::parseSemVer(minimum);
        const auto current = LmgSemVer::parseSemVer(QCoreApplication::applicationVersion());
        if (!requested.valid || !current.valid || LmgSemVer::compareSemVer(current, requested) < 0) return QStringLiteral("Package requires a newer or unsupported application version.");
    }
    const QJsonValue schema = manifest.value(QStringLiteral("save_version"));
    if (!schema.isUndefined() && (!schema.isDouble() || schema.toInt(-1) < 1 || schema.toDouble() != schema.toInt())) return QStringLiteral("Invalid package save schema.");
    return QString{};
}
}
