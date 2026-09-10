// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameRegistry.h"
#include "ModManager.h"
#include "PluginManager.h"

#include <QModelIndex>
#include <utility>

namespace {
QVariant modelValue(const QAbstractItemModel *model, int row, const QByteArray &roleName)
{
    const auto names = model->roleNames();
    for (auto it = names.cbegin(); it != names.cend(); ++it) {
        if (it.value() == roleName)
            return model->data(model->index(row, 0), it.key());
    }
    return QVariant{};
}
}

GameRegistry::GameRegistry(PluginManager *plugins, ModManager *mods, QObject *parent)
    : QAbstractListModel(parent), m_plugins(plugins), m_mods(mods)
{
    if (m_plugins) {
        connect(m_plugins, &QAbstractItemModel::modelReset, this, &GameRegistry::refresh);
        connect(m_plugins, &QAbstractItemModel::rowsInserted, this, &GameRegistry::refresh);
        connect(m_plugins, &QAbstractItemModel::rowsRemoved, this, &GameRegistry::refresh);
    }
    if (m_mods) {
        connect(m_mods, &ModManager::modInstalled, this, [this](const QString &) { refresh(); });
        connect(m_mods, &ModManager::modUninstalled, this, [this](const QString &) { refresh(); });
        connect(m_mods, &QAbstractItemModel::modelReset, this, &GameRegistry::refresh);
    }
    refresh();
}

int GameRegistry::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_visible.size());
}

QVariant GameRegistry::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant{};
    const Entry &entry = m_all.at(static_cast<std::size_t>(m_visible.at(static_cast<std::size_t>(index.row()))));
    switch (role) {
    case IdRole: return entry.id;
    case NameRole: return entry.name;
    case LocalizedNameRole: return entry.localizedName;
    case DescriptionRole: return entry.description;
    case CategoryRole: return entry.category;
    case VersionRole: return entry.version;
    case PublisherRole: return entry.publisher;
    case IconRole: return entry.icon;
    case IconUrlRole: return entry.iconUrl;
    case BuiltInRole: return entry.builtIn;
    case SourceRole: return entry.source;
    case TagsRole: return entry.tags;
    case PublisherStatusRole: return entry.publisherStatus;
    case VerifiedRole: return entry.verified;
    case PluginLevelRole: return entry.pluginLevel;
    case NativeRole: return entry.native;
    case LicenseRole: return entry.license;
    case SourceUrlRole: return entry.sourceUrl;
    case SourceAvailableRole: return entry.sourceAvailable;
    case SaveVersionRole: return entry.saveVersion;
    default: return QVariant{};
    }
}

QHash<int, QByteArray> GameRegistry::roleNames() const
{
    return {
        {IdRole, "gameId"}, {NameRole, "gameName"}, {LocalizedNameRole, "gameLocalizedName"},
        {DescriptionRole, "gameDescription"}, {CategoryRole, "gameCategory"}, {VersionRole, "gameVersion"},
        {PublisherRole, "gamePublisher"}, {IconRole, "gameIcon"}, {IconUrlRole, "gameIconUrl"},
        {BuiltInRole, "builtIn"}, {SourceRole, "gameSource"}, {TagsRole, "gameTags"},
        {PublisherStatusRole, "publisherStatus"}, {VerifiedRole, "publisherVerified"},
        {PluginLevelRole, "pluginLevel"}, {NativeRole, "nativePlugin"}, {LicenseRole, "gameLicense"},
        {SourceUrlRole, "gameSourceUrl"}, {SourceAvailableRole, "gameSourceAvailable"}, {SaveVersionRole, "saveVersion"}
    };
}

QString GameRegistry::query() const { return m_query; }
int GameRegistry::count() const { return rowCount(); }
int GameRegistry::totalCount() const { return static_cast<int>(m_all.size()); }

void GameRegistry::refresh()
{
    const int oldTotal = totalCount();
    beginResetModel();
    m_all.clear();

    if (m_plugins) {
        for (int row = 0; row < m_plugins->rowCount(); ++row) {
            Entry entry;
            entry.id = modelValue(m_plugins, row, "gameId").toString();
            entry.name = modelValue(m_plugins, row, "gameName").toString();
            entry.localizedName = entry.name;
            entry.description = modelValue(m_plugins, row, "gameDescription").toString();
            entry.category = modelValue(m_plugins, row, "gameCategory").toString();
            entry.version = modelValue(m_plugins, row, "gameVersion").toString();
            entry.publisher = modelValue(m_plugins, row, "gameAuthor").toString();
            entry.icon = modelValue(m_plugins, row, "gameIcon").toString();
            entry.builtIn = modelValue(m_plugins, row, "builtIn").toBool();
            entry.source = entry.builtIn ? QStringLiteral("BuiltIn") : QStringLiteral("TrustedNative");
            entry.publisherStatus = entry.builtIn ? QStringLiteral("Official Publisher") : QStringLiteral("Verified + Native/L3");
            entry.verified = true;
            entry.pluginLevel = entry.builtIn ? 2 : 3;
            entry.native = !entry.builtIn;
            entry.license = entry.builtIn ? QStringLiteral("GPL-3.0-or-later") : QString{};
            m_all.push_back(std::move(entry));
        }
    }

    if (m_mods) {
        for (int row = 0; row < m_mods->rowCount(); ++row) {
            if (!modelValue(m_mods, row, "modInstalled").toBool())
                continue;
            const QString id = modelValue(m_mods, row, "modId").toString();
            bool duplicate = false;
            for (const Entry &existing : m_all) {
                if (existing.id == id) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate)
                continue;

            Entry entry;
            entry.id = id;
            entry.name = modelValue(m_mods, row, "modName").toString();
            entry.localizedName = modelValue(m_mods, row, "modLocalizedName").toString();
            if (entry.localizedName.isEmpty())
                entry.localizedName = entry.name;
            entry.description = modelValue(m_mods, row, "modDescription").toString();
            entry.category = modelValue(m_mods, row, "modCategory").toString();
            entry.version = modelValue(m_mods, row, "modInstalledVersion").toString();
            entry.publisher = modelValue(m_mods, row, "modAuthor").toString();
            entry.icon = modelValue(m_mods, row, "modIcon").toString();
            entry.iconUrl = modelValue(m_mods, row, "modIconUrl").toString();
            entry.source = QStringLiteral("ExternalRcc");
            entry.tags = modelValue(m_mods, row, "modTags").toStringList();
            entry.publisherStatus = modelValue(m_mods, row, "modPublisherStatus").toString();
            entry.verified = modelValue(m_mods, row, "modPublisherVerified").toBool();
            entry.pluginLevel = qBound(1, modelValue(m_mods, row, "modPluginLevel").toInt(), 2);
            entry.native = false;
            entry.license = modelValue(m_mods, row, "modLicense").toString();
            entry.sourceUrl = modelValue(m_mods, row, "modSourceUrl").toString();
            entry.sourceAvailable = modelValue(m_mods, row, "modSourceAvailable").toBool();
            entry.saveVersion = qMax(1, modelValue(m_mods, row, "modSaveVersion").toInt());
            m_all.push_back(std::move(entry));
        }
    }

    m_visible.clear();
    for (int i = 0; i < static_cast<int>(m_all.size()); ++i) {
        if (matches(m_all.at(static_cast<std::size_t>(i))))
            m_visible.push_back(i);
    }
    endResetModel();
    emit countChanged();
    if (oldTotal != totalCount())
        emit totalCountChanged();
    emit refreshed();
}

void GameRegistry::setQuery(const QString &query)
{
    const QString normalized = query.trimmed();
    if (m_query == normalized)
        return;
    m_query = normalized;
    rebuildVisible();
    emit queryChanged();
}

bool GameRegistry::matches(const Entry &entry) const
{
    if (m_query.isEmpty())
        return true;
    const QString needle = m_query.toCaseFolded();
    const QStringList fields = {
        entry.name, entry.localizedName, entry.category, entry.publisher,
        entry.publisherStatus, entry.tags.join(QLatin1Char(' '))
    };
    for (const QString &field : fields) {
        if (field.toCaseFolded().contains(needle))
            return true;
    }
    return false;
}

void GameRegistry::rebuildVisible()
{
    beginResetModel();
    m_visible.clear();
    for (int i = 0; i < static_cast<int>(m_all.size()); ++i) {
        if (matches(m_all.at(static_cast<std::size_t>(i))))
            m_visible.push_back(i);
    }
    endResetModel();
    emit countChanged();
}

const GameRegistry::Entry *GameRegistry::find(const QString &id) const
{
    for (const Entry &entry : m_all) {
        if (entry.id == id)
            return &entry;
    }
    return nullptr;
}

QString GameRegistry::nameFor(const QString &id) const { const Entry *e = find(id); return e ? e->name : QString{}; }
QString GameRegistry::descriptionFor(const QString &id) const { const Entry *e = find(id); return e ? e->description : QString{}; }
QString GameRegistry::versionFor(const QString &id) const { const Entry *e = find(id); return e ? e->version : QString{}; }
QString GameRegistry::sourceFor(const QString &id) const { const Entry *e = find(id); return e ? e->source : QString{}; }
int GameRegistry::saveVersionFor(const QString &id) const { const Entry *e = find(id); return e ? e->saveVersion : 1; }

QUrl GameRegistry::entryUrl(const QString &id) const
{
    const Entry *entry = find(id);
    if (!entry)
        return QUrl{};
    if (entry->source == QStringLiteral("ExternalRcc"))
        return m_mods ? m_mods->entryUrl(id) : QUrl{};
    return m_plugins ? m_plugins->entryUrl(id) : QUrl{};
}

QObject *GameRegistry::createGame(const QString &id, QObject *parent) const
{
    const Entry *entry = find(id);
    if (!entry || entry->source == QStringLiteral("ExternalRcc"))
        return nullptr;
    return m_plugins ? m_plugins->createGame(id, parent) : nullptr;
}
