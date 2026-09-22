// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "UpdateService.h"
#include "SettingsManager.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
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

namespace {
constexpr qint64 kMaxUpdateMetadataBytes = 2 * 1024 * 1024;
constexpr qint64 kAutomaticCheckIntervalSeconds = 12 * 60 * 60;
const QUrl kLatestStableReleaseUrl(QStringLiteral("https://api.github.com/repos/YoungLionOrganization/LeoMiniGames/releases/latest"));
const QUrl kReleaseListUrl(QStringLiteral("https://api.github.com/repos/YoungLionOrganization/LeoMiniGames/releases?per_page=20"));

struct SemVer {
    int major = 0;
    int minor = 0;
    int patch = 0;
    QStringList prerelease;
    bool valid = false;
};

SemVer parseSemVer(QString value)
{
    static const QRegularExpression re(QStringLiteral(
        R"(^[vV]?(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-([0-9A-Za-z.-]+))?(?:\+[0-9A-Za-z.-]+)?$)"));
    value = value.trimmed();
    const QRegularExpressionMatch match = re.match(value);
    if (!match.hasMatch())
        return SemVer{};

    SemVer out;
    out.major = match.captured(1).toInt();
    out.minor = match.captured(2).toInt();
    out.patch = match.captured(3).toInt();
    if (!match.captured(4).isEmpty())
        out.prerelease = match.captured(4).split(QLatin1Char('.'));
    out.valid = true;
    return out;
}

int compareIdentifier(const QString &left, const QString &right)
{
    bool leftNumeric = false;
    bool rightNumeric = false;
    const qulonglong leftValue = left.toULongLong(&leftNumeric);
    const qulonglong rightValue = right.toULongLong(&rightNumeric);
    if (leftNumeric && rightNumeric)
        return leftValue == rightValue ? 0 : (leftValue < rightValue ? -1 : 1);
    if (leftNumeric != rightNumeric)
        return leftNumeric ? -1 : 1;
    const int result = QString::compare(left, right, Qt::CaseSensitive);
    return result < 0 ? -1 : (result > 0 ? 1 : 0);
}

int compareSemVer(const SemVer &left, const SemVer &right)
{
    if (left.major != right.major)
        return left.major < right.major ? -1 : 1;
    if (left.minor != right.minor)
        return left.minor < right.minor ? -1 : 1;
    if (left.patch != right.patch)
        return left.patch < right.patch ? -1 : 1;

    if (left.prerelease.isEmpty() && right.prerelease.isEmpty())
        return 0;
    if (left.prerelease.isEmpty())
        return 1;
    if (right.prerelease.isEmpty())
        return -1;

    const qsizetype common = qMin(left.prerelease.size(), right.prerelease.size());
    for (qsizetype i = 0; i < common; ++i) {
        const int result = compareIdentifier(left.prerelease.at(i), right.prerelease.at(i));
        if (result != 0)
            return result;
    }
    if (left.prerelease.size() == right.prerelease.size())
        return 0;
    return left.prerelease.size() < right.prerelease.size() ? -1 : 1;
}

QString normalizedArchitecture()
{
    const QString arch = QSysInfo::currentCpuArchitecture().toLower();
    if (arch == QStringLiteral("arm64") || arch == QStringLiteral("aarch64"))
        return QStringLiteral("arm64");
    if (arch == QStringLiteral("x86_64") || arch == QStringLiteral("amd64"))
        return QStringLiteral("x86_64");
    if (arch == QStringLiteral("i386") || arch == QStringLiteral("i686") || arch == QStringLiteral("x86"))
        return QStringLiteral("x86");
    if (arch.contains(QStringLiteral("arm")))
        return QStringLiteral("armeabi-v7a");
    return arch;
}

QString releaseTagVersion(const QJsonObject &release)
{
    QString tag = release.value(QStringLiteral("tag_name")).toString().trimmed();
    if (tag.startsWith(QLatin1Char('v'), Qt::CaseInsensitive))
        tag.remove(0, 1);
    return tag;
}
}

UpdateService::UpdateService(SettingsManager *settings, QObject *parent)
    : QObject(parent), m_settings(settings), m_network(new QNetworkAccessManager(this))
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
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_latestVersion.clear();
    m_releaseName.clear();
    m_releaseNotes.clear();
    m_releaseUrl = QUrl{};
    m_downloadUrl = QUrl{};
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
    m_settings->setValue(QStringLiteral("updates/lastCheckEpoch"), now);
    checkForUpdates();
}

void UpdateService::checkForUpdates()
{
    if (m_checking)
        return;

    m_errorString.clear();
    m_status = QStringLiteral("checking");
    m_checking = true;
    m_updateAvailable = false;
    emit updateStateChanged();

    const QUrl endpoint = m_channel == QStringLiteral("preview") ? kReleaseListUrl : kLatestStableReleaseUrl;
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("LeoMiniGames/%1 update-check").arg(currentVersion()));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_network->get(request);
    QTimer *timeout = new QTimer(m_reply);
    timeout->setSingleShot(true);
    timeout->setInterval(15000);
    connect(timeout, &QTimer::timeout, m_reply, &QNetworkReply::abort);
    timeout->start();
    connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64) {
        if (m_reply) {
            if (received > kMaxUpdateMetadataBytes)
                m_reply->abort();
        }
    });
    connect(m_reply, &QNetworkReply::finished, this, [this] {
        QNetworkReply *reply = m_reply;
        m_reply = nullptr;
        if (!reply)
            return;

        const QByteArray payload = reply->readAll();
        const auto networkError = reply->error();
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QUrl finalUrl = reply->url();
        reply->deleteLater();

        m_checking = false;
        if (!isAllowedGitHubUrl(finalUrl)) {
            finishWithError(tr("The update request was redirected to an untrusted host."));
            return;
        }
        if (networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
            finishWithError(networkError == QNetworkReply::OperationCanceledError
                                ? tr("Update metadata was too large or the request was cancelled.")
                                : tr("Could not check GitHub for updates."));
            return;
        }
        if (payload.size() > kMaxUpdateMetadataBytes) {
            finishWithError(tr("Update metadata exceeded the safety limit."));
            return;
        }

        if (m_channel == QStringLiteral("preview"))
            parseReleaseList(payload);
        else
            parseStableRelease(payload);
    });
}

void UpdateService::parseStableRelease(const QByteArray &payload)
{
    QJsonParseError error{};
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        finishWithError(tr("GitHub returned invalid update metadata."));
        return;
    }
    applyRelease(document.object());
}

void UpdateService::parseReleaseList(const QByteArray &payload)
{
    QJsonParseError error{};
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        finishWithError(tr("GitHub returned invalid update metadata."));
        return;
    }

    QJsonObject best;
    SemVer bestVersion;
    for (const QJsonValue &value : document.array()) {
        if (!value.isObject())
            continue;
        const QJsonObject release = value.toObject();
        if (release.value(QStringLiteral("draft")).toBool())
            continue;
        const SemVer candidate = parseSemVer(release.value(QStringLiteral("tag_name")).toString());
        if (!candidate.valid)
            continue;
        if (!bestVersion.valid || compareSemVer(candidate, bestVersion) > 0) {
            best = release;
            bestVersion = candidate;
        }
    }
    if (best.isEmpty()) {
        finishWithError(tr("No compatible GitHub release metadata was found."));
        return;
    }
    applyRelease(best);
}

void UpdateService::applyRelease(const QJsonObject &release)
{
    const QString remoteVersionText = releaseTagVersion(release);
    const SemVer localVersion = parseSemVer(currentVersion());
    const SemVer remoteVersion = parseSemVer(remoteVersionText);
    const QUrl releaseUrl(release.value(QStringLiteral("html_url")).toString());

    if (!localVersion.valid || !remoteVersion.valid || !isAllowedGitHubUrl(releaseUrl)) {
        finishWithError(tr("The release metadata failed validation."));
        return;
    }

    m_latestVersion = remoteVersionText;
    m_releaseName = release.value(QStringLiteral("name")).toString();
    if (m_releaseName.trimmed().isEmpty())
        m_releaseName = QStringLiteral("LeoMiniGames v%1").arg(remoteVersionText);
    m_releaseNotes = release.value(QStringLiteral("body")).toString();
    m_releaseUrl = releaseUrl;
    m_downloadUrl = selectDownloadUrl(release);
    m_updateAvailable = compareSemVer(remoteVersion, localVersion) > 0;
    m_errorString.clear();
    m_status = m_updateAvailable ? QStringLiteral("available") : QStringLiteral("up-to-date");
    emit updateStateChanged();
}

QUrl UpdateService::selectDownloadUrl(const QJsonObject &release) const
{
    const QJsonArray assets = release.value(QStringLiteral("assets")).toArray();
    const QString arch = normalizedArchitecture();
    QStringList preferredFragments;
#if defined(Q_OS_WIN)
    preferredFragments << QStringLiteral("Windows-%1.zip").arg(arch == QStringLiteral("arm64") ? QStringLiteral("ARM64") : arch);
#elif defined(Q_OS_ANDROID)
    preferredFragments << QStringLiteral("Android-universal.apk");
    if (arch == QStringLiteral("arm64"))
        preferredFragments << QStringLiteral("Android-arm64-v8a.apk");
    else if (arch == QStringLiteral("x86_64"))
        preferredFragments << QStringLiteral("Android-x86_64.apk");
#elif defined(Q_OS_MACOS)
    preferredFragments << QStringLiteral("macOS-universal.dmg") << QStringLiteral("macOS-%1.dmg").arg(arch);
#elif defined(Q_OS_LINUX)
    preferredFragments << QStringLiteral("Ubuntu-24.04-%1.AppImage").arg(arch)
                       << QStringLiteral("Linux-Ubuntu-24.04-%1.tar.gz").arg(arch);
#endif

    for (const QString &fragment : preferredFragments) {
        for (const QJsonValue &assetValue : assets) {
            const QJsonObject asset = assetValue.toObject();
            const QString name = asset.value(QStringLiteral("name")).toString();
            if (!name.endsWith(fragment, Qt::CaseInsensitive))
                continue;
            const QUrl url(asset.value(QStringLiteral("browser_download_url")).toString());
            if (isAllowedGitHubUrl(url))
                return url;
        }
    }
    return QUrl{};
}

QUrl UpdateService::maintenanceRepositoryUrl() const
{
#if !defined(Q_OS_WIN)
    return QUrl{};
#else
    const QString architecture = normalizedArchitecture() == QStringLiteral("arm64")
        ? QStringLiteral("ARM64") : QStringLiteral("x86_64");
    const QString track = m_channel == QStringLiteral("preview") ? QStringLiteral("preview") : QStringLiteral("stable");
    return QUrl(QStringLiteral("https://raw.githubusercontent.com/YoungLionOrganization/LeoMiniGames/updates/%1/windows/%2")
                    .arg(track, architecture));
#endif
}

bool UpdateService::launchMaintenance()
{
#if defined(Q_OS_WIN)
    const QString tool = maintenanceToolPath();
    if (tool.isEmpty())
        return false;
    const QUrl repository = maintenanceRepositoryUrl();
    if (!repository.isValid() || repository.scheme() != QStringLiteral("https"))
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
    if (m_downloadUrl.isValid() && isAllowedGitHubUrl(m_downloadUrl))
        return QDesktopServices::openUrl(m_downloadUrl);
    return openReleasePage();
}

bool UpdateService::openReleasePage()
{
    return m_releaseUrl.isValid() && isAllowedGitHubUrl(m_releaseUrl)
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

bool UpdateService::isAllowedGitHubUrl(const QUrl &url)
{
    if (!url.isValid() || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0)
        return false;
    const QString host = url.host().toLower();
    return host == QStringLiteral("github.com")
        || host == QStringLiteral("api.github.com")
        || host == QStringLiteral("objects.githubusercontent.com")
        || host == QStringLiteral("github-releases.githubusercontent.com")
        || host == QStringLiteral("raw.githubusercontent.com");
}
