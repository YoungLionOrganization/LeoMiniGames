// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonObject>
#include <QPluginLoader>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include <memory>
#include <vector>

#include "sdk/IGamePlugin.h"

class AppPaths;

class PluginManager final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList errors READ errors NOTIFY errorsChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        CategoryRole,
        VersionRole,
        AuthorRole,
        IconRole,
        BuiltInRole
    };
    Q_ENUM(Roles)

    explicit PluginManager(AppPaths *paths, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList errors() const;
    int count() const;
    Q_INVOKABLE QString nameFor(const QString &id) const;
    Q_INVOKABLE QString descriptionFor(const QString &id) const;
    Q_INVOKABLE void reload();
    QObject *createGame(const QString &id, QObject *parent);
    QUrl entryUrl(const QString &id) const;

signals:
    void errorsChanged();
    void countChanged();

private:
    struct Entry {
        IGamePlugin *plugin = nullptr;
        GamePluginInfo info;
        bool builtIn = false;
    };

    void registerPlugin(QObject *object, bool builtIn, const QJsonObject &loaderMetadata = QJsonObject{});
    void addError(const QString &message);

    AppPaths *m_paths = nullptr;
    std::vector<Entry> m_entries;
    std::vector<std::unique_ptr<QPluginLoader>> m_dynamicLoaders;
    QStringList m_errors;
};
