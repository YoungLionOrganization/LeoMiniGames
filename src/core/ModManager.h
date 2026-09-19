// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <memory>

class AppPaths;
class SettingsManager;

class ModManager final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString apiBaseUrl READ apiBaseUrl NOTIFY apiBaseUrlChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool tlsAvailable READ tlsAvailable CONSTANT)
    Q_PROPERTY(QString tlsBackend READ tlsBackend CONSTANT)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1, NameRole, LocalizedNameRole, DescriptionRole, AuthorRole, VersionRole,
        CategoryRole, IconRole, IconUrlRole, SizeRole, DownloadsRole, InstalledRole, InstalledVersionRole,
        StateRole, ProgressRole, ErrorRole, MinAppVersionRole, TagsRole, PublisherStatusRole,
        PublisherVerifiedRole, PublisherVerificationRole, LicenseRole, LicenseFileRole, SourceUrlRole, SourceAvailableRole,
        PluginLevelRole, NativeRole, ReviewedRole, SaveVersionRole, DefaultLocaleRole, LocalesRole,
        SettingsSchemaRole, UpdatedAtRole, InCatalogRole
    };
    Q_ENUM(Roles)

    explicit ModManager(AppPaths *paths, SettingsManager *settings, QObject *parent = nullptr);
    ~ModManager() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loading() const; QString error() const; QString apiBaseUrl() const; int count() const;
    bool tlsAvailable() const; QString tlsBackend() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void install(const QString &id);
    Q_INVOKABLE void uninstall(const QString &id);
    Q_INVOKABLE bool isInstalled(const QString &id) const;
    Q_INVOKABLE QString installedVersion(const QString &id) const;
    Q_INVOKABLE QUrl entryUrl(const QString &id) const;
    Q_INVOKABLE QUrl licenseUrl(const QString &id) const;
    Q_INVOKABLE QUrl sourceUrlFor(const QString &id) const;
    Q_INVOKABLE QString defaultLocaleFor(const QString &id) const;
    Q_INVOKABLE QStringList localesFor(const QString &id) const;
    Q_INVOKABLE int saveVersionFor(const QString &id) const;
    Q_INVOKABLE QVariantMap settingsSchemaFor(const QString &id) const;
    Q_INVOKABLE bool networkAllowedFor(const QString &id) const;

signals:
    void loadingChanged(); void errorChanged(); void apiBaseUrlChanged(); void countChanged();
    void modInstalled(const QString &id); void modUninstalled(const QString &id);

private:
    struct Entry {
        QString id, name, localizedName, description, author, version, category, icon, iconUrl, sha256;
        QString packageFormat = QStringLiteral("rcc-v1");
        QString entryPath = QStringLiteral("Main.qml");
        QString minAppVersion, downloadEndpoint, downloadUrl, updatedAt;
        qint64 sizeBytes = 0, downloads = 0;
        bool inCatalog = false;

        QStringList tags;
        QString apiVersion;
        QStringList capabilities;
        QString publisherStatus = QStringLiteral("Unverified Publisher");
        bool publisherVerified = false;
        QString publisherVerification = QStringLiteral("unverified");
        QString license, licenseFile, sourceUrl;
        bool sourceAvailable = false;
        int pluginLevel = 1;
        bool native = false;
        bool reviewed = false;
        int saveVersion = 1;
        QString defaultLocale = QStringLiteral("en");
        QStringList locales;
        QVariantMap settingsSchema;

        bool installed = false;
        QString installedVersion, installedFile, installedEntry, installedMountRoot;
        QString state = QStringLiteral("idle");
        qreal progress = 0.0;
        QString operationError;
    };

    struct Impl;
    int indexOf(const QString &id) const;
    void setError(const QString &message); void setLoading(bool loading); void loadInstalled(); void saveInstalled() const;
    void registerInstalled(Entry &entry); void applyManifestMetadata(Entry &entry);
    void updateRow(int row, const QList<int> &roles = QList<int>{}); void failEntry(int row, const QString &message);
    void requestDownloadTicket(int row); void startPackageDownload(int row, const QUrl &url, const QString &expectedSha, qint64 expectedSize);
    static bool validId(const QString &id); static bool safeEntryPath(const QString &path); static bool versionAtLeast(const QString &current, const QString &minimum);

    AppPaths *m_paths = nullptr; SettingsManager *m_settings = nullptr; std::unique_ptr<Impl> m_impl;
    QList<Entry> m_entries; bool m_loading = false; QString m_error;
};
