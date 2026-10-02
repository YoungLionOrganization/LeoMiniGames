// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "UpdateService.h"
#include "NetworkSafety.h"
#include "SemVer.h"
#include "SettingsManager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#if defined(Q_OS_WIN)
#include <QProcess>
#endif
#include <QRegularExpression>
#include <QSysInfo>
#include <QTimer>
#include <QUrlQuery>

namespace {
constexpr qint64 kMaxUpdateMetadataBytes = 2 * 1024 * 1024;
constexpr qint64 kAutomaticCheckIntervalSeconds = 12 * 60 * 60;
const QUrl kUpdateGatewayUrl(QStringLiteral("https://leominigames.younglion.xyz/api/v1/updates/v1/check"));

using namespace LmgSemVer;

QString normalizedArchitecture()
{
    const QString arch = QSysInfo::currentCpuArchitecture().toLower();
    if (arch == QStringLiteral("arm64") || arch == QStringLiteral("aarch64"))
        return QStringLiteral("arm64");
    if (arch == QStringLiteral("x86_64") || arch == QStringLiteral("amd64")) {
#if defined(Q_OS_WIN) && defined(LMG_WINDOWS_X64_AVX2)
        return QStringLiteral("x86_64-avx2");
#else
        return QStringLiteral("x86_64");
#endif
    }
    if (arch == QStringLiteral("i386") || arch == QStringLiteral("i686") || arch == QStringLiteral("x86"))
        return QStringLiteral("x86");
    if (arch.contains(QStringLiteral("arm")))
        return QStringLiteral("armv7");
    return arch;
}

QString platformName()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#elif defined(Q_OS_ANDROID)
    return QStringLiteral("android");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("macos");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("linux");
#else
    return QStringLiteral("unsupported");
#endif
}
}

UpdateService::UpdateService(SettingsManager *settings, QObject *parent, QNetworkAccessManager *network)
    : QObject(parent), m_settings(settings), m_network(network ? network : new QNetworkAccessManager(this))
{
    const QString savedChannel = m_settings
        ? m_settings->value(QStringLiteral("updates/channel"), QStringLiteral("stable")).toString().trimmed().toLower()
        : QStringLiteral("stable");
    m_channel = savedChannel == QStringLiteral("preview") ? QStringLiteral("preview") : QStringLiteral("stable");
}

QString UpdateService::currentVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString UpdateService::maintenanceToolPath() const
{
#if defined(Q_OS_WIN)
    const QString candidate = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("LeoMiniGamesMaintenance.exe"));
#elif defined(Q_OS_MACOS)
    const QString candidate = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../Resources/LeoMiniGamesMaintenance.app/Contents/MacOS/LeoMiniGamesMaintenance"));
#else
    const QString candidate = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("LeoMiniGamesMaintenance"));
#endif
    const QFileInfo info(QDir::cleanPath(candidate));
    return info.isFile() && info.isExecutable() ? info.absoluteFilePath() : QString{};
}

bool UpdateService::maintenanceAvailable() const
{
    return !maintenanceToolPath().isEmpty();
}

QString UpdateService::installType() const
{
    return maintenanceAvailable() ? QStringLiteral("installer") : QStringLiteral("portable");
}

void UpdateService::setChannel(const QString &channel)
{
    const QString normalized = channel.trimmed().toLower();
    if (normalized != QStringLiteral("stable") && normalized != QStringLiteral("preview"))
        return;
    if (m_channel == normalized)
        return;
    m_channel = normalized;
    if (m_settings)
        m_settings->setValue(QStringLiteral("updates/channel"), m_channel);
    clearResult();
    emit channelChanged();
}

void UpdateService::clearResult()
{
    if (m_reply) {
        QNetworkReply *old = m_reply;
        m_reply = nullptr;
        old->disconnect(this);
        old->abort();
        old->deleteLater();
    }
    m_latestVersion.clear();
    m_releaseName.clear();
    m_releaseNotes.clear();
    m_releaseUrl = QUrl{};
    m_downloadUrl = QUrl{};
    m_maintenanceRepositoryUrl = QUrl{};
    m_errorString.clear();
    m_status = QStringLiteral("idle");
    m_checking = false;
    m_updateAvailable = false;
    emit updateStateChanged();
}

void UpdateService::checkForUpdatesAutomatically()
{
    if (!m_settings || m_checking)
        return;
    const qint64 now = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
    const qint64 last = m_settings->value(QStringLiteral("updates/lastCheckEpoch"), 0).toLongLong();
    if (last > 0 && now >= last && now - last < kAutomaticCheckIntervalSeconds)
        return;
    checkForUpdates();
}

void UpdateService::checkForUpdates()
{
    if (m_checking)
        return;
    if (platformName() == QStringLiteral("unsupported")) {
        finishWithError(tr("Updates are not available for this platform."));
        return;
    }

    clearResult();
    m_errorString.clear();
    m_status = QStringLiteral("checking");
    m_checking = true;
    m_updateAvailable = false;
    emit updateStateChanged();

    QUrl endpoint = kUpdateGatewayUrl;
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("channel"), m_channel);
    query.addQueryItem(QStringLiteral("platform"), platformName());
    query.addQueryItem(QStringLiteral("arch"), normalizedArchitecture());
    query.addQueryItem(QStringLiteral("install"), installType());
    query.addQueryItem(QStringLiteral("current"), currentVersion());
    endpoint.setQuery(query);

    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("LeoMiniGames/%1 update-check").arg(currentVersion()));
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
    m_reply = m_network->get(request);
    NetworkSafety::boundJsonReply(m_reply, kMaxUpdateMetadataBytes);

    QTimer *timeout = new QTimer(m_reply);
    timeout->setSingleShot(true);
    timeout->setInterval(15000);
    connect(timeout, &QTimer::timeout, m_reply, &QNetworkReply::abort);
    timeout->start();
    connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64) {
        if (m_reply && received > kMaxUpdateMetadataBytes)
            m_reply->abort();
    });
    QNetworkReply *requestReply = m_reply;
    connect(m_reply, &QNetworkReply::finished, this, [this, requestReply] {
        if (m_reply != requestReply) { requestReply->deleteLater(); return; }
        QNetworkReply *reply = requestReply;
        m_reply = nullptr;
        if (!reply)
            return;

        const QByteArray payload = reply->readAll();
        const auto networkError = reply->error();
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QUrl finalUrl = reply->url();
        reply->deleteLater();

        m_checking = false;
        if (!isAllowedUpdateUrl(finalUrl)) {
            finishWithError(tr("The update request was redirected to an untrusted host."));
            return;
        }
        if (networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
            finishWithError(networkError == QNetworkReply::OperationCanceledError
                                ? tr("Update metadata was too large or the request was cancelled.")
                                : tr("Could not check the LeoMiniGames update service."));
            return;
        }
        if (payload.size() > kMaxUpdateMetadataBytes) {
            finishWithError(tr("Update metadata exceeded the safety limit."));
            return;
        }
        parseGatewayResponse(payload);
    });
}

void UpdateService::parseGatewayResponse(const QByteArray &payload)
{
    QJsonParseError error{};
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        finishWithError(tr("The update service returned invalid metadata."));
        return;
    }

    const QJsonObject root = document.object();
    if (!root.value(QStringLiteral("ok")).toBool()) {
        finishWithError(tr("The update service could not resolve an update."));
        return;
    }
    const QJsonObject data = root.value(QStringLiteral("data")).toObject();
    if (data.value(QStringLiteral("schema")).toInt() != 1
        || data.value(QStringLiteral("provider_contract")).toString() != QStringLiteral("leominigames-update-v1")) {
        finishWithError(tr("The update service returned an unsupported metadata contract."));
        return;
    }

    const QJsonObject release = data.value(QStringLiteral("release")).toObject();
    const QString remoteVersionText = release.value(QStringLiteral("version")).toString().trimmed();
    const SemVer localVersion = parseSemVer(currentVersion());
    const SemVer remoteVersion = parseSemVer(remoteVersionText);
    const QUrl releaseUrl(release.value(QStringLiteral("page_url")).toString());
    const QJsonObject download = data.value(QStringLiteral("download")).toObject();
    const QUrl downloadUrl(download.value(QStringLiteral("url")).toString());
    const QUrl maintenanceUrl(data.value(QStringLiteral("maintenance_repository_url")).toString());

    if (!localVersion.valid || !remoteVersion.valid || !isAllowedUpdateUrl(releaseUrl)) {
        finishWithError(tr("The release metadata failed validation."));
        return;
    }
    if (downloadUrl.isValid() && !downloadUrl.isEmpty() && !isAllowedUpdateUrl(downloadUrl)) {
        finishWithError(tr("The update download URL failed validation."));
        return;
    }
    if (maintenanceUrl.isValid() && !maintenanceUrl.isEmpty() && !isAllowedUpdateUrl(maintenanceUrl)) {
        finishWithError(tr("The maintenance repository URL failed validation."));
        return;
    }

    if (m_channel == QStringLiteral("stable") && !remoteVersion.prerelease.isEmpty()) {
        finishWithError(tr("Stable update metadata contains a prerelease.")); return;
    }
    if (m_settings) m_settings->setValue(QStringLiteral("updates/lastCheckEpoch"), QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
    m_latestVersion = remoteVersionText;
    m_releaseName = release.value(QStringLiteral("name")).toString().trimmed();
    if (m_releaseName.isEmpty())
        m_releaseName = QStringLiteral("LeoMiniGames v%1").arg(remoteVersionText);
    m_releaseNotes = release.value(QStringLiteral("notes")).toString();
    m_releaseUrl = releaseUrl;
    m_downloadUrl = downloadUrl;
    m_maintenanceRepositoryUrl = maintenanceUrl;
    m_updateAvailable = compareSemVer(remoteVersion, localVersion) > 0;
    m_errorString.clear();
    m_status = m_updateAvailable ? QStringLiteral("available") : QStringLiteral("up-to-date");
    emit updateStateChanged();
}

QUrl UpdateService::maintenanceRepositoryUrl() const
{
#if defined(Q_OS_WIN)
    return m_maintenanceRepositoryUrl;
#else
    return QUrl{};
#endif
}

bool UpdateService::launchMaintenance()
{
#if defined(Q_OS_WIN)
    const QString tool = maintenanceToolPath();
    if (tool.isEmpty())
        return false;
    const QUrl repository = maintenanceRepositoryUrl();
    if (!isAllowedUpdateUrl(repository))
        return false;

    const QStringList arguments{
        QStringLiteral("--set-temp-repository"), repository.toString(),
        QStringLiteral("--start-updater")
    };
    return QProcess::startDetached(tool, arguments);
#else
    return false;
#endif
}

bool UpdateService::openUpdate()
{
    if (maintenanceAvailable() && launchMaintenance())
        return true;
    if (m_downloadUrl.isValid() && isAllowedUpdateUrl(m_downloadUrl))
        return QDesktopServices::openUrl(m_downloadUrl);
    return openReleasePage();
}

bool UpdateService::openReleasePage()
{
    return m_releaseUrl.isValid() && isAllowedUpdateUrl(m_releaseUrl)
        ? QDesktopServices::openUrl(m_releaseUrl) : false;
}

void UpdateService::finishWithError(const QString &message)
{
    m_status = QStringLiteral("error");
    m_errorString = message;
    m_checking = false;
    m_updateAvailable = false;
    emit updateStateChanged();
}

bool UpdateService::isAllowedUpdateUrl(const QUrl &url)
{
    if (!url.isValid() || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0)
        return false;
    if (url.port(443) != 443 || !url.userInfo().isEmpty() || url.host().compare(QStringLiteral("leominigames.younglion.xyz"), Qt::CaseInsensitive) != 0)
        return false;
    const QString path = url.path();
    return path.startsWith(QStringLiteral("/api/v1/updates/"))
        || path.startsWith(QStringLiteral("/updates/"));
}
