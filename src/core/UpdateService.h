// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class SettingsManager;

class UpdateService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseName READ releaseName NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY updateStateChanged)
    Q_PROPERTY(QUrl releaseUrl READ releaseUrl NOTIFY updateStateChanged)
    Q_PROPERTY(QUrl downloadUrl READ downloadUrl NOTIFY updateStateChanged)
    Q_PROPERTY(QString channel READ channel WRITE setChannel NOTIFY channelChanged)
    Q_PROPERTY(QString status READ status NOTIFY updateStateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY updateStateChanged)
    Q_PROPERTY(bool checking READ checking NOTIFY updateStateChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateStateChanged)
    Q_PROPERTY(bool maintenanceAvailable READ maintenanceAvailable NOTIFY maintenanceAvailabilityChanged)
    Q_PROPERTY(QString installType READ installType NOTIFY maintenanceAvailabilityChanged)

public:
    explicit UpdateService(SettingsManager *settings, QObject *parent = nullptr);

    QString currentVersion() const;
    QString latestVersion() const { return m_latestVersion; }
    QString releaseName() const { return m_releaseName; }
    QString releaseNotes() const { return m_releaseNotes; }
    QUrl releaseUrl() const { return m_releaseUrl; }
    QUrl downloadUrl() const { return m_downloadUrl; }
    QString channel() const { return m_channel; }
    QString status() const { return m_status; }
    QString errorString() const { return m_errorString; }
    bool checking() const { return m_checking; }
    bool updateAvailable() const { return m_updateAvailable; }
    bool maintenanceAvailable() const;
    QString installType() const;

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void checkForUpdatesAutomatically();
    Q_INVOKABLE bool openUpdate();
    Q_INVOKABLE bool openReleasePage();
    Q_INVOKABLE bool launchMaintenance();
    Q_INVOKABLE void clearResult();

public slots:
    void setChannel(const QString &channel);

signals:
    void updateStateChanged();
    void channelChanged();
    void maintenanceAvailabilityChanged();

private:
    void finishWithError(const QString &message);
    void parseGatewayResponse(const QByteArray &payload);
    QString maintenanceToolPath() const;
    QUrl maintenanceRepositoryUrl() const;
    static bool isAllowedUpdateUrl(const QUrl &url);

    SettingsManager *m_settings = nullptr;
    QNetworkAccessManager *m_network = nullptr;
    QNetworkReply *m_reply = nullptr;
    QString m_channel = QStringLiteral("stable");
    QString m_latestVersion;
    QString m_releaseName;
    QString m_releaseNotes;
    QUrl m_releaseUrl;
    QUrl m_downloadUrl;
    QUrl m_maintenanceRepositoryUrl;
    QString m_status = QStringLiteral("idle");
    QString m_errorString;
    bool m_checking = false;
    bool m_updateAvailable = false;
};
