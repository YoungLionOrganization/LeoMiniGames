// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <vector>

class ModManager;
class PluginManager;

class GameRegistry final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        LocalizedNameRole,
        DescriptionRole,
        CategoryRole,
        VersionRole,
        PublisherRole,
        IconRole,
        IconUrlRole,
        BuiltInRole,
        SourceRole,
        TagsRole,
        PublisherStatusRole,
        VerifiedRole,
        PluginLevelRole,
        NativeRole,
        LicenseRole,
        SourceUrlRole,
        SourceAvailableRole,
        SaveVersionRole
    };
    Q_ENUM(Roles)

    explicit GameRegistry(PluginManager *plugins, ModManager *mods, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString query() const;
    int count() const;
    int totalCount() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString nameFor(const QString &id) const;
    Q_INVOKABLE QString descriptionFor(const QString &id) const;
    Q_INVOKABLE QString versionFor(const QString &id) const;
    Q_INVOKABLE QString sourceFor(const QString &id) const;
    Q_INVOKABLE int saveVersionFor(const QString &id) const;
    Q_INVOKABLE QUrl entryUrl(const QString &id) const;
    QObject *createGame(const QString &id, QObject *parent) const;

public slots:
    void setQuery(const QString &query);

signals:
    void queryChanged();
    void countChanged();
    void totalCountChanged();
    void refreshed();

private:
    struct Entry {
        QString id;
        QString name;
        QString localizedName;
        QString description;
        QString category;
        QString version;
        QString publisher;
        QString icon;
        QString iconUrl;
        QString source;
        QStringList tags;
        QString publisherStatus;
        QString license;
        QString sourceUrl;
        bool sourceAvailable = false;
        bool builtIn = false;
        bool verified = false;
        int pluginLevel = 1;
        bool native = false;
        int saveVersion = 1;
    };

    const Entry *find(const QString &id) const;
    bool matches(const Entry &entry) const;
    void rebuildVisible();

    PluginManager *m_plugins = nullptr;
    ModManager *m_mods = nullptr;
    std::vector<Entry> m_all;
    std::vector<int> m_visible;
    QString m_query;
};
