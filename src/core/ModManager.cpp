// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "ModManager.h"
#include "AppPaths.h"
#include "SettingsManager.h"
#include "RccPackageInspector.h"
#include "GameRuntime.h"
#include "PublisherTrustResolver.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QResource>
#include <QSaveFile>
#include <QSslSocket>
#include <QVersionNumber>
#include <QtGlobal>

#include <memory>
#include <utility>

namespace {
constexpr qint64 kMaxModBytes = 64LL * 1024LL * 1024LL;

QString normalizedBaseUrl(const QString &raw)
{
    QString result = raw.trimmed();
    while (result.endsWith(QLatin1Char('/')))
        result.chop(1);
    return result;
}

QUrl resolveEndpoint(const QString &base, const QString &endpoint)
{
    const QUrl endpointUrl(endpoint);
    if (endpointUrl.isValid() && !endpointUrl.isRelative() && !endpointUrl.scheme().isEmpty())
        return endpointUrl;

    QUrl root(base);
    if (!root.isValid() || root.scheme().isEmpty() || root.host().isEmpty())
        return QUrl{};

    if (endpoint.startsWith(QLatin1Char('/'))) {
        root.setPath(endpoint);
        root.setQuery(QString{});
        root.setFragment(QString{});
        return root;
    }

    if (!root.path().endsWith(QLatin1Char('/'))) {
        QString path = root.path();
        path += QLatin1Char('/');
        root.setPath(path);
    }
    return root.resolved(endpointUrl);
}

bool isOfficialCatalogOrigin(const QString &base)
{
    const QUrl url(base);
    if (!url.isValid() || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0) return false;
    if (url.host().compare(QStringLiteral("leominigames.younglion.xyz"), Qt::CaseInsensitive) != 0) return false;
    if (url.port(-1) != -1 && url.port(-1) != 443) return false;
    const QString path = QDir::cleanPath(url.path());
    return path == QStringLiteral("/api/v1") || path.startsWith(QStringLiteral("/api/v1/"));
}

constexpr qint64 kMaxJsonResponseBytes = 2LL * 1024LL * 1024LL;
}

struct ModManager::Impl
{
    explicit Impl(QObject *owner) : network(owner) {}

    struct DownloadJob {
        int row = -1;
        QString id;
        QString finalPath;
        QString partPath;
        QString expectedSha;
        qint64 expectedSize = 0;
        qint64 received = 0;
        QFile file;
        QCryptographicHash hash{QCryptographicHash::Sha256};
    };

    QNetworkAccessManager network;
    QHash<QNetworkReply *, std::shared_ptr<DownloadJob>> downloads;
};

ModManager::ModManager(AppPaths *paths, SettingsManager *settings, QObject *parent)
    : QAbstractListModel(parent),
      m_paths(paths),
      m_settings(settings),
      m_impl(std::make_unique<Impl>(this))
{
    loadInstalled();
}

ModManager::~ModManager() = default;

int ModManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant ModManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return QVariant{};

    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case IdRole: return entry.id;
    case NameRole: return entry.name;
    case LocalizedNameRole: return entry.localizedName;
    case DescriptionRole: return entry.description;
    case AuthorRole: return entry.author;
    case VersionRole: return entry.version;
    case CategoryRole: return entry.category;
    case IconRole: return entry.icon;
    case IconUrlRole: return entry.iconUrl;
    case SizeRole: return entry.sizeBytes;
    case DownloadsRole: return entry.downloads;
    case InstalledRole: return entry.installed;
    case InstalledVersionRole: return entry.installedVersion;
    case StateRole: return entry.state;
    case ProgressRole: return entry.progress;
    case ErrorRole: return entry.operationError;
    case MinAppVersionRole: return entry.minAppVersion;
    case TagsRole: return entry.tags;
    case PublisherStatusRole: return entry.publisherStatus;
    case PublisherVerifiedRole: return entry.publisherVerified;
    case PublisherVerificationRole: return entry.publisherVerification;
    case LicenseRole: return entry.license;
    case LicenseFileRole: return entry.licenseFile;
    case SourceUrlRole: return entry.sourceUrl;
    case SourceAvailableRole: return entry.sourceAvailable;
    case PluginLevelRole: return entry.pluginLevel;
    case NativeRole: return entry.native;
    case ReviewedRole: return entry.reviewed;
    case SaveVersionRole: return entry.saveVersion;
    case DefaultLocaleRole: return entry.defaultLocale;
    case LocalesRole: return entry.locales;
    case SettingsSchemaRole: return entry.settingsSchema;
    case UpdatedAtRole: return entry.updatedAt;
    case InCatalogRole: return entry.inCatalog;
    default: return QVariant{};
    }
}

QHash<int, QByteArray> ModManager::roleNames() const
{
    return {
        {IdRole, "modId"}, {NameRole, "modName"}, {LocalizedNameRole, "modLocalizedName"},
        {DescriptionRole, "modDescription"}, {AuthorRole, "modAuthor"}, {VersionRole, "modVersion"},
        {CategoryRole, "modCategory"}, {IconRole, "modIcon"}, {IconUrlRole, "modIconUrl"},
        {SizeRole, "modSizeBytes"}, {DownloadsRole, "modDownloads"}, {InstalledRole, "modInstalled"},
        {InstalledVersionRole, "modInstalledVersion"}, {StateRole, "modState"}, {ProgressRole, "modProgress"},
        {ErrorRole, "modError"}, {MinAppVersionRole, "modMinAppVersion"}, {TagsRole, "modTags"},
        {PublisherStatusRole, "modPublisherStatus"}, {PublisherVerifiedRole, "modPublisherVerified"},
        {PublisherVerificationRole, "modPublisherVerification"},
        {LicenseRole, "modLicense"}, {LicenseFileRole, "modLicenseFile"}, {SourceUrlRole, "modSourceUrl"},
        {SourceAvailableRole, "modSourceAvailable"}, {PluginLevelRole, "modPluginLevel"}, {NativeRole, "modNative"},
        {ReviewedRole, "modReviewed"}, {SaveVersionRole, "modSaveVersion"}, {DefaultLocaleRole, "modDefaultLocale"},
        {LocalesRole, "modLocales"}, {SettingsSchemaRole, "modSettingsSchema"}, {UpdatedAtRole, "modUpdatedAt"},
        {InCatalogRole, "modInCatalog"}
    };
}

bool ModManager::loading() const { return m_loading; }
QString ModManager::error() const { return m_error; }
int ModManager::count() const { return rowCount(); }
bool ModManager::tlsAvailable() const { return QSslSocket::supportsSsl(); }
QString ModManager::tlsBackend() const { return QSslSocket::activeBackend(); }

QString ModManager::apiBaseUrl() const
{
    if (!m_settings)
        return QStringLiteral("https://leominigames.younglion.xyz/api/v1");
    return normalizedBaseUrl(
        m_settings->value(QStringLiteral("mods/apiBaseUrl"),
                          QStringLiteral("https://leominigames.younglion.xyz/api/v1")).toString());
}

void ModManager::setError(const QString &message)
{
    if (m_error == message)
        return;
    m_error = message;
    emit errorChanged();
}

void ModManager::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    emit loadingChanged();
}

int ModManager::indexOf(const QString &id) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).id == id)
            return i;
    }
    return -1;
}

bool ModManager::validId(const QString &id)
{
    static const QRegularExpression pattern(QStringLiteral("^[a-z0-9][a-z0-9_.-]{1,63}$"));
    return pattern.match(id).hasMatch();
}

bool ModManager::safeEntryPath(const QString &path)
{
    if (path.isEmpty() || path.startsWith(QLatin1Char('/')) || path.contains(QStringLiteral("..")))
        return false;
    return !QDir::cleanPath(path).startsWith(QStringLiteral("../"));
}

bool ModManager::versionAtLeast(const QString &current, const QString &minimum)
{
    if (minimum.trimmed().isEmpty())
        return true;

    const QVersionNumber currentVersion = QVersionNumber::fromString(current);
    const QVersionNumber minimumVersion = QVersionNumber::fromString(minimum);
    if (currentVersion.isNull() || minimumVersion.isNull())
        return true;
    return QVersionNumber::compare(currentVersion, minimumVersion) >= 0;
}

void ModManager::updateRow(int row, const QList<int> &roles)
{
    if (row < 0 || row >= m_entries.size())
        return;
    const QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, roles);
}

void ModManager::failEntry(int row, const QString &message)
{
    if (row < 0 || row >= m_entries.size())
        return;
    Entry &entry = m_entries[row];
    entry.state = QStringLiteral("error");
    entry.operationError = message;
    entry.progress = 0.0;
    updateRow(row, {StateRole, ErrorRole, ProgressRole});
}

void ModManager::refresh()
{
    // Keep model rows stable while a package download/install callback still references them.
    if (m_loading || (m_impl && !m_impl->downloads.isEmpty()))
        return;
    for (const Entry &entry : std::as_const(m_entries)) {
        if (entry.state == QStringLiteral("resolving") ||
            entry.state == QStringLiteral("downloading") ||
            entry.state == QStringLiteral("installing"))
            return;
    }

    setError(QString{});
    setLoading(true);

    QUrl url(apiBaseUrl() + QStringLiteral("/mods?limit=100"));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setTransferTimeout(15000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("LeoMiniGames/%1").arg(QCoreApplication::applicationVersion()));

    QNetworkReply *reply = m_impl->network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)> guard(
            reply, [](QNetworkReply *r) { r->deleteLater(); });
        setLoading(false);

        if (reply->error() != QNetworkReply::NoError) {
            const QString detail = reply->errorString();
            if (!QSslSocket::supportsSsl() || detail.contains(QStringLiteral("TLS initialization"), Qt::CaseInsensitive)) {
                const QString backends = QSslSocket::availableBackends().join(QStringLiteral(", "));
                setError(QStringLiteral("Secure connection is unavailable on this device. TLS backend: %1; available: %2")
                             .arg(QSslSocket::activeBackend().isEmpty() ? QStringLiteral("none") : QSslSocket::activeBackend(),
                                  backends.isEmpty() ? QStringLiteral("none") : backends));
            } else {
                setError(QStringLiteral("Catalog request failed: %1").arg(detail));
            }
            return;
        }

        const QByteArray catalogPayload = reply->readAll();
        if (catalogPayload.size() > kMaxJsonResponseBytes) { setError(QStringLiteral("Catalog response exceeds the 2 MiB safety limit.")); return; }
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(catalogPayload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            setError(QStringLiteral("Catalog returned invalid JSON."));
            return;
        }

        const QJsonArray dataArray = document.object().value(QStringLiteral("data")).toArray();
        QHash<QString, Entry> installed;
        for (const Entry &entry : std::as_const(m_entries)) {
            if (entry.installed)
                installed.insert(entry.id, entry);
        }

        QList<Entry> fresh;
        fresh.reserve(dataArray.size() + installed.size());

        for (const QJsonValue &value : dataArray) {
            const QJsonObject object = value.toObject();
            Entry entry;
            entry.id = object.value(QStringLiteral("id")).toString();
            if (!validId(entry.id))
                continue;

            entry.inCatalog = true;
            entry.name = object.value(QStringLiteral("name")).toString(entry.id);
            entry.localizedName = object.value(QStringLiteral("localized_name")).toString(entry.name);
            entry.description = object.value(QStringLiteral("description")).toString();
            const bool authoritativeCatalog = isOfficialCatalogOrigin(apiBaseUrl());
            const QJsonObject publisherObject = object.value(QStringLiteral("publisher")).toObject();
            entry.author = !publisherObject.isEmpty()
                ? publisherObject.value(QStringLiteral("display_name")).toString(object.value(QStringLiteral("author")).toString(QStringLiteral("Unknown")))
                : object.value(QStringLiteral("publisher")).toString(object.value(QStringLiteral("author")).toString(QStringLiteral("Unknown")));
            entry.version = object.value(QStringLiteral("version")).toString();
            entry.category = object.value(QStringLiteral("category")).toString(QStringLiteral("Mod"));
            entry.icon = object.value(QStringLiteral("icon")).toString(QStringLiteral("MOD"));
            entry.iconUrl = object.value(QStringLiteral("icon_url")).toString();
            entry.sha256 = object.value(QStringLiteral("sha256")).toString().toLower();
            entry.packageFormat = object.value(QStringLiteral("package_format")).toString(QStringLiteral("rcc-v1"));
            entry.entryPath = object.value(QStringLiteral("entry")).toString(QStringLiteral("Main.qml"));
            entry.minAppVersion = object.value(QStringLiteral("min_app_version")).toString();
            entry.downloadEndpoint = object.value(QStringLiteral("download_endpoint")).toString();
            entry.downloadUrl = object.value(QStringLiteral("download_url")).toString();
            entry.updatedAt = object.value(QStringLiteral("updated_at")).toString();
            entry.sizeBytes = object.value(QStringLiteral("size_bytes")).toVariant().toLongLong();
            entry.downloads = object.value(QStringLiteral("downloads")).toVariant().toLongLong();
            for (const QJsonValue &tag : object.value(QStringLiteral("tags")).toArray())
                entry.tags.append(tag.toString());
            entry.apiVersion = object.value(QStringLiteral("api_version")).toString();
            for (const QJsonValue &capability : object.value(QStringLiteral("capabilities")).toArray()) {
                const QString cap = capability.toString().trimmed();
                if (!cap.isEmpty() && !entry.capabilities.contains(cap)) entry.capabilities.append(cap);
            }
            // Operational permissions need to see modern hard requirements as well.
            for (const QJsonValue &capability : object.value(QStringLiteral("required_capabilities")).toArray()) {
                const QString cap = capability.toString().trimmed();
                if (!cap.isEmpty() && !entry.capabilities.contains(cap)) entry.capabilities.append(cap);
            }
            const QJsonObject trustObject = object.value(QStringLiteral("trust")).toObject();
            const QJsonObject licenseObject = object.value(QStringLiteral("license")).toObject();
            const PublisherTrustDecision publisherTrust = resolvePublisherTrust(object, authoritativeCatalog, true);
            const QString publisherType = publisherTrust.type;
            const int trustPluginLevel = qBound(1, !trustObject.isEmpty() ? trustObject.value(QStringLiteral("plugin_level")).toInt(1) : object.value(QStringLiteral("plugin_level")).toInt(1), 3);
            const bool trustNativeAllowed = authoritativeCatalog && !trustObject.isEmpty()
                && trustObject.value(QStringLiteral("native_allowed")).toBool(false) && trustPluginLevel == 3;
            const bool verifiedNative = publisherType == QStringLiteral("verified") && trustNativeAllowed;
            entry.publisherVerification = verifiedNative ? QStringLiteral("verified_native") : publisherType;
            entry.publisherVerified = publisherTrust.verified;
            entry.publisherStatus = !authoritativeCatalog ? QStringLiteral("Third-party Catalog")
                : publisherType == QStringLiteral("official") ? QStringLiteral("Official Publisher")
                : verifiedNative ? QStringLiteral("Verified + Native/L3")
                : publisherType == QStringLiteral("verified") ? QStringLiteral("Verified Publisher")
                : QStringLiteral("Unverified Publisher");
            entry.reviewed = authoritativeCatalog && (!trustObject.isEmpty() ? trustObject.value(QStringLiteral("reviewed")).toBool(false) : object.value(QStringLiteral("reviewed")).toBool(false));
            entry.license = !licenseObject.isEmpty() ? licenseObject.value(QStringLiteral("id")).toString() : object.value(QStringLiteral("license")).toString();
            entry.licenseFile = object.value(QStringLiteral("license_file")).toString();
            entry.sourceUrl = !licenseObject.isEmpty() ? licenseObject.value(QStringLiteral("source_url")).toString() : object.value(QStringLiteral("source_url")).toString();
            entry.sourceAvailable = !licenseObject.isEmpty() ? licenseObject.value(QStringLiteral("source_available")).toBool(false) : object.value(QStringLiteral("source_available")).toBool(false);
            entry.saveVersion = qMax(1, object.value(QStringLiteral("save_version")).toInt(1));
            entry.defaultLocale = object.value(QStringLiteral("default_locale")).toString(QStringLiteral("en"));
            for (const QJsonValue &locale : object.value(QStringLiteral("locales")).toArray())
                entry.locales.append(locale.toString());
            // RCC execution is always confined to Level 1/2. Native/L3 permission only affects
            // the verified publisher badge; native artifacts use PluginManager's separate path.
            entry.pluginLevel = qBound(1, trustPluginLevel, 2);
            entry.native = false;

            if (installed.contains(entry.id)) {
                const Entry local = installed.take(entry.id);
                entry.installed = true;
                entry.installedVersion = local.installedVersion;
                entry.installedFile = local.installedFile;
                entry.installedEntry = local.installedEntry;
                entry.installedMountRoot = local.installedMountRoot;
                entry.settingsSchema = local.settingsSchema;
                if (entry.license.isEmpty()) entry.license = local.license;
                if (entry.licenseFile.isEmpty()) entry.licenseFile = local.licenseFile;
                if (entry.sourceUrl.isEmpty()) entry.sourceUrl = local.sourceUrl;
                if (entry.tags.isEmpty()) entry.tags = local.tags;
                if (entry.apiVersion.isEmpty()) entry.apiVersion = local.apiVersion;
                if (entry.capabilities.isEmpty()) entry.capabilities = local.capabilities;
                if (entry.locales.isEmpty()) entry.locales = local.locales;
                if (entry.defaultLocale.isEmpty()) entry.defaultLocale = local.defaultLocale;
                if (entry.saveVersion <= 1) entry.saveVersion = local.saveVersion;
                entry.state = QStringLiteral("installed");
                const QString localIconPath = QStringLiteral(":/mods/%1/assets/icon.png").arg(entry.id);
                if (QFileInfo::exists(localIconPath))
                    entry.iconUrl = QStringLiteral("qrc:/mods/%1/assets/icon.png").arg(entry.id);
            }
            fresh.append(entry);
        }

        for (Entry local : installed) {
            local.inCatalog = false;
            fresh.append(local);
        }

        beginResetModel();
        m_entries = fresh;
        endResetModel();
        emit countChanged();
        setError(QString{});
    });
}

void ModManager::install(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;

    Entry &entry = m_entries[row];
    if (entry.state == QStringLiteral("resolving") ||
        entry.state == QStringLiteral("downloading") ||
        entry.state == QStringLiteral("installing"))
        return;

    if (entry.packageFormat != QStringLiteral("rcc-v1")) {
        failEntry(row, QStringLiteral("Unsupported mod package format."));
        return;
    }
    if (!safeEntryPath(entry.entryPath)) {
        failEntry(row, QStringLiteral("Unsafe mod entry path."));
        return;
    }
    if (!versionAtLeast(QCoreApplication::applicationVersion(), entry.minAppVersion)) {
        failEntry(row, QStringLiteral("This mod requires LeoMiniGames %1 or newer.")
                        .arg(entry.minAppVersion));
        return;
    }
    if (entry.sizeBytes <= 0 || entry.sizeBytes > kMaxModBytes) {
        failEntry(row, QStringLiteral("Mod size is invalid or exceeds the 64 MiB safety limit."));
        return;
    }
    if (!QRegularExpression(QStringLiteral("^[a-f0-9]{64}$")).match(entry.sha256).hasMatch()) {
        failEntry(row, QStringLiteral("Catalog SHA-256 is invalid."));
        return;
    }

    entry.state = QStringLiteral("resolving");
    entry.operationError.clear();
    entry.progress = 0.0;
    updateRow(row, {StateRole, ErrorRole, ProgressRole});

    const QUrl directUrl(entry.downloadUrl);
    if (directUrl.isValid() && directUrl.scheme() == QStringLiteral("https") && !directUrl.host().isEmpty()) {
        startPackageDownload(row, directUrl, entry.sha256, entry.sizeBytes);
        return;
    }

    requestDownloadTicket(row);
}

void ModManager::requestDownloadTicket(int row)
{
    if (row < 0 || row >= m_entries.size())
        return;

    Entry &entry = m_entries[row];
    const QString endpoint = entry.downloadEndpoint.isEmpty()
        ? QStringLiteral("/api/v1/mods/%1/download").arg(entry.id)
        : entry.downloadEndpoint;

    const QUrl baseUrl(apiBaseUrl());
    const QUrl ticketUrl = resolveEndpoint(apiBaseUrl(), endpoint);
    if (!ticketUrl.isValid() || ticketUrl.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0 ||
        ticketUrl.host().isEmpty() || ticketUrl.scheme().compare(baseUrl.scheme(), Qt::CaseInsensitive) != 0 ||
        ticketUrl.host().compare(baseUrl.host(), Qt::CaseInsensitive) != 0 || ticketUrl.port(-1) != baseUrl.port(-1)) {
        failEntry(row, QStringLiteral("Download ticket endpoint must remain on the configured catalog origin."));
        return;
    }
    QNetworkRequest request(ticketUrl);
    request.setRawHeader("Accept", "application/json");
    request.setTransferTimeout(15000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
    QNetworkReply *reply = m_impl->network.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, row] {
        const QByteArray payload = reply->readAll();
        if (payload.size() > 512 * 1024) { reply->deleteLater(); failEntry(row, QStringLiteral("Download ticket response is too large.")); return; }
        const auto error = reply->error();
        const QString errorString = reply->errorString();
        reply->deleteLater();

        if (row < 0 || row >= m_entries.size())
            return;
        if (error != QNetworkReply::NoError) {
            QString serverMessage;
            QJsonParseError serverParseError;
            const QJsonDocument serverDoc = QJsonDocument::fromJson(payload, &serverParseError);
            if (serverParseError.error == QJsonParseError::NoError && serverDoc.isObject())
                serverMessage = serverDoc.object().value(QStringLiteral("error")).toObject().value(QStringLiteral("message")).toString();

            if (serverMessage.isEmpty())
                serverMessage = errorString;
            failEntry(row, QStringLiteral("Download ticket failed: %1").arg(serverMessage));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        const QJsonObject data = document.object().value(QStringLiteral("data")).toObject();
        const QUrl url(data.value(QStringLiteral("url")).toString());
        const QString sha = data.value(QStringLiteral("sha256")).toString(m_entries[row].sha256).toLower();
        const qint64 size = data.value(QStringLiteral("size_bytes")).toVariant().toLongLong();

        if (parseError.error != QJsonParseError::NoError || !url.isValid() ||
            !QRegularExpression(QStringLiteral("^[a-f0-9]{64}$")).match(sha).hasMatch()) {
            failEntry(row, QStringLiteral("Download ticket is invalid."));
            return;
        }

        startPackageDownload(row, url, sha, size > 0 ? size : m_entries[row].sizeBytes);
    });
}

void ModManager::startPackageDownload(int row, const QUrl &url,
                                      const QString &expectedSha, qint64 expectedSize)
{
    if (row < 0 || row >= m_entries.size())
        return;
    if (!url.isValid() || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0 || url.host().isEmpty()) {
        failEntry(row, QStringLiteral("Mod downloads require a valid HTTPS URL."));
        return;
    }

    Entry &entry = m_entries[row];
    const QString versionDir = m_paths->mods() + QLatin1Char('/') + entry.id +
                               QLatin1Char('/') + entry.version;
    if (!QDir().mkpath(versionDir)) {
        failEntry(row, QStringLiteral("Could not create the mod directory."));
        return;
    }

    auto job = std::make_shared<Impl::DownloadJob>();
    job->row = row;
    job->id = entry.id;
    job->expectedSha = expectedSha;
    job->expectedSize = expectedSize;
    job->finalPath = versionDir + QLatin1Char('/') + entry.id + QStringLiteral(".rcc");
    job->partPath = job->finalPath + QStringLiteral(".part");
    QFile::remove(job->partPath);
    job->file.setFileName(job->partPath);

    if (!job->file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        failEntry(row, QStringLiteral("Could not open the mod download file."));
        return;
    }

    entry.state = QStringLiteral("downloading");
    entry.progress = 0.0;
    updateRow(row, {StateRole, ProgressRole});

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/octet-stream");
    request.setTransferTimeout(60000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_impl->network.get(request);
    m_impl->downloads.insert(reply, job);

    connect(reply, &QNetworkReply::downloadProgress, this, [this, job](qint64 received, qint64 total) {
        if (!job || job->row < 0 || job->row >= m_entries.size())
            return;
        Entry &entry = m_entries[job->row];
        const qint64 denominator = job->expectedSize > 0 ? job->expectedSize : total;
        if (denominator > 0) {
            entry.progress = qBound<qreal>(0.0, static_cast<qreal>(received) / denominator, 1.0);
            updateRow(job->row, {ProgressRole});
        }
    });

    connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
        const auto job = m_impl->downloads.value(reply);
        if (!job || !reply->isOpen() || !reply->isReadable())
            return;
        const QByteArray chunk = reply->readAll();
        job->received += chunk.size();
        if (job->received > kMaxModBytes) {
            reply->abort();
            return;
        }
        job->hash.addData(chunk);
        if (job->file.write(chunk) != chunk.size()) {
            reply->abort();
            return;
        }

        if (job->row >= 0 && job->row < m_entries.size()) {
            Entry &entry = m_entries[job->row];
            const qint64 total = reply->bytesAvailable() + job->received;
            const qint64 denominator = job->expectedSize > 0 ? job->expectedSize : total;
            entry.progress = denominator > 0
                ? qBound<qreal>(0.0, static_cast<qreal>(job->received) / denominator, 1.0)
                : 0.0;
            updateRow(job->row, {ProgressRole});
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto job = m_impl->downloads.take(reply);
        if (!job) {
            reply->deleteLater();
            return;
        }

        bool finalWriteOk = true;
        if (reply->isOpen() && reply->isReadable() && reply->bytesAvailable() > 0) {
            const QByteArray chunk = reply->readAll();
            if (job->received + chunk.size() > kMaxModBytes) {
                finalWriteOk = false;
            } else {
                job->received += chunk.size();
                job->hash.addData(chunk);
                finalWriteOk = job->file.write(chunk) == chunk.size();
            }
        }
        job->file.close();

        const auto error = reply->error();
        const QString errorString = reply->errorString();
        reply->deleteLater();

        if (job->row < 0 || job->row >= m_entries.size()) {
            QFile::remove(job->partPath);
            return;
        }

        Entry &entry = m_entries[job->row];
        if (error != QNetworkReply::NoError || job->received > kMaxModBytes || !finalWriteOk) {
            QFile::remove(job->partPath);
            failEntry(job->row, QStringLiteral("Download failed: %1").arg(errorString));
            return;
        }

        const QString actualSha = QString::fromLatin1(job->hash.result().toHex());
        if (actualSha.compare(job->expectedSha, Qt::CaseInsensitive) != 0) {
            QFile::remove(job->partPath);
            failEntry(job->row, QStringLiteral("SHA-256 verification failed."));
            return;
        }
        if (job->expectedSize > 0 && job->received != job->expectedSize) {
            QFile::remove(job->partPath);
            failEntry(job->row, QStringLiteral("Downloaded file size does not match the catalog."));
            return;
        }

        entry.state = QStringLiteral("installing");
        entry.progress = 1.0;
        updateRow(job->row, {StateRole, ProgressRole});

        const bool hadPrevious = entry.installed && !entry.installedFile.isEmpty() && QFileInfo::exists(entry.installedFile);
        const QString previousFile = entry.installedFile;
        const QString previousVersion = entry.installedVersion;
        const QString previousEntry = entry.installedEntry;
        const QString previousMountRoot = entry.installedMountRoot.isEmpty() ? QStringLiteral("/mods/%1").arg(entry.id) : entry.installedMountRoot;
        const QString rollbackFile = job->finalPath + QStringLiteral(".rollback");
        QFile::remove(rollbackFile);

        if (hadPrevious) {
            QResource::unregisterResource(previousFile, previousMountRoot);
            if (QFileInfo(previousFile).absoluteFilePath() == QFileInfo(job->finalPath).absoluteFilePath()) {
                if (!QFile::rename(previousFile, rollbackFile)) {
                    QResource::registerResource(previousFile, previousMountRoot);
                    failEntry(job->row, QStringLiteral("Could not prepare the installed mod for replacement."));
                    return;
                }
            }
        }

        const auto rollback = [&](const QString &message) {
            if (!entry.installedMountRoot.isEmpty()) QResource::unregisterResource(job->finalPath, entry.installedMountRoot);
            QFile::remove(job->finalPath);
            if (QFileInfo::exists(rollbackFile))
                QFile::rename(rollbackFile, previousFile);
            if (hadPrevious && QFileInfo::exists(previousFile) && QResource::registerResource(previousFile, previousMountRoot)) {
                entry.installed = true;
                entry.installedFile = previousFile;
                entry.installedVersion = previousVersion;
                entry.installedEntry = previousEntry;
                entry.installedMountRoot = previousMountRoot;
                entry.state = QStringLiteral("installed");
                entry.operationError = QStringLiteral("%1 Previous version restored.").arg(message);
                updateRow(job->row, {InstalledRole, InstalledVersionRole, StateRole, ErrorRole, ProgressRole});
            } else {
                entry.installed = false;
                failEntry(job->row, message);
            }
        };

        QFile::remove(job->finalPath);
        if (!QFile::rename(job->partPath, job->finalPath)) { rollback(QStringLiteral("Could not finalize the mod package.")); return; }
        const RccPackageInspection inspection = RccPackageInspector::inspect(job->finalPath, entry.id, entry.entryPath);
        if (!inspection.valid) { rollback(QStringLiteral("RCC validation failed: %1").arg(inspection.error)); return; }
        entry.installedMountRoot = inspection.mountRoot;
        entry.entryPath = inspection.entryPath;
        if (!RccPackageInspector::mount(job->finalPath, inspection)) { rollback(QStringLiteral("Qt rejected the validated RCC package.")); return; }

        const QString resourcePath = QStringLiteral(":/mods/%1/%2").arg(entry.id, inspection.entryPath);
        if (!QFileInfo::exists(resourcePath)) { rollback(QStringLiteral("The mod entry file is missing after RCC mount.")); return; }
        QFile::remove(rollbackFile);
        if (hadPrevious && QFileInfo(previousFile).absoluteFilePath() != QFileInfo(job->finalPath).absoluteFilePath()) {
            const QString oldVersionDir = QFileInfo(previousFile).absolutePath();
            QFile::remove(previousFile);
            QDir(oldVersionDir).removeRecursively();
        }

        entry.installed = true;
        entry.installedVersion = entry.version;
        entry.installedFile = job->finalPath;
        entry.installedEntry = entry.entryPath;
        entry.state = QStringLiteral("installed");
        entry.operationError.clear();
        entry.progress = 1.0;
        applyManifestMetadata(entry);
        const QString localIconPath = QStringLiteral(":/mods/%1/assets/icon.png").arg(entry.id);
        if (QFileInfo::exists(localIconPath))
            entry.iconUrl = QStringLiteral("qrc:/mods/%1/assets/icon.png").arg(entry.id);
        saveInstalled();
        updateRow(job->row, {InstalledRole, InstalledVersionRole, StateRole, ErrorRole, ProgressRole, IconUrlRole});
        emit modInstalled(entry.id);
    });
}

void ModManager::uninstall(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;

    Entry &entry = m_entries[row];
    if (!entry.installed)
        return;
    if (entry.state == QStringLiteral("resolving") ||
        entry.state == QStringLiteral("downloading") ||
        entry.state == QStringLiteral("installing"))
        return;

    const QString mapRoot = entry.installedMountRoot.isEmpty() ? QStringLiteral("/mods/%1").arg(entry.id) : entry.installedMountRoot;
    if (!entry.installedFile.isEmpty())
        QResource::unregisterResource(entry.installedFile, mapRoot);

    const QString idDir = m_paths->mods() + QLatin1Char('/') + entry.id;
    QDir(idDir).removeRecursively();

    entry.installed = false;
    entry.installedVersion.clear();
    entry.installedFile.clear();
    entry.installedEntry.clear();
    entry.installedMountRoot.clear();
    entry.state = QStringLiteral("idle");
    entry.progress = 0.0;
    entry.operationError.clear();

    saveInstalled();
    updateRow(row, {InstalledRole, InstalledVersionRole, StateRole, ProgressRole, ErrorRole});
    emit modUninstalled(id);
}

bool ModManager::isInstalled(const QString &id) const
{
    const int row = indexOf(id);
    return row >= 0 && m_entries.at(row).installed;
}

QString ModManager::installedVersion(const QString &id) const
{
    const int row = indexOf(id);
    return row >= 0 ? m_entries.at(row).installedVersion : QString{};
}

QUrl ModManager::entryUrl(const QString &id) const
{
    const int row = indexOf(id);
    if (row < 0)
        return QUrl{};

    const Entry &entry = m_entries.at(row);
    if (!entry.installed)
        return QUrl{};

    const QString path = entry.installedEntry.isEmpty()
        ? entry.entryPath
        : entry.installedEntry;
    return QUrl(QStringLiteral("qrc:/mods/%1/%2").arg(entry.id, path));
}



QUrl ModManager::licenseUrl(const QString &id) const
{
    const int row = indexOf(id);
    if (row < 0 || !m_entries.at(row).installed || m_entries.at(row).licenseFile.isEmpty() ||
        !safeEntryPath(m_entries.at(row).licenseFile))
        return QUrl{};
    const QString resource = QStringLiteral(":/mods/%1/%2").arg(id, m_entries.at(row).licenseFile);
    return QFileInfo::exists(resource) ? QUrl(QStringLiteral("qrc:/mods/%1/%2").arg(id, m_entries.at(row).licenseFile)) : QUrl{};
}

QUrl ModManager::sourceUrlFor(const QString &id) const
{
    const int row = indexOf(id);
    if (row < 0 || !m_entries.at(row).sourceAvailable)
        return QUrl{};
    const QUrl url(m_entries.at(row).sourceUrl);
    return url.isValid() && (url.scheme() == QStringLiteral("https") || url.scheme() == QStringLiteral("http")) ? url : QUrl{};
}

QString ModManager::defaultLocaleFor(const QString &id) const
{
    const int row = indexOf(id); return row >= 0 ? m_entries.at(row).defaultLocale : QStringLiteral("en");
}
QStringList ModManager::localesFor(const QString &id) const
{
    const int row = indexOf(id); return row >= 0 ? m_entries.at(row).locales : QStringList{};
}
int ModManager::saveVersionFor(const QString &id) const
{
    const int row = indexOf(id); return row >= 0 ? qMax(1, m_entries.at(row).saveVersion) : 1;
}
QVariantMap ModManager::settingsSchemaFor(const QString &id) const
{
    const int row = indexOf(id); return row >= 0 ? m_entries.at(row).settingsSchema : QVariantMap{};
}

bool ModManager::networkAllowedFor(const QString &id) const
{
    const int row = indexOf(id);
    if (row < 0 || !m_entries.at(row).installed)
        return false;
    const Entry &entry = m_entries.at(row);
    // v0.5/v0.6 packages historically ran in the host engine and therefore had network
    // access. Keep that behavior for legacy manifests; v0.7+ must opt in explicitly.
    const QVersionNumber api = QVersionNumber::fromString(entry.apiVersion);
    if (entry.apiVersion.trimmed().isEmpty() || api.isNull() || QVersionNumber::compare(api, QVersionNumber(0, 7)) < 0)
        return true;
    for (const QString &capability : entry.capabilities) {
        const QString cap = capability.trimmed().toLower();
        if (cap == QStringLiteral("network") || cap == QStringLiteral("network.https"))
            return true;
    }
    return false;
}

void ModManager::applyManifestMetadata(Entry &entry)
{
    QFile file(QStringLiteral(":/mods/%1/manifest.json").arg(entry.id));
    if (!file.open(QIODevice::ReadOnly))
        return;
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return;
    const QJsonObject object = doc.object();
    const QString manifestId = object.value(QStringLiteral("id")).toString();
    if (!manifestId.isEmpty() && manifestId != entry.id)
        return;

    if (entry.name.isEmpty()) entry.name = object.value(QStringLiteral("name")).toString(entry.id);
    if (entry.localizedName.isEmpty()) entry.localizedName = entry.name;
    if (entry.description.isEmpty()) entry.description = object.value(QStringLiteral("description")).toString();
    if (entry.author.isEmpty()) entry.author = object.value(QStringLiteral("publisher")).toString(object.value(QStringLiteral("author")).toString());
    if (entry.category.isEmpty()) entry.category = object.value(QStringLiteral("category")).toString(QStringLiteral("Mod"));
    if (entry.icon.isEmpty()) entry.icon = object.value(QStringLiteral("icon")).toString(QStringLiteral("MOD"));
    const QString iconPath = object.value(QStringLiteral("icon_path")).toString();
    if (!iconPath.isEmpty() && safeEntryPath(iconPath)) {
        const QString resourceIcon = QStringLiteral(":/mods/%1/%2").arg(entry.id, iconPath);
        if (QFileInfo::exists(resourceIcon))
            entry.iconUrl = QStringLiteral("qrc:/mods/%1/%2").arg(entry.id, iconPath);
    }
    if (entry.license.isEmpty()) entry.license = object.value(QStringLiteral("license")).toString();
    if (entry.licenseFile.isEmpty()) entry.licenseFile = object.value(QStringLiteral("license_file")).toString();
    if (entry.sourceUrl.isEmpty()) entry.sourceUrl = object.value(QStringLiteral("source_url")).toString();
    entry.sourceAvailable = entry.sourceAvailable || object.value(QStringLiteral("source_available")).toBool(false);
    if (entry.tags.isEmpty()) for (const QJsonValue &v : object.value(QStringLiteral("tags")).toArray()) entry.tags.append(v.toString());
    if (entry.apiVersion.isEmpty()) entry.apiVersion = object.value(QStringLiteral("api_version")).toString();
    for (const QJsonValue &v : object.value(QStringLiteral("capabilities")).toArray()) {
        const QString cap=v.toString().trimmed();
        if(!cap.isEmpty()&&!entry.capabilities.contains(cap))entry.capabilities.append(cap);
    }
    for (const QJsonValue &v : object.value(QStringLiteral("required_capabilities")).toArray()) {
        const QString cap=v.toString().trimmed();
        if(!cap.isEmpty()&&!entry.capabilities.contains(cap))entry.capabilities.append(cap);
    }
    if (entry.locales.isEmpty()) for (const QJsonValue &v : object.value(QStringLiteral("locales")).toArray()) entry.locales.append(v.toString());
    const QString defaultLocale = object.value(QStringLiteral("default_locale")).toString();
    if (!defaultLocale.isEmpty()) entry.defaultLocale = defaultLocale;
    entry.saveVersion = qMax(entry.saveVersion, qMax(1, object.value(QStringLiteral("save_version")).toInt(1)));
    const QJsonObject schema = object.value(QStringLiteral("settings_schema")).toObject();
    if (!schema.isEmpty()) entry.settingsSchema = schema.toVariantMap();
    // Security boundary: verified/native/reviewed/Level 3 are never trusted from manifest.json.
    entry.pluginLevel = qBound(1, entry.pluginLevel, 2);
    entry.native = false;
}

void ModManager::registerInstalled(Entry &entry)
{
    if (entry.installedFile.isEmpty() || !QFileInfo::exists(entry.installedFile)) {
        entry.installed = false;
        return;
    }

    const RccPackageInspection inspection = RccPackageInspector::inspect(entry.installedFile, entry.id, entry.installedEntry);
    if (!inspection.valid) {
        entry.installed = false;
        entry.state = QStringLiteral("error");
        entry.operationError = inspection.error.isEmpty() ? QStringLiteral("Could not inspect installed RCC package.") : inspection.error;
        return;
    }
    {
        QStringList required;
        for (const QJsonValue &value : inspection.manifest.value(QStringLiteral("required_capabilities")).toArray())
            required.append(value.toString());
        GameRuntime runtime;
        const QVariantMap compatibility = runtime.checkCompatibility(
            inspection.manifest.value(QStringLiteral("api_version")).toString(),
            inspection.manifest.value(QStringLiteral("min_api_version")).toString(), required);
        if (!compatibility.value(QStringLiteral("ok")).toBool()) {
            entry.installed = false;
            entry.state = QStringLiteral("error");
            entry.operationError = QStringLiteral("Installed mod is incompatible: %1").arg(compatibility.value(QStringLiteral("error")).toString());
            return;
        }
    }
    if (!RccPackageInspector::mount(entry.installedFile, inspection)) {
        entry.installed = false;
        entry.state = QStringLiteral("error");
        entry.operationError = QStringLiteral("Could not register installed RCC package.");
        return;
    }
    entry.installedMountRoot = inspection.mountRoot;
    entry.installedEntry = inspection.entryPath;

    const QString resourcePath = QStringLiteral(":/mods/%1/%2").arg(entry.id, entry.installedEntry);
    if (!QFileInfo::exists(resourcePath)) {
        RccPackageInspector::unmount(entry.installedFile, inspection);
        entry.installed = false;
        entry.state = QStringLiteral("error");
        entry.operationError = QStringLiteral("Installed mod entry file is missing.");
        return;
    }

    entry.installed = true;
    entry.state = QStringLiteral("installed");
    applyManifestMetadata(entry);
    const QString localIconPath = QStringLiteral(":/mods/%1/assets/icon.png").arg(entry.id);
    if (QFileInfo::exists(localIconPath))
        entry.iconUrl = QStringLiteral("qrc:/mods/%1/assets/icon.png").arg(entry.id);
}

void ModManager::loadInstalled()
{
    const QString indexPath = m_paths->mods() + QStringLiteral("/installed.json");
    QFile file(indexPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isArray())
        return;

    beginResetModel();
    m_entries.clear();
    for (const QJsonValue &value : document.array()) {
        const QJsonObject object = value.toObject();
        Entry entry;
        entry.id = object.value(QStringLiteral("id")).toString();
        entry.name = object.value(QStringLiteral("name")).toString(entry.id);
        entry.description = object.value(QStringLiteral("description")).toString();
        entry.author = object.value(QStringLiteral("author")).toString();
        entry.version = object.value(QStringLiteral("catalog_version")).toString();
        entry.category = object.value(QStringLiteral("category")).toString(QStringLiteral("Mod"));
        entry.icon = object.value(QStringLiteral("icon")).toString(QStringLiteral("MOD"));
        entry.iconUrl = object.value(QStringLiteral("icon_url")).toString();
        entry.localizedName = object.value(QStringLiteral("localized_name")).toString(entry.name);
        for (const QJsonValue &v : object.value(QStringLiteral("tags")).toArray()) entry.tags.append(v.toString());
        entry.apiVersion = object.value(QStringLiteral("api_version")).toString();
        for (const QJsonValue &v : object.value(QStringLiteral("capabilities")).toArray()) if (v.isString()) entry.capabilities.append(v.toString());
        entry.publisherStatus = object.value(QStringLiteral("publisher_status")).toString(QStringLiteral("Local/Sideloaded"));
        entry.publisherVerified = false;
        entry.publisherVerification = QStringLiteral("unverified");
        entry.reviewed = object.value(QStringLiteral("reviewed")).toBool(false);
        entry.license = object.value(QStringLiteral("license")).toString();
        entry.licenseFile = object.value(QStringLiteral("license_file")).toString();
        entry.sourceUrl = object.value(QStringLiteral("source_url")).toString();
        entry.sourceAvailable = object.value(QStringLiteral("source_available")).toBool(false);
        entry.pluginLevel = qBound(1, object.value(QStringLiteral("plugin_level")).toInt(1), 2);
        entry.native = false;
        entry.saveVersion = qMax(1, object.value(QStringLiteral("save_version")).toInt(1));
        entry.defaultLocale = object.value(QStringLiteral("default_locale")).toString(QStringLiteral("en"));
        for (const QJsonValue &v : object.value(QStringLiteral("locales")).toArray()) entry.locales.append(v.toString());
        entry.settingsSchema = object.value(QStringLiteral("settings_schema")).toObject().toVariantMap();
        entry.updatedAt = object.value(QStringLiteral("updated_at")).toString();
        entry.inCatalog = object.value(QStringLiteral("in_catalog")).toBool(false);
        entry.installed = true;
        entry.installedVersion = object.value(QStringLiteral("installed_version")).toString();
        entry.installedFile = object.value(QStringLiteral("file")).toString();
        entry.installedEntry = object.value(QStringLiteral("entry")).toString(QStringLiteral("Main.qml"));
        entry.installedMountRoot = object.value(QStringLiteral("mount_root")).toString();
        entry.entryPath = entry.installedEntry;
        entry.packageFormat = QStringLiteral("rcc-v1");

        if (!validId(entry.id) || !safeEntryPath(entry.installedEntry))
            continue;
        registerInstalled(entry);
        m_entries.append(entry);
    }
    endResetModel();
    emit countChanged();
}

void ModManager::saveInstalled() const
{
    QJsonArray array;
    for (const Entry &entry : m_entries) {
        if (!entry.installed)
            continue;
        QJsonObject object;
        object.insert(QStringLiteral("id"), entry.id);
        object.insert(QStringLiteral("name"), entry.name);
        object.insert(QStringLiteral("description"), entry.description);
        object.insert(QStringLiteral("author"), entry.author);
        object.insert(QStringLiteral("category"), entry.category);
        object.insert(QStringLiteral("icon"), entry.icon);
        object.insert(QStringLiteral("icon_url"), entry.iconUrl);
        object.insert(QStringLiteral("localized_name"), entry.localizedName);
        object.insert(QStringLiteral("tags"), QJsonArray::fromStringList(entry.tags));
        object.insert(QStringLiteral("api_version"), entry.apiVersion);
        object.insert(QStringLiteral("capabilities"), QJsonArray::fromStringList(entry.capabilities));
        object.insert(QStringLiteral("publisher_status"), entry.publisherStatus);
        object.insert(QStringLiteral("publisher_verified"), false); // trust is refreshed from the authoritative catalog each run
        object.insert(QStringLiteral("publisher_verification"), QStringLiteral("unverified"));
        object.insert(QStringLiteral("reviewed"), entry.reviewed);
        object.insert(QStringLiteral("license"), entry.license);
        object.insert(QStringLiteral("license_file"), entry.licenseFile);
        object.insert(QStringLiteral("source_url"), entry.sourceUrl);
        object.insert(QStringLiteral("source_available"), entry.sourceAvailable);
        object.insert(QStringLiteral("plugin_level"), qBound(1, entry.pluginLevel, 2));
        object.insert(QStringLiteral("native"), false);
        object.insert(QStringLiteral("save_version"), entry.saveVersion);
        object.insert(QStringLiteral("default_locale"), entry.defaultLocale);
        object.insert(QStringLiteral("locales"), QJsonArray::fromStringList(entry.locales));
        object.insert(QStringLiteral("settings_schema"), QJsonObject::fromVariantMap(entry.settingsSchema));
        object.insert(QStringLiteral("updated_at"), entry.updatedAt);
        object.insert(QStringLiteral("in_catalog"), entry.inCatalog);
        object.insert(QStringLiteral("catalog_version"), entry.version);
        object.insert(QStringLiteral("installed_version"), entry.installedVersion);
        object.insert(QStringLiteral("file"), entry.installedFile);
        object.insert(QStringLiteral("entry"), entry.installedEntry);
        object.insert(QStringLiteral("mount_root"), entry.installedMountRoot);
        array.append(object);
    }

    QSaveFile file(m_paths->mods() + QStringLiteral("/installed.json"));
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    file.commit();
}
