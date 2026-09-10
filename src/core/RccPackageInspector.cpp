// SPDX-License-Identifier: GPL-3.0-or-later
#include "RccPackageInspector.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QResource>

namespace {
constexpr qint64 kMaxManifestBytes = 1024 * 1024;

QString probeRootFor(const QString &path)
{
    const QByteArray digest = QCryptographicHash::hash(QFileInfo(path).absoluteFilePath().toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return QStringLiteral("/__lmg_probe_%1").arg(QString::fromLatin1(digest));
}

QString stripProbe(const QString &absoluteResource, const QString &probeRoot)
{
    const QString prefix = QStringLiteral(":") + probeRoot + QLatin1Char('/');
    return absoluteResource.startsWith(prefix) ? absoluteResource.mid(prefix.size()) : QString{};
}

QJsonObject readManifest(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return QJsonObject();
    if (file.size() > kMaxManifestBytes) {
        if (error) *error = QStringLiteral("manifest.json exceeds the 1 MiB safety limit.");
        return QJsonObject();
    }
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error) *error = QStringLiteral("manifest.json is not valid JSON.");
        return QJsonObject();
    }
    return doc.object();
}
}

bool RccPackageInspector::validId(const QString &id)
{
    static const QRegularExpression pattern(QStringLiteral("^[a-z0-9][a-z0-9_.-]{1,63}$"));
    return pattern.match(id).hasMatch();
}

bool RccPackageInspector::safeRelativePath(const QString &path)
{
    if (path.isEmpty() || path.startsWith(QLatin1Char('/')) || path.contains(QLatin1Char('\\')) || path.contains(QStringLiteral(".."))) return false;
    const QString cleaned = QDir::cleanPath(path);
    return cleaned == path && cleaned != QStringLiteral(".") && !cleaned.startsWith(QStringLiteral("../"));
}

RccPackageInspection RccPackageInspector::inspect(const QString &rccFile, const QString &expectedId, const QString &expectedEntry)
{
    RccPackageInspection result;
    const QFileInfo info(rccFile);
    if (!info.isFile()) { result.error = QStringLiteral("RCC package does not exist."); return result; }
    if (!expectedId.isEmpty() && !validId(expectedId)) { result.error = QStringLiteral("Expected package id is invalid."); return result; }
    if (!expectedEntry.isEmpty() && !safeRelativePath(expectedEntry)) { result.error = QStringLiteral("Expected entry path is unsafe."); return result; }

    const QString probeRoot = probeRootFor(rccFile);
    if (!QResource::registerResource(info.absoluteFilePath(), probeRoot)) { result.error = QStringLiteral("Qt rejected the RCC package."); return result; }
    struct Guard { QString file, root; ~Guard() { QResource::unregisterResource(file, root); } } guard{info.absoluteFilePath(), probeRoot};

    const QString probeUrl = QStringLiteral(":") + probeRoot;
    QDirIterator it(probeUrl, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString relative = stripProbe(it.next(), probeRoot);
        if (!relative.isEmpty()) result.resources.append(relative);
        if (result.resources.size() > 10000) { result.error = QStringLiteral("RCC contains too many resources."); return result; }
    }
    if (result.resources.isEmpty()) { result.error = QStringLiteral("RCC package contains no files."); return result; }

    QString id = expectedId;
    QString entry = expectedEntry.isEmpty() ? QStringLiteral("Main.qml") : expectedEntry;
    QString manifestError;
    const QString canonicalManifest = probeUrl + QStringLiteral("/manifest.json");
    QJsonObject manifest;
    bool legacy = false;

    if (QFileInfo::exists(canonicalManifest)) {
        manifest = readManifest(canonicalManifest, &manifestError);
    } else if (!id.isEmpty()) {
        const QString legacyManifest = probeUrl + QStringLiteral("/mods/%1/manifest.json").arg(id);
        if (QFileInfo::exists(legacyManifest)) { legacy = true; manifest = readManifest(legacyManifest, &manifestError); }
    } else {
        QString legacyManifestResource;
        QString legacyManifestId;
        for (const QString &resource : std::as_const(result.resources)) {
            const QRegularExpressionMatch match = QRegularExpression(
                QStringLiteral("^mods/([a-z0-9][a-z0-9_.-]{1,63})/manifest\\.json$")).match(resource);
            if (!match.hasMatch())
                continue;
            if (!legacyManifestResource.isEmpty() && match.captured(1) != legacyManifestId) {
                result.error = QStringLiteral("Legacy RCC is ambiguous: multiple package namespaces contain manifest.json.");
                return result;
            }
            legacyManifestResource = resource;
            legacyManifestId = match.captured(1);
        }
        if (!legacyManifestResource.isEmpty()) {
            id = legacyManifestId;
            legacy = true;
            manifest = readManifest(probeUrl + QLatin1Char('/') + legacyManifestResource, &manifestError);
        }
    }
    if (!manifestError.isEmpty()) { result.error = manifestError; return result; }

    if (!manifest.isEmpty()) {
        const QString manifestId = manifest.value(QStringLiteral("id")).toString();
        if (!manifestId.isEmpty()) {
            if (!validId(manifestId)) { result.error = QStringLiteral("manifest id is invalid."); return result; }
            if (!id.isEmpty() && id != manifestId) { result.error = QStringLiteral("manifest id does not match the expected package id."); return result; }
            id = manifestId;
        }
        const QString manifestEntry = manifest.value(QStringLiteral("entry")).toString();
        if (!manifestEntry.isEmpty()) entry = manifestEntry;
    }
    if (id.isEmpty() || !validId(id)) { result.error = QStringLiteral("Package id could not be established safely."); return result; }
    if (!safeRelativePath(entry)) { result.error = QStringLiteral("Package entry path is unsafe."); return result; }

    const QString canonicalEntry = probeUrl + QLatin1Char('/') + entry;
    const QString legacyEntry = probeUrl + QStringLiteral("/mods/%1/%2").arg(id, entry);
    if (QFileInfo::exists(canonicalEntry)) {
        legacy = false;
    } else if (QFileInfo::exists(legacyEntry)) {
        legacy = true;
    } else {
        result.error = QStringLiteral("Package entry file is missing from the RCC.");
        return result;
    }

    if (legacy) {
        const QString requiredPrefix = QStringLiteral("mods/%1/").arg(id);
        for (const QString &resource : std::as_const(result.resources)) {
            if (!resource.startsWith(requiredPrefix)) {
                result.error = QStringLiteral("Legacy RCC tries to expose resources outside its own /mods/<id> namespace.");
                return result;
            }
        }
        result.mountRoot = QStringLiteral("/");
    } else {
        result.mountRoot = QStringLiteral("/mods/%1").arg(id);
    }

    result.valid = true;
    result.legacyLayout = legacy;
    result.packageId = id;
    result.entryPath = entry;
    result.resourceRoot = QStringLiteral("qrc:/mods/%1/").arg(id);
    result.manifest = manifest;
    return result;
}

bool RccPackageInspector::mount(const QString &rccFile, const RccPackageInspection &inspection)
{
    return inspection.valid && QResource::registerResource(QFileInfo(rccFile).absoluteFilePath(), inspection.mountRoot);
}
void RccPackageInspector::unmount(const QString &rccFile, const RccPackageInspection &inspection)
{
    if (!inspection.mountRoot.isEmpty()) QResource::unregisterResource(QFileInfo(rccFile).absoluteFilePath(), inspection.mountRoot);
}
