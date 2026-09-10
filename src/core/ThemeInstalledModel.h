// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>

class ThemeManager;
class ThemeCatalogManager;

class ThemeInstalledModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        PublisherRole,
        PublisherStatusRole,
        PublisherVerifiedRole,
        VersionRole,
        InstalledVersionRole,
        CategoryRole,
        TagsRole,
        IconUrlRole,
        PreviewUrlsRole,
        SizeRole,
        DownloadsRole,
        UpdatedAtRole,
        StateRole,
        ProgressRole,
        OperationErrorRole,
        InstalledRole,
        UpdateAvailableRole,
        ActiveRole,
        BuiltInRole,
        InCatalogRole
    };
    Q_ENUM(Roles)

    explicit ThemeInstalledModel(ThemeManager *themes, ThemeCatalogManager *catalog, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const;

signals:
    void countChanged();

private slots:
    void rebuild();
    void catalogDataChanged();

private:
    QVariantMap catalogInfo(const QString &id) const;

    ThemeManager *m_themes = nullptr;
    ThemeCatalogManager *m_catalog = nullptr;
    QList<QVariantMap> m_items;
};
