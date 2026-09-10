// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QSortFilterProxyModel>

class ModFilterProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ModFilterProxyModel(QObject *parent = nullptr);

    QString searchQuery() const;
    QString category() const;
    QString mode() const;
    QString sortMode() const;
    int count() const;

public slots:
    void setSearchQuery(const QString &value);
    void setCategory(const QString &value);
    void setMode(const QString &value);
    void setSortMode(const QString &value);

signals:
    void searchQueryChanged();
    void categoryChanged();
    void modeChanged();
    void sortModeChanged();
    void countChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QVariant valueFor(const QModelIndex &index, const QByteArray &roleName) const;
    void refilter();

    QString m_searchQuery;
    QString m_category = QStringLiteral("All");
    QString m_mode = QStringLiteral("catalog");
    QString m_sortMode = QStringLiteral("updated");
};
