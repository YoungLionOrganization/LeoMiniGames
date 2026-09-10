// SPDX-License-Identifier: GPL-3.0-or-later
#include "ThemeInstalledModel.h"

#include "ThemeCatalogManager.h"
#include "ThemeManager.h"

ThemeInstalledModel::ThemeInstalledModel(ThemeManager *themes, ThemeCatalogManager *catalog, QObject *parent)
    : QAbstractListModel(parent), m_themes(themes), m_catalog(catalog)
{
    if (m_themes) {
        connect(m_themes, &ThemeManager::themesChanged, this, &ThemeInstalledModel::rebuild);
        connect(m_themes, &ThemeManager::activeThemeChanged, this, &ThemeInstalledModel::catalogDataChanged);
    }
    if (m_catalog) {
        connect(m_catalog, &QAbstractItemModel::modelReset, this, &ThemeInstalledModel::rebuild);
        connect(m_catalog, &QAbstractItemModel::rowsInserted, this, &ThemeInstalledModel::rebuild);
        connect(m_catalog, &QAbstractItemModel::rowsRemoved, this, &ThemeInstalledModel::rebuild);
        connect(m_catalog, &QAbstractItemModel::dataChanged, this, &ThemeInstalledModel::catalogDataChanged);
    }
    rebuild();
}

int ThemeInstalledModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

int ThemeInstalledModel::count() const { return rowCount(); }

QVariantMap ThemeInstalledModel::catalogInfo(const QString &id) const
{
    return m_catalog ? m_catalog->entryInfo(id) : QVariantMap{};
}

QVariant ThemeInstalledModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant{};
    const QVariantMap &local = m_items.at(index.row());
    const QString id = local.value(QStringLiteral("id")).toString();
    const QVariantMap catalog = catalogInfo(id);
    const bool inCatalog = !catalog.isEmpty();
    const QString installedVersion = local.value(QStringLiteral("version")).toString();
    const QString latestVersion = inCatalog ? catalog.value(QStringLiteral("themeVersion")).toString() : installedVersion;

    switch (role) {
    case IdRole: return id;
    case NameRole: return catalog.value(QStringLiteral("themeName"), local.value(QStringLiteral("name")));
    case DescriptionRole: return catalog.value(QStringLiteral("themeDescription"), QString{});
    case PublisherRole: return catalog.value(QStringLiteral("themePublisher"), local.value(QStringLiteral("publisher")));
    case PublisherStatusRole:
        return catalog.value(QStringLiteral("themePublisherStatus"), local.value(QStringLiteral("builtIn")).toBool()
                             ? QStringLiteral("YoungLion Official") : QStringLiteral("Local/Sideloaded"));
    case PublisherVerifiedRole:
        return catalog.value(QStringLiteral("themePublisherVerified"), local.value(QStringLiteral("builtIn")).toBool());
    case VersionRole: return latestVersion;
    case InstalledVersionRole: return installedVersion;
    case CategoryRole: return catalog.value(QStringLiteral("themeCategory"), QStringLiteral("Theme"));
    case TagsRole: return catalog.value(QStringLiteral("themeTags"), QVariantList{});
    case IconUrlRole: return catalog.value(QStringLiteral("themeIconUrl"), QString{});
    case PreviewUrlsRole: return catalog.value(QStringLiteral("themePreviewUrls"), QVariantList{});
    case SizeRole: return catalog.value(QStringLiteral("themeSizeBytes"), 0);
    case DownloadsRole: return catalog.value(QStringLiteral("themeDownloads"), 0);
    case UpdatedAtRole: return catalog.value(QStringLiteral("themeUpdatedAt"), QString{});
    case StateRole: return catalog.value(QStringLiteral("themeState"), QStringLiteral("installed"));
    case ProgressRole: return catalog.value(QStringLiteral("themeProgress"), 0.0);
    case OperationErrorRole: return catalog.value(QStringLiteral("themeOperationError"), QString{});
    case InstalledRole: return true;
    case UpdateAvailableRole: return catalog.value(QStringLiteral("themeUpdateAvailable"), false);
    case ActiveRole: return m_themes && m_themes->activeThemeId() == id;
    case BuiltInRole: return local.value(QStringLiteral("builtIn"));
    case InCatalogRole: return inCatalog;
    default: return QVariant{};
    }
}

QHash<int, QByteArray> ThemeInstalledModel::roleNames() const
{
    return {
        {IdRole, "themeId"}, {NameRole, "themeName"}, {DescriptionRole, "themeDescription"},
        {PublisherRole, "themePublisher"}, {PublisherStatusRole, "themePublisherStatus"},
        {PublisherVerifiedRole, "themePublisherVerified"}, {VersionRole, "themeVersion"},
        {InstalledVersionRole, "themeInstalledVersion"}, {CategoryRole, "themeCategory"},
        {TagsRole, "themeTags"}, {IconUrlRole, "themeIconUrl"}, {PreviewUrlsRole, "themePreviewUrls"},
        {SizeRole, "themeSizeBytes"}, {DownloadsRole, "themeDownloads"}, {UpdatedAtRole, "themeUpdatedAt"},
        {StateRole, "themeState"}, {ProgressRole, "themeProgress"}, {OperationErrorRole, "themeOperationError"},
        {InstalledRole, "themeInstalled"}, {UpdateAvailableRole, "themeUpdateAvailable"},
        {ActiveRole, "themeActive"}, {BuiltInRole, "themeBuiltIn"}, {InCatalogRole, "themeInCatalog"}
    };
}

void ThemeInstalledModel::rebuild()
{
    beginResetModel();
    m_items.clear();
    if (m_themes) {
        const QVariantList themes = m_themes->themes();
        m_items.reserve(themes.size());
        for (const QVariant &value : themes)
            m_items.append(value.toMap());
    }
    endResetModel();
    emit countChanged();
}

void ThemeInstalledModel::catalogDataChanged()
{
    if (m_items.isEmpty())
        return;
    emit dataChanged(index(0, 0), index(m_items.size() - 1, 0));
}
