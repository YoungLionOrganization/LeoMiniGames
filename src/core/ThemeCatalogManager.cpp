// SPDX-License-Identifier: GPL-3.0-or-later
#include "ThemeCatalogManager.h"

#include "AppPaths.h"
#include "SettingsManager.h"
#include "ThemeManager.h"
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
#include <QSet>
#include <QUrl>
#include <QVersionNumber>
#include <QtGlobal>

#include <memory>
#include <utility>

namespace {
constexpr qint64 kMaxThemeDownloadBytes = 32LL * 1024LL * 1024LL;
constexpr qint64 kMaxThemeJsonBytes = 2LL * 1024LL * 1024LL;

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
}

struct ThemeCatalogManager::Impl
{
    explicit Impl(QObject *owner) : network(owner) {}

    struct DownloadJob {
        int row = -1;
        QString id;
        QString partPath;
        QString expectedSha;
        qint64 expectedSize = 0;
        qint64 received = 0;
        QFile file;
        QCryptographicHash hash{QCryptographicHash::Sha256};
    };

    QNetworkAccessManager network;
    QHash<QNetworkReply *, std::shared_ptr<DownloadJob>> downloads;
    QSet<QString> activeOperations;
};

ThemeCatalogManager::ThemeCatalogManager(AppPaths *paths, SettingsManager *settings, ThemeManager *themes, QObject *parent)
    : QAbstractListModel(parent),
      m_paths(paths),
      m_settings(settings),
      m_themes(themes),
      m_impl(std::make_unique<Impl>(this))
{
    if (m_themes) {
        connect(m_themes, &ThemeManager::themesChanged, this, &ThemeCatalogManager::notifyInstallStateChanged);
        connect(m_themes, &ThemeManager::activeThemeChanged, this, &ThemeCatalogManager::notifyInstallStateChanged);
    }
}

ThemeCatalogManager::~ThemeCatalogManager() = default;

int ThemeCatalogManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant ThemeCatalogManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return QVariant{};
    const Entry &entry = m_entries.at(index.row());
    const bool installed = m_themes && m_themes->isInstalled(entry.id);
    const QString installedVersion = installed ? m_themes->installedVersion(entry.id) : QString{};
    const bool update = installed && versionGreater(entry.version, installedVersion);
    const bool active = m_themes && m_themes->activeThemeId() == entry.id;
    switch (role) {
    case IdRole: return entry.id;
    case NameRole: return entry.name;
    case DescriptionRole: return entry.description;
    case PublisherRole: return entry.publisher;
    case PublisherStatusRole: return entry.publisherStatus;
    case PublisherVerifiedRole: return entry.publisherVerified;
    case PublisherVerificationRole: return entry.publisherVerification;
    case VersionRole: return entry.version;
    case CategoryRole: return entry.category;
    case TagsRole: return entry.tags;
    case IconUrlRole: return entry.iconUrl;
    case PreviewUrlsRole: return entry.previewUrls;
    case SizeRole: return entry.sizeBytes;
    case DownloadsRole: return entry.downloads;
    case UpdatedAtRole: return entry.updatedAt;
    case MinAppVersionRole: return entry.minAppVersion;
    case PackageFormatRole: return entry.packageFormat;
    case ThemeApiVersionRole: return entry.themeApiVersion;
    case InstalledRole: return installed;
    case InstalledVersionRole: return installedVersion;
    case UpdateAvailableRole: return update;
    case ActiveRole: return active;
    case StateRole:
        if (entry.state != QStringLiteral("idle"))
            return entry.state;
        return update ? QStringLiteral("update_available") : (installed ? QStringLiteral("installed") : QStringLiteral("idle"));
    case ProgressRole: return entry.progress;
    case OperationErrorRole: return entry.operationError;
    default: return QVariant{};
    }
}

QHash<int, QByteArray> ThemeCatalogManager::roleNames() const
{
    return {
        {IdRole, "themeId"}, {NameRole, "themeName"}, {DescriptionRole, "themeDescription"},
        {PublisherRole, "themePublisher"}, {PublisherStatusRole, "themePublisherStatus"},
        {PublisherVerifiedRole, "themePublisherVerified"}, {PublisherVerificationRole, "themePublisherVerification"}, {VersionRole, "themeVersion"},
        {CategoryRole, "themeCategory"}, {TagsRole, "themeTags"}, {IconUrlRole, "themeIconUrl"},
        {PreviewUrlsRole, "themePreviewUrls"}, {SizeRole, "themeSizeBytes"}, {DownloadsRole, "themeDownloads"},
        {UpdatedAtRole, "themeUpdatedAt"}, {MinAppVersionRole, "themeMinAppVersion"},
        {PackageFormatRole, "themePackageFormat"}, {ThemeApiVersionRole, "themeApiVersion"},
        {InstalledRole, "themeInstalled"}, {InstalledVersionRole, "themeInstalledVersion"},
        {UpdateAvailableRole, "themeUpdateAvailable"}, {ActiveRole, "themeActive"}, {StateRole, "themeState"},
        {ProgressRole, "themeProgress"}, {OperationErrorRole, "themeOperationError"}
    };
}

bool ThemeCatalogManager::loading() const { return m_loading; }
QString ThemeCatalogManager::error() const { return m_error; }
int ThemeCatalogManager::count() const { return rowCount(); }

QString ThemeCatalogManager::apiBaseUrl() const
{
    if (!m_settings)
        return QStringLiteral("https://leominigames.younglion.xyz/api/v1");
    const QString shared = m_settings->value(QStringLiteral("mods/apiBaseUrl"),
        QStringLiteral("https://leominigames.younglion.xyz/api/v1")).toString();
    return normalizedBaseUrl(m_settings->value(QStringLiteral("themes/apiBaseUrl"), shared).toString());
}

int ThemeCatalogManager::indexOf(const QString &id) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).id == id)
            return i;
    }
    return -1;
}

void ThemeCatalogManager::setError(const QString &message)
{
    if (m_error == message)
        return;
    m_error = message;
    emit errorChanged();
}

void ThemeCatalogManager::setLoading(bool value)
{
    if (m_loading == value)
        return;
    m_loading = value;
    emit loadingChanged();
}

void ThemeCatalogManager::updateRow(int row, const QList<int> &roles)
{
    if (row < 0 || row >= m_entries.size())
        return;
    const QModelIndex modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex, roles);
}

void ThemeCatalogManager::failEntry(int row, const QString &message)
{
    if (row < 0 || row >= m_entries.size())
        return;
    Entry &entry = m_entries[row];
    if (m_impl)
        m_impl->activeOperations.remove(entry.id);
    entry.state = QStringLiteral("error");
    entry.operationError = message;
    entry.progress = 0.0;
    updateRow(row, {StateRole, OperationErrorRole, ProgressRole});
}

bool ThemeCatalogManager::validId(const QString &id)
{
    static const QRegularExpression pattern(QStringLiteral("^[a-z0-9][a-z0-9_.-]{1,127}$"));
    return pattern.match(id).hasMatch();
}

bool ThemeCatalogManager::versionAtLeast(const QString &current, const QString &minimum)
{
    if (minimum.trimmed().isEmpty())
        return true;
    const QVersionNumber currentVersion = QVersionNumber::fromString(current);
    const QVersionNumber minimumVersion = QVersionNumber::fromString(minimum);
    if (currentVersion.isNull() || minimumVersion.isNull())
        return true;
    return QVersionNumber::compare(currentVersion, minimumVersion) >= 0;
}

bool ThemeCatalogManager::versionGreater(const QString &candidate, const QString &installed)
{
    const QVersionNumber candidateVersion = QVersionNumber::fromString(candidate);
    const QVersionNumber installedVersion = QVersionNumber::fromString(installed);
    if (candidateVersion.isNull() || installedVersion.isNull())
        return !candidate.isEmpty() && candidate != installed;
    return QVersionNumber::compare(candidateVersion, installedVersion) > 0;
}

void ThemeCatalogManager::refresh()
{
    if (m_loading || (m_impl && !m_impl->activeOperations.isEmpty()))
        return;
    setError(QString{});
    const QUrl catalogUrl(apiBaseUrl() + QStringLiteral("/themes?limit=100"));
    if (!catalogUrl.isValid() || catalogUrl.scheme() != QStringLiteral("https") || catalogUrl.host().isEmpty()) {
        setError(QStringLiteral("Theme catalog requires a valid HTTPS API base URL."));
        return;
    }
    setLoading(true);
    QNetworkRequest request(catalogUrl);
    request.setRawHeader("Accept", "application/json");
    request.setTransferTimeout(15000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader,
        QStringLiteral("LeoMiniGames/%1").arg(QCoreApplication::applicationVersion()));
    QNetworkReply *reply = m_impl->network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)> guard(reply, [](QNetworkReply *r) { r->deleteLater(); });
        setLoading(false);
        if (reply->error() != QNetworkReply::NoError) {
            setError(QStringLiteral("Theme catalog request failed: %1").arg(reply->errorString()));
            return;
        }
        const QByteArray payload = reply->readAll();
        if (payload.size() > kMaxThemeJsonBytes) { setError(QStringLiteral("Theme catalog response exceeds the 2 MiB safety limit.")); return; }
        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            setError(QStringLiteral("Theme catalog returned invalid JSON."));
            return;
        }
        QList<Entry> fresh;
        const QJsonArray rows = document.object().value(QStringLiteral("data")).toArray();
        fresh.reserve(rows.size());
        for (const QJsonValue &value : rows) {
            const QJsonObject object = value.toObject();
            Entry entry;
            entry.id = object.value(QStringLiteral("id")).toString();
            if (!validId(entry.id))
                continue;
            entry.name = object.value(QStringLiteral("name")).toString(entry.id);
            entry.description = object.value(QStringLiteral("description")).toString();
            const bool authoritativeCatalog = isOfficialCatalogOrigin(apiBaseUrl());
            const QJsonObject publisherObject = object.value(QStringLiteral("publisher")).toObject();
            entry.publisher = !publisherObject.isEmpty() ? publisherObject.value(QStringLiteral("display_name")).toString(QStringLiteral("Unknown")) : object.value(QStringLiteral("publisher")).toString(QStringLiteral("Unknown"));
            const PublisherTrustDecision publisherTrust = resolvePublisherTrust(object, authoritativeCatalog, true);
            const QString publisherType = publisherTrust.type;
            entry.publisherVerification = publisherType;
            entry.publisherVerified = publisherTrust.verified;
            entry.publisherStatus = !authoritativeCatalog ? QStringLiteral("Third-party Catalog")
                : publisherType == QStringLiteral("official") ? QStringLiteral("Official Publisher")
                : publisherType == QStringLiteral("verified") ? QStringLiteral("Verified Publisher")
                : QStringLiteral("Unverified Publisher");
            entry.version = object.value(QStringLiteral("version")).toString();
            entry.category = object.value(QStringLiteral("category")).toString(QStringLiteral("Theme"));
            for (const QJsonValue &tag : object.value(QStringLiteral("tags")).toArray())
                entry.tags.append(tag.toString());
            entry.iconUrl = object.value(QStringLiteral("icon_url")).toString();
            for (const QJsonValue &preview : object.value(QStringLiteral("preview_urls")).toArray())
                entry.previewUrls.append(preview.toString());
            entry.sizeBytes = object.value(QStringLiteral("size_bytes")).toVariant().toLongLong();
            entry.downloads = object.value(QStringLiteral("downloads")).toVariant().toLongLong();
            entry.updatedAt = object.value(QStringLiteral("updated_at")).toString();
            entry.minAppVersion = object.value(QStringLiteral("min_app_version")).toString();
            entry.packageFormat = object.value(QStringLiteral("package_format")).toString(QStringLiteral("theme-rcc-v1"));
            entry.themeApiVersion = qMax(1, object.value(QStringLiteral("theme_api_version")).toInt(1));
            entry.sha256 = object.value(QStringLiteral("sha256")).toString().toLower();
            entry.downloadEndpoint = object.value(QStringLiteral("download_endpoint")).toString();
            entry.downloadUrl = object.value(QStringLiteral("download_url")).toString();
            fresh.append(entry);
        }
        beginResetModel();
        m_entries = fresh;
        endResetModel();
        emit countChanged();
        setError(QString{});
    });
}

void ThemeCatalogManager::install(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;
    Entry &entry = m_entries[row];
    if (entry.state == QStringLiteral("resolving") || entry.state == QStringLiteral("downloading") || entry.state == QStringLiteral("installing"))
        return;
    if (entry.packageFormat != QStringLiteral("theme-rcc-v1")) {
        failEntry(row, QStringLiteral("Unsupported theme package format."));
        return;
    }
    if (!versionAtLeast(QCoreApplication::applicationVersion(), entry.minAppVersion)) {
        failEntry(row, QStringLiteral("This theme requires LeoMiniGames %1 or newer.").arg(entry.minAppVersion));
        return;
    }
    if (entry.sizeBytes <= 0 || entry.sizeBytes > kMaxThemeDownloadBytes) {
        failEntry(row, QStringLiteral("Theme size is invalid or exceeds the client safety limit."));
        return;
    }
    if (!QRegularExpression(QStringLiteral("^[a-f0-9]{64}$")).match(entry.sha256).hasMatch()) {
        failEntry(row, QStringLiteral("Catalog SHA-256 is invalid."));
        return;
    }
    if (m_impl->activeOperations.contains(entry.id))
        return;
    m_impl->activeOperations.insert(entry.id);
    entry.state = QStringLiteral("resolving");
    entry.progress = 0.0;
    entry.operationError.clear();
    updateRow(row, {StateRole, ProgressRole, OperationErrorRole});

    const QUrl direct(entry.downloadUrl);
    if (direct.isValid() && direct.scheme() == QStringLiteral("https") && !direct.host().isEmpty()) {
        startDownload(row, direct, entry.sha256, entry.sizeBytes);
        return;
    }
    requestDownloadTicket(row);
}

void ThemeCatalogManager::requestDownloadTicket(int row)
{
    if (row < 0 || row >= m_entries.size())
        return;
    const QString entryId = m_entries.at(row).id;
    const QString endpoint = m_entries.at(row).downloadEndpoint.isEmpty()
        ? QStringLiteral("/api/v1/themes/%1/download").arg(entryId)
        : m_entries.at(row).downloadEndpoint;
    const QUrl baseUrl(apiBaseUrl());
    const QUrl url = resolveEndpoint(apiBaseUrl(), endpoint);
    if (!url.isValid() || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0 || url.host().isEmpty() ||
        url.scheme().compare(baseUrl.scheme(), Qt::CaseInsensitive) != 0 ||
        url.host().compare(baseUrl.host(), Qt::CaseInsensitive) != 0 || url.port(-1) != baseUrl.port(-1)) {
        failEntry(row, QStringLiteral("Theme download ticket endpoint must remain on the configured catalog origin."));
        return;
    }
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setTransferTimeout(15000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
    QNetworkReply *reply = m_impl->network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, entryId] {
        std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)> guard(reply, [](QNetworkReply *r) { r->deleteLater(); });
        const int currentRow = indexOf(entryId);
        if (currentRow < 0) {
            m_impl->activeOperations.remove(entryId);
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            failEntry(currentRow, QStringLiteral("Theme download ticket failed: %1").arg(reply->errorString()));
            return;
        }
        const QByteArray payload = reply->readAll();
        if (payload.size() > 512 * 1024) { failEntry(currentRow, QStringLiteral("Theme download ticket response is too large.")); return; }
        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            failEntry(currentRow, QStringLiteral("Theme download ticket returned invalid JSON."));
            return;
        }
        const QJsonObject data = document.object().value(QStringLiteral("data")).toObject();
        const QUrl downloadUrl(data.value(QStringLiteral("url")).toString());
        const QString sha = data.value(QStringLiteral("sha256")).toString().toLower();
        const qint64 size = data.value(QStringLiteral("size_bytes")).toVariant().toLongLong();
        const Entry &entry = m_entries.at(currentRow);
        if (sha.compare(entry.sha256, Qt::CaseInsensitive) != 0 || size != entry.sizeBytes) {
            failEntry(currentRow, QStringLiteral("Theme download ticket metadata does not match the catalog."));
            return;
        }
        if (!downloadUrl.isValid() || downloadUrl.scheme() != QStringLiteral("https") || downloadUrl.host().isEmpty()) {
            failEntry(currentRow, QStringLiteral("Theme downloads require a valid HTTPS URL."));
            return;
        }
        startDownload(currentRow, downloadUrl, sha, size);
    });
}

void ThemeCatalogManager::startDownload(int row, const QUrl &url, const QString &expectedSha, qint64 expectedSize)
{
    if (row < 0 || row >= m_entries.size() || !m_paths || !m_themes)
        return;
    Entry &entry = m_entries[row];
    auto job = std::make_shared<Impl::DownloadJob>();
    job->row = row;
    job->id = entry.id;
    job->expectedSha = expectedSha;
    job->expectedSize = expectedSize;
    job->partPath = m_paths->cache() + QStringLiteral("/theme-") + entry.id + QStringLiteral(".rcc.part");
    QFile::remove(job->partPath);
    job->file.setFileName(job->partPath);
    if (!job->file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        failEntry(row, QStringLiteral("Could not open theme download file."));
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
        if (!job)
            return;
        const int currentRow = indexOf(job->id);
        if (currentRow < 0)
            return;
        const qint64 denominator = job->expectedSize > 0 ? job->expectedSize : total;
        if (denominator > 0) {
            m_entries[currentRow].progress = qBound<qreal>(0.0, static_cast<qreal>(received) / denominator, 1.0);
            updateRow(currentRow, {ProgressRole});
        }
    });

    connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
        const auto job = m_impl->downloads.value(reply);
        if (!job || !reply->isOpen() || !reply->isReadable())
            return;
        const QByteArray chunk = reply->readAll();
        job->received += chunk.size();
        if (job->received > kMaxThemeDownloadBytes) {
            reply->abort();
            return;
        }
        job->hash.addData(chunk);
        if (job->file.write(chunk) != chunk.size())
            reply->abort();
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
            job->received += chunk.size();
            if (job->received > kMaxThemeDownloadBytes) {
                finalWriteOk = false;
            } else {
                job->hash.addData(chunk);
                finalWriteOk = job->file.write(chunk) == chunk.size();
            }
        }
        job->file.close();
        const auto networkError = reply->error();
        const QString networkMessage = reply->errorString();
        reply->deleteLater();

        const int currentRow = indexOf(job->id);
        if (currentRow < 0) {
            m_impl->activeOperations.remove(job->id);
            QFile::remove(job->partPath);
            return;
        }
        if (!finalWriteOk || networkError != QNetworkReply::NoError || job->received > kMaxThemeDownloadBytes) {
            QFile::remove(job->partPath);
            failEntry(currentRow, QStringLiteral("Theme download failed: %1").arg(networkMessage));
            return;
        }
        const QString actualSha = QString::fromLatin1(job->hash.result().toHex());
        if (actualSha.compare(job->expectedSha, Qt::CaseInsensitive) != 0
            || (job->expectedSize > 0 && job->received != job->expectedSize)) {
            QFile::remove(job->partPath);
            failEntry(currentRow, QStringLiteral("Theme download integrity verification failed."));
            return;
        }

        Entry &entry = m_entries[currentRow];
        entry.state = QStringLiteral("installing");
        entry.progress = 1.0;
        updateRow(currentRow, {StateRole, ProgressRole});
        const bool installed = m_themes->installThemeRcc(job->partPath, job->expectedSha);
        QFile::remove(job->partPath);
        if (!installed) {
            failEntry(currentRow, m_themes->lastError().isEmpty()
                                  ? QStringLiteral("Theme installation failed.")
                                  : m_themes->lastError());
            return;
        }
        m_impl->activeOperations.remove(job->id);
        entry.state = QStringLiteral("idle");
        entry.operationError.clear();
        entry.progress = 1.0;
        updateRow(currentRow, {InstalledRole, InstalledVersionRole, UpdateAvailableRole, ActiveRole,
                               StateRole, ProgressRole, OperationErrorRole});
        emit themeInstalled(entry.id);
    });
}

void ThemeCatalogManager::remove(const QString &id)
{
    if (!m_themes || !m_themes->isInstalled(id))
        return;
    if (!m_themes->removeTheme(id)) {
        const int row = indexOf(id);
        if (row >= 0)
            failEntry(row, m_themes->lastError());
        return;
    }
    notifyInstallStateChanged();
    emit themeRemoved(id);
}

bool ThemeCatalogManager::isInstalled(const QString &id) const
{
    return m_themes && m_themes->isInstalled(id);
}

bool ThemeCatalogManager::updateAvailable(const QString &id) const
{
    const int row = indexOf(id);
    if (row < 0 || !m_themes || !m_themes->isInstalled(id))
        return false;
    return versionGreater(m_entries.at(row).version, m_themes->installedVersion(id));
}

QVariantMap ThemeCatalogManager::entryInfo(const QString &id) const
{
    const int row = indexOf(id);
    if (row < 0)
        return QVariantMap{};
    const QModelIndex modelIndex = index(row, 0);
    QVariantMap out;
    const auto roles = roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it)
        out.insert(QString::fromLatin1(it.value()), data(modelIndex, it.key()));
    return out;
}

void ThemeCatalogManager::notifyInstallStateChanged()
{
    if (m_entries.isEmpty())
        return;
    emit dataChanged(index(0, 0), index(m_entries.size() - 1, 0),
                     {InstalledRole, InstalledVersionRole, UpdateAvailableRole, ActiveRole, StateRole});
}
