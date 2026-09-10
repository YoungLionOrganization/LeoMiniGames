// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QUrl>
#include <memory>

class AppPaths;
class SettingsManager;
class ThemeManager;

class ThemeCatalogManager final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString apiBaseUrl READ apiBaseUrl NOTIFY apiBaseUrlChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        PublisherRole,
        PublisherStatusRole,
        PublisherVerifiedRole,
        PublisherVerificationRole,
        VersionRole,
        CategoryRole,
        TagsRole,
        IconUrlRole,
        PreviewUrlsRole,
        SizeRole,
        DownloadsRole,
        UpdatedAtRole,
        MinAppVersionRole,
        PackageFormatRole,
        ThemeApiVersionRole,
        InstalledRole,
        InstalledVersionRole,
        UpdateAvailableRole,
        ActiveRole,
        StateRole,
        ProgressRole,
        OperationErrorRole
    };
    Q_ENUM(Roles)

    explicit ThemeCatalogManager(AppPaths *paths, SettingsManager *settings, ThemeManager *themes, QObject *parent = nullptr);
    ~ThemeCatalogManager() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loading() const;
    QString error() const;
    QString apiBaseUrl() const;
    int count() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void install(const QString &id);
    Q_INVOKABLE void remove(const QString &id);
    Q_INVOKABLE bool isInstalled(const QString &id) const;
    Q_INVOKABLE bool updateAvailable(const QString &id) const;
    Q_INVOKABLE QVariantMap entryInfo(const QString &id) const;

signals:
    void loadingChanged();
    void errorChanged();
    void apiBaseUrlChanged();
    void countChanged();
    void themeInstalled(const QString &id);
    void themeRemoved(const QString &id);

private:
    struct Entry {
        QString id;
        QString name;
        QString description;
        QString publisher;
        QString publisherStatus;
        bool publisherVerified = false;
        QString publisherVerification = QStringLiteral("unverified");
        QString version;
        QString category;
        QStringList tags;
        QString iconUrl;
        QVariantList previewUrls;
        qint64 sizeBytes = 0;
        qint64 downloads = 0;
        QString updatedAt;
        QString minAppVersion;
        QString packageFormat = QStringLiteral("theme-rcc-v1");
        int themeApiVersion = 1;
        QString sha256;
        QString downloadEndpoint;
        QString downloadUrl;
        QString state = QStringLiteral("idle");
        qreal progress = 0.0;
        QString operationError;
    };

    struct Impl;
    int indexOf(const QString &id) const;
    void setError(const QString &message);
    void setLoading(bool value);
    void updateRow(int row, const QList<int> &roles = QList<int>{});
    void failEntry(int row, const QString &message);
    void requestDownloadTicket(int row);
    void startDownload(int row, const QUrl &url, const QString &expectedSha, qint64 expectedSize);
    void notifyInstallStateChanged();
    static bool validId(const QString &id);
    static bool versionAtLeast(const QString &current, const QString &minimum);
    static bool versionGreater(const QString &candidate, const QString &installed);

    AppPaths *m_paths = nullptr;
    SettingsManager *m_settings = nullptr;
    ThemeManager *m_themes = nullptr;
    std::unique_ptr<Impl> m_impl;
    QList<Entry> m_entries;
    bool m_loading = false;
    QString m_error;
};
