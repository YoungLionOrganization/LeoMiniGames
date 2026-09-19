// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "PluginManager.h"
#include "AppPaths.h"

#include <QDir>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QJsonObject>
#include <QLibrary>
#include <QRegularExpression>
#include <QSet>
#include <QtGlobal>

PluginManager::PluginManager(AppPaths *paths, QObject *parent)
    : QAbstractListModel(parent), m_paths(paths)
{
    reload();
}

int PluginManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant PluginManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant{};

    const auto &entry = m_entries.at(static_cast<std::size_t>(index.row()));
    switch (role) {
    case IdRole: return entry.info.id;
    case NameRole: return entry.info.name;
    case DescriptionRole: return entry.info.description;
    case CategoryRole: return entry.info.category;
    case VersionRole: return entry.info.version;
    case AuthorRole: return entry.info.author;
    case IconRole: return entry.info.icon;
    case BuiltInRole: return entry.builtIn;
    default: return QVariant{};
    }
}

QHash<int, QByteArray> PluginManager::roleNames() const
{
    return {
        {IdRole, "gameId"},
        {NameRole, "gameName"},
        {DescriptionRole, "gameDescription"},
        {CategoryRole, "gameCategory"},
        {VersionRole, "gameVersion"},
        {AuthorRole, "gameAuthor"},
        {IconRole, "gameIcon"},
        {BuiltInRole, "builtIn"}
    };
}

QStringList PluginManager::errors() const
{
    return m_errors;
}

int PluginManager::count() const
{
    return rowCount();
}

QString PluginManager::nameFor(const QString &id) const
{
    for (const auto &entry : m_entries) {
        if (entry.info.id == id)
            return entry.info.name;
    }
    return QString{};
}

QString PluginManager::descriptionFor(const QString &id) const
{
    for (const auto &entry : m_entries) {
        if (entry.info.id == id)
            return entry.info.description;
    }
    return QString{};
}

void PluginManager::addError(const QString &message)
{
    m_errors.append(message);
}

void PluginManager::registerPlugin(QObject *object, bool builtIn, const QJsonObject &loaderMetadata)
{
    if (!object)
        return;

    auto *plugin = qobject_cast<IGamePlugin *>(object);
    if (!plugin)
        return;

    const GamePluginInfo info = plugin->info();
    static const QRegularExpression idPattern(QStringLiteral("^[a-z0-9][a-z0-9_.-]{1,63}$"));

    if (!idPattern.match(info.id).hasMatch()) {
        addError(QStringLiteral("Invalid plugin id: %1").arg(info.id));
        return;
    }
    if (info.apiVersion != 1) {
        addError(QStringLiteral("Unsupported plugin API for %1: %2").arg(info.id).arg(info.apiVersion));
        return;
    }
    if (!info.entryUrl.isValid() || info.entryUrl.isEmpty()) {
        addError(QStringLiteral("Missing QML entry URL for %1").arg(info.id));
        return;
    }
    for (const auto &entry : m_entries) {
        if (entry.info.id == info.id) {
            addError(QStringLiteral("Duplicate plugin id: %1").arg(info.id));
            return;
        }
    }

    if (!loaderMetadata.isEmpty()) {
        const QJsonObject custom = loaderMetadata.value(QStringLiteral("MetaData")).toObject();
        const QString metadataId = custom.value(QStringLiteral("id")).toString();
        if (!metadataId.isEmpty() && metadataId != info.id) {
            addError(QStringLiteral("Plugin metadata id mismatch: %1").arg(info.id));
            return;
        }
    }

    m_entries.push_back(Entry{plugin, info, builtIn});
}

void PluginManager::reload()
{
    beginResetModel();
    m_entries.clear();
    m_dynamicLoaders.clear();
    m_errors.clear();

    // Built-in games are real static Qt plugins. main.cpp imports them with
    // Q_IMPORT_PLUGIN and the plugin targets are linked to the executable.
    const QObjectList staticPlugins = QPluginLoader::staticInstances();
    for (QObject *pluginObject : staticPlugins)
        registerPlugin(pluginObject, true);

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    // Level 3/native plugins are never trusted because a manifest says so.
    // They must be explicitly present in the local reviewed trust snapshot and hash-match it.
    QDir pluginDir(m_paths->plugins());
    QFile trustFile(pluginDir.filePath(QStringLiteral("trusted-native.json")));
    if (trustFile.open(QIODevice::ReadOnly)) {
        constexpr qint64 kMaxTrustSnapshotBytes = 2LL * 1024LL * 1024LL;
        if (trustFile.size() > kMaxTrustSnapshotBytes) {
            addError(QStringLiteral("Trusted native snapshot exceeds the 2 MiB safety limit."));
            trustFile.close();
        }
        QJsonParseError parseError;
        const QByteArray trustPayload = trustFile.isOpen() ? trustFile.read(kMaxTrustSnapshotBytes + 1) : QByteArray{};
        const QJsonDocument doc = QJsonDocument::fromJson(trustPayload, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonArray entries = doc.object().value(QStringLiteral("plugins")).toArray();
            for (const QJsonValue &value : entries) {
                const QJsonObject trust = value.toObject();
                if (!trust.value(QStringLiteral("reviewed")).toBool(false) ||
                    !trust.value(QStringLiteral("publisher_verified")).toBool(false))
                    continue;
                const QString fileName = trust.value(QStringLiteral("file")).toString();
                const QString expectedSha = trust.value(QStringLiteral("sha256")).toString().toLower();
                const int apiVersion = trust.value(QStringLiteral("api_version")).toInt(1);
                const int qtMajor = trust.value(QStringLiteral("qt_major")).toInt(QT_VERSION_MAJOR);
                if (fileName.isEmpty() || QFileInfo(fileName).fileName() != fileName || fileName.contains(QLatin1Char('/')) ||
                    fileName.contains(QLatin1Char('\\')) || apiVersion != 1 || qtMajor != QT_VERSION_MAJOR)
                    continue;
                const QFileInfo file(pluginDir.filePath(fileName));
                constexpr qint64 kMaxNativePluginBytes = 256LL * 1024LL * 1024LL;
                if (!file.exists() || file.size() <= 0 || file.size() > kMaxNativePluginBytes ||
                    !QLibrary::isLibrary(file.absoluteFilePath()) ||
                    !QRegularExpression(QStringLiteral("^[a-f0-9]{64}$")).match(expectedSha).hasMatch())
                    continue;
                QFile library(file.absoluteFilePath());
                if (!library.open(QIODevice::ReadOnly))
                    continue;
                QCryptographicHash hash(QCryptographicHash::Sha256);
                while (!library.atEnd()) hash.addData(library.read(1024 * 1024));
                if (QString::fromLatin1(hash.result().toHex()).compare(expectedSha, Qt::CaseInsensitive) != 0) {
                    addError(QStringLiteral("Trusted native hash mismatch: %1").arg(fileName));
                    continue;
                }
                auto loader = std::make_unique<QPluginLoader>(file.absoluteFilePath());
                QObject *object = loader->instance();
                if (!object) {
                    addError(QStringLiteral("%1: %2").arg(file.fileName(), loader->errorString()));
                    continue;
                }
                registerPlugin(object, false, loader->metaData());
                m_dynamicLoaders.push_back(std::move(loader));
            }
        }
    }
#endif

    endResetModel();
    emit errorsChanged();
    emit countChanged();
}

QObject *PluginManager::createGame(const QString &id, QObject *parent)
{
    for (const auto &entry : m_entries) {
        if (entry.info.id == id)
            return entry.plugin->createGame(parent);
    }
    return nullptr;
}

QUrl PluginManager::entryUrl(const QString &id) const
{
    for (const auto &entry : m_entries) {
        if (entry.info.id == id)
            return entry.info.entryUrl;
    }
    return QUrl{};
}
