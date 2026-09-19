// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "ThemeFilterProxyModel.h"

#include <QSet>

ThemeFilterProxyModel::ThemeFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(this, &QAbstractItemModel::modelReset, this, &ThemeFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsInserted, this, &ThemeFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &ThemeFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged, this, &ThemeFilterProxyModel::countChanged);
}

QString ThemeFilterProxyModel::searchQuery() const { return m_searchQuery; }
QString ThemeFilterProxyModel::category() const { return m_category; }
QString ThemeFilterProxyModel::sortMode() const { return m_sortMode; }
int ThemeFilterProxyModel::count() const { return rowCount(); }

QStringList ThemeFilterProxyModel::categories() const
{
    QSet<QString> unique;
    if (sourceModel()) {
        for (int row = 0; row < sourceModel()->rowCount(); ++row) {
            const QString value = valueFor(sourceModel()->index(row, 0), "themeCategory").toString().trimmed();
            if (!value.isEmpty())
                unique.insert(value);
        }
    }
    QStringList result = unique.values();
    result.sort(Qt::CaseInsensitive);
    result.prepend(QStringLiteral("All"));
    return result;
}

void ThemeFilterProxyModel::setSearchQuery(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_searchQuery == normalized)
        return;
    m_searchQuery = normalized;
    refilter();
    emit searchQueryChanged();
}

void ThemeFilterProxyModel::setCategory(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("All") : value.trimmed();
    if (m_category == normalized)
        return;
    m_category = normalized;
    refilter();
    emit categoryChanged();
}

void ThemeFilterProxyModel::setSortMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (m_sortMode == normalized)
        return;
    m_sortMode = normalized;
    invalidate();
    sort(0);
    emit sortModeChanged();
}

QVariant ThemeFilterProxyModel::valueFor(const QModelIndex &index, const QByteArray &roleName) const
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

bool ThemeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel())
        return false;
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (m_category.compare(QStringLiteral("All"), Qt::CaseInsensitive) != 0
        && valueFor(index, "themeCategory").toString().compare(m_category, Qt::CaseInsensitive) != 0) {
        return false;
    }
    if (m_searchQuery.isEmpty())
        return true;

    const QString needle = m_searchQuery.toCaseFolded();
    const QStringList fields = {
        valueFor(index, "themeName").toString(),
        valueFor(index, "themeDescription").toString(),
        valueFor(index, "themePublisher").toString(),
        valueFor(index, "themePublisherStatus").toString(),
        valueFor(index, "themeCategory").toString(),
        valueFor(index, "themeTags").toStringList().join(QLatin1Char(' ')),
        valueFor(index, "themeVersion").toString()
    };
    for (const QString &field : fields) {
        if (field.toCaseFolded().contains(needle))
            return true;
    }
    return false;
}

bool ThemeFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (m_sortMode == QStringLiteral("name"))
        return valueFor(left, "themeName").toString().localeAwareCompare(valueFor(right, "themeName").toString()) < 0;
    if (m_sortMode == QStringLiteral("downloads"))
        return valueFor(left, "themeDownloads").toLongLong() > valueFor(right, "themeDownloads").toLongLong();
    if (m_sortMode == QStringLiteral("active")) {
        const bool leftActive = valueFor(left, "themeActive").toBool();
        const bool rightActive = valueFor(right, "themeActive").toBool();
        if (leftActive != rightActive)
            return leftActive;
        return valueFor(left, "themeName").toString().localeAwareCompare(valueFor(right, "themeName").toString()) < 0;
    }
    return valueFor(left, "themeUpdatedAt").toString() > valueFor(right, "themeUpdatedAt").toString();
}

void ThemeFilterProxyModel::refilter()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    endFilterChange();
#else
    // Qt 6.5-6.9 compatibility: beginFilterChange/endFilterChange are newer APIs.
    invalidateRowsFilter();
#endif
    sort(0);
    emit countChanged();
}
