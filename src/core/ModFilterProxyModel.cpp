// SPDX-License-Identifier: GPL-3.0-or-later
#include "ModFilterProxyModel.h"

ModFilterProxyModel::ModFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(this, &QAbstractItemModel::modelReset, this, &ModFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsInserted, this, &ModFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &ModFilterProxyModel::countChanged);
}

QString ModFilterProxyModel::searchQuery() const { return m_searchQuery; }
QString ModFilterProxyModel::category() const { return m_category; }
QString ModFilterProxyModel::mode() const { return m_mode; }
QString ModFilterProxyModel::sortMode() const { return m_sortMode; }
int ModFilterProxyModel::count() const { return rowCount(); }

void ModFilterProxyModel::setSearchQuery(const QString &value) { const QString v = value.trimmed(); if (m_searchQuery == v) return; m_searchQuery = v; refilter(); emit searchQueryChanged(); }
void ModFilterProxyModel::setCategory(const QString &value) { const QString v = value.trimmed().isEmpty() ? QStringLiteral("All") : value.trimmed(); if (m_category == v) return; m_category = v; refilter(); emit categoryChanged(); }
void ModFilterProxyModel::setMode(const QString &value) { const QString v = value.toLower(); if (m_mode == v) return; m_mode = v; refilter(); emit modeChanged(); }
void ModFilterProxyModel::setSortMode(const QString &value) { const QString v = value.toLower(); if (m_sortMode == v) return; m_sortMode = v; invalidate(); sort(0); emit sortModeChanged(); }

QVariant ModFilterProxyModel::valueFor(const QModelIndex &index, const QByteArray &roleName) const
{
    if (!sourceModel())
        return QVariant{};
    const auto roles = sourceModel()->roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
        if (it.value() == roleName)
            return sourceModel()->data(index, it.key());
    }
    return QVariant{};
}

bool ModFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel())
        return false;
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const bool installed = valueFor(idx, "modInstalled").toBool();
    const bool inCatalog = valueFor(idx, "modInCatalog").toBool();
    if (m_mode == QStringLiteral("installed") && !installed)
        return false;
    if (m_mode == QStringLiteral("catalog") && !inCatalog)
        return false;
    if (m_category.compare(QStringLiteral("All"), Qt::CaseInsensitive) != 0 &&
        valueFor(idx, "modCategory").toString().compare(m_category, Qt::CaseInsensitive) != 0)
        return false;
    if (m_searchQuery.isEmpty())
        return true;

    const QString needle = m_searchQuery.toCaseFolded();
    const QStringList fields = {
        valueFor(idx, "modName").toString(), valueFor(idx, "modLocalizedName").toString(),
        valueFor(idx, "modDescription").toString(), valueFor(idx, "modAuthor").toString(),
        valueFor(idx, "modCategory").toString(), valueFor(idx, "modTags").toStringList().join(QLatin1Char(' '))
    };
    for (const QString &field : fields) {
        if (field.toCaseFolded().contains(needle))
            return true;
    }
    return false;
}

bool ModFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (m_sortMode == QStringLiteral("name"))
        return valueFor(left, "modName").toString().localeAwareCompare(valueFor(right, "modName").toString()) < 0;
    if (m_sortMode == QStringLiteral("downloads"))
        return valueFor(left, "modDownloads").toLongLong() > valueFor(right, "modDownloads").toLongLong();
    return valueFor(left, "modUpdatedAt").toString() > valueFor(right, "modUpdatedAt").toString();
}

void ModFilterProxyModel::refilter()
{
    beginFilterChange();
    endFilterChange();
    sort(0);
    emit countChanged();
}
