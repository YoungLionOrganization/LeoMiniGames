// SPDX-License-Identifier: GPL-3.0-or-later
#include "ThemeManager.h"

#include "AppPaths.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QResource>
#include <QSaveFile>
#include <QSet>
#include <QSettings>

#include <cmath>

namespace {
constexpr int kMaxAliasDepth = 32;
constexpr qint64 kMaxThemeBytes = 64 * 1024 * 1024;
constexpr qint64 kMaxManifestBytes = 2 * 1024 * 1024;
constexpr int kMaxTokens = 10000;
constexpr int kMaxAliases = 20000;
constexpr int kMaxSurfaces = 256;


bool isProtectedPrimitiveKey(const QString &key)
{
    return key.startsWith(QStringLiteral("metric.unit."))
        || key.startsWith(QStringLiteral("metric.ratio."))
        || key.startsWith(QStringLiteral("metric.data."));
}

bool isAllowedThemeResource(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    static const QSet<QString> allowed{
        QStringLiteral("json"), QStringLiteral("png"), QStringLiteral("webp"),
        QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("svg"),
        QStringLiteral("ttf"), QStringLiteral("otf"), QStringLiteral("woff2")
    };
    return allowed.contains(suffix);
}

qreal sanitizedThemeNumber(const QString &key, qreal value)
{
    if (!std::isfinite(value))
        return 0.0;
    const QString lower = key.toLower();
    if (lower.contains(QStringLiteral("opacity"))
        || lower.contains(QStringLiteral("ratio"))
        || lower.contains(QStringLiteral("progress"))) {
        return qBound<qreal>(0.0, value, 1.0);
    }
    if (lower.contains(QStringLiteral("scale")))
        return qBound<qreal>(0.0, value, 8.0);
    if (lower.contains(QStringLiteral("duration")) || lower.contains(QStringLiteral("delay")))
        return qBound<qreal>(0.0, value, 60000.0);
    if (lower.contains(QStringLiteral("offset")) || lower.contains(QStringLiteral("letterspacing")))
        return qBound<qreal>(-4096.0, value, 4096.0);
    return qBound<qreal>(0.0, value, 4096.0);
}
}

ThemeManager::ThemeManager(AppPaths *paths, QObject *parent)
    : QObject(parent), m_paths(paths)
{
    loadBuiltIn();
    reloadInstalled();
    restoreSelection();
}

ThemeManager::~ThemeManager()
{
    for (auto it = m_records.cbegin(); it != m_records.cend(); ++it) {
        if (!it->builtIn && it->registered)
            QResource::unregisterResource(it->filePath, it->resourceRoot);
    }
}

QString ThemeManager::activeThemeId() const { return m_activeId; }
QString ThemeManager::activeThemeName() const { return m_records.value(m_activeId).name; }
int ThemeManager::revision() const { return m_revision; }
int ThemeManager::themeCount() const { return m_records.size(); }
QString ThemeManager::lastError() const { return m_lastError; }

QVariantList ThemeManager::themes() const
{
    QVariantList out;
    QStringList ids = m_records.keys();
    ids.sort(Qt::CaseInsensitive);
    for (const QString &id : ids) {
        const ThemeRecord &r = m_records[id];
        QVariantMap item;
        item.insert(QStringLiteral("id"), r.id);
        item.insert(QStringLiteral("name"), r.name);
        item.insert(QStringLiteral("version"), r.version);
        item.insert(QStringLiteral("publisher"), r.publisher);
        item.insert(QStringLiteral("builtIn"), r.builtIn);
        item.insert(QStringLiteral("active"), r.id == m_activeId);
        item.insert(QStringLiteral("capabilities"), r.document.value(QStringLiteral("capabilities")));
        out.push_back(item);
    }
    return out;
}

QVariant ThemeManager::value(const QString &key) const
{
    return resolveKey(key);
}

QColor ThemeManager::color(const QString &key) const
{
    const QVariant v = resolveKey(key);
    if (v.canConvert<QColor>()) {
        const QColor c = v.value<QColor>();
        if (c.isValid())
            return c;
    }
    const QColor parsed(v.toString());
    return parsed.isValid() ? parsed : QColor(Qt::transparent);
}

qreal ThemeManager::number(const QString &key) const
{
    bool ok = false;
    const qreal n = resolveKey(key).toDouble(&ok);
    return ok ? sanitizedThemeNumber(key, n) : 0.0;
}

QString ThemeManager::stringValue(const QString &key) const
{
    return resolveKey(key).toString();
}

bool ThemeManager::hasValue(const QString &key) const
{
    return recordContainsKey(m_records.value(m_activeId), key)
        || recordContainsKey(m_records.value(m_defaultId), key);
}

QStringList ThemeManager::keys(const QString &prefix) const
{
    QSet<QString> merged;
    const auto collect = [&merged, &prefix](const ThemeRecord &record) {
        const QVariantMap aliases = record.document.value(QStringLiteral("aliases")).toMap();
        const QVariantMap tokens = record.document.value(QStringLiteral("tokens")).toMap();
        for (auto it = aliases.cbegin(); it != aliases.cend(); ++it) {
            if (prefix.isEmpty() || it.key().startsWith(prefix))
                merged.insert(it.key());
        }
        for (auto it = tokens.cbegin(); it != tokens.cend(); ++it) {
            if (prefix.isEmpty() || it.key().startsWith(prefix))
                merged.insert(it.key());
        }
    };
    collect(m_records.value(m_defaultId));
    if (m_activeId != m_defaultId)
        collect(m_records.value(m_activeId));
    QStringList out = merged.values();
    out.sort(Qt::CaseInsensitive);
    return out;
}

QVariantMap ThemeManager::values(const QString &prefix) const
{
    QVariantMap out;
    const QStringList names = keys(prefix);
    for (const QString &key : names)
        out.insert(key, resolveKey(key));
    return out;
}

bool ThemeManager::isInstalled(const QString &id) const
{
    return m_records.contains(id);
}

QString ThemeManager::installedVersion(const QString &id) const
{
    return m_records.contains(id) ? m_records.value(id).version : QString{};
}

QVariantMap ThemeManager::surface(const QString &key) const
{
    const ThemeRecord active = m_records.value(m_activeId);
    const ThemeRecord fallback = m_records.value(m_defaultId);
    const QVariantMap surfaces = active.document.value(QStringLiteral("surfaces")).toMap();
    const QVariantMap baseSurfaces = fallback.document.value(QStringLiteral("surfaces")).toMap();
    if (surfaces.contains(key))
        return resolveNested(surfaces.value(key), active.resourceRoot).toMap();
    return resolveNested(baseSurfaces.value(key), fallback.resourceRoot).toMap();
}

bool ThemeManager::applyTheme(const QString &id)
{
    if (!m_records.contains(id)) {
        setError(QStringLiteral("Theme not installed: %1").arg(id));
        return false;
    }
    if (m_activeId == id)
        return true;
    m_activeId = id;
    persistSelection();
    emit activeThemeChanged();
    bumpRevision();
    emit themesChanged();
    return true;
}

bool ThemeManager::installThemeRcc(const QString &sourcePath, const QString &expectedSha256)
{
    if (!m_paths) {
        setError(QStringLiteral("Theme storage is unavailable."));
        return false;
    }
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.isFile() || sourceInfo.size() <= 0 || sourceInfo.size() > kMaxThemeBytes) {
        setError(QStringLiteral("Invalid theme package size."));
        return false;
    }
    const QString actualHash = sha256File(sourcePath);
    if (actualHash.isEmpty()) {
        setError(QStringLiteral("Cannot hash theme package."));
        return false;
    }
    if (!expectedSha256.trimmed().isEmpty() && actualHash.compare(expectedSha256.trimmed(), Qt::CaseInsensitive) != 0) {
        setError(QStringLiteral("Theme SHA-256 verification failed."));
        return false;
    }

    const QString dest = m_paths->themes() + QLatin1Char('/') + actualHash.left(24) + QStringLiteral(".rcc");
    bool copied = false;
    if (QFileInfo(sourcePath).absoluteFilePath() != QFileInfo(dest).absoluteFilePath()) {
        QFile input(sourcePath);
        if (!input.open(QIODevice::ReadOnly)) {
            setError(QStringLiteral("Cannot open theme package."));
            return false;
        }
        QSaveFile output(dest);
        if (!output.open(QIODevice::WriteOnly)) {
            setError(QStringLiteral("Cannot create installed theme package."));
            return false;
        }
        QByteArray buffer;
        buffer.resize(64 * 1024);
        bool copyOk = true;
        while (!input.atEnd()) {
            const qint64 count = input.read(buffer.data(), buffer.size());
            if (count < 0 || output.write(buffer.constData(), count) != count) {
                copyOk = false;
                break;
            }
        }
        if (!copyOk || !output.commit()) {
            output.cancelWriting();
            setError(QStringLiteral("Cannot install theme package."));
            return false;
        }
        copied = true;
    }

    // Same hash is already installed: treat reinstall as success rather than
    // trying to register the same RCC root twice.
    for (auto it = m_records.cbegin(); it != m_records.cend(); ++it) {
        if (!it->builtIn && QFileInfo(it->filePath).absoluteFilePath() == QFileInfo(dest).absoluteFilePath())
            return true;
    }

    ThemeRecord candidate;
    if (!loadExternalRecord(dest, &candidate)) {
        if (copied)
            QFile::remove(dest);
        return false;
    }
    if (candidate.id == m_defaultId) {
        QResource::unregisterResource(candidate.filePath, candidate.resourceRoot);
        if (copied)
            QFile::remove(dest);
        setError(QStringLiteral("External themes cannot replace the built-in theme id."));
        return false;
    }

    const bool replacing = m_records.contains(candidate.id);
    ThemeRecord previous;
    if (replacing) {
        previous = m_records.value(candidate.id);
        if (previous.builtIn) {
            QResource::unregisterResource(candidate.filePath, candidate.resourceRoot);
            if (copied)
                QFile::remove(dest);
            setError(QStringLiteral("Built-in themes cannot be replaced."));
            return false;
        }
    }

    // Candidate is already parsed and registered, so commit is a no-fail map
    // swap. Only after that do we release/delete the old RCC.
    m_records.insert(candidate.id, candidate);
    if (replacing && previous.registered)
        QResource::unregisterResource(previous.filePath, previous.resourceRoot);
    if (replacing && QFileInfo(previous.filePath).absoluteFilePath() != QFileInfo(candidate.filePath).absoluteFilePath())
        QFile::remove(previous.filePath);

    if (replacing)
        emit themeUpdated(candidate.id);
    else
        emit themeInstalled(candidate.id);
    if (m_activeId == candidate.id) {
        emit activeThemeChanged();
        bumpRevision();
    }
    emit themesChanged();
    return true;
}

bool ThemeManager::updateThemeRcc(const QString &sourcePath, const QString &expectedSha256)
{
    return installThemeRcc(sourcePath, expectedSha256);
}

bool ThemeManager::removeTheme(const QString &id)
{
    if (!m_records.contains(id) || m_records[id].builtIn) {
        setError(QStringLiteral("Built-in or unknown theme cannot be removed."));
        return false;
    }
    if (m_activeId == id)
        applyTheme(m_defaultId);
    const ThemeRecord record = m_records.take(id);
    if (record.registered)
        QResource::unregisterResource(record.filePath, record.resourceRoot);
    QFile::remove(record.filePath);
    emit themeRemoved(id);
    emit themesChanged();
    bumpRevision();
    return true;
}

void ThemeManager::reloadInstalled()
{
    if (!m_paths)
        return;
    const QDir dir(m_paths->themes());
    const QFileInfoList files = dir.entryInfoList(QStringList{QStringLiteral("*.rcc")}, QDir::Files, QDir::Name);
    for (const QFileInfo &file : files) {
        bool already = false;
        for (auto it = m_records.cbegin(); it != m_records.cend(); ++it) {
            const ThemeRecord &r = it.value();
            if (QFileInfo(r.filePath).absoluteFilePath() == file.absoluteFilePath()) {
                already = true;
                break;
            }
        }
        if (!already)
            registerExternal(file.absoluteFilePath());
    }
    emit themesChanged();
}

bool ThemeManager::loadBuiltIn()
{
    QFile file(QStringLiteral(":/themes/default_theme.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Built-in theme is missing."));
        return false;
    }
    if (file.size() > kMaxManifestBytes) {
        setError(QStringLiteral("Built-in theme manifest is too large."));
        return false;
    }
    ThemeRecord record;
    if (!parseDocument(file.readAll(), &record, QStringLiteral("qrc:/themes"), QString{}, true))
        return false;
    m_defaultId = record.id;
    m_activeId = record.id;
    m_records.insert(record.id, record);
    return true;
}

bool ThemeManager::registerExternal(const QString &filePath)
{
    ThemeRecord record;
    if (!loadExternalRecord(filePath, &record))
        return false;
    if (record.id == m_defaultId || m_records.contains(record.id)) {
        QResource::unregisterResource(record.filePath, record.resourceRoot);
        setError(QStringLiteral("Duplicate theme id: %1").arg(record.id));
        return false;
    }
    m_records.insert(record.id, record);
    emit themeInstalled(record.id);
    return true;
}

bool ThemeManager::loadExternalRecord(const QString &filePath, ThemeRecord *record)
{
    const QString hash = sha256File(filePath);
    if (hash.isEmpty()) {
        setError(QStringLiteral("Cannot hash theme package."));
        return false;
    }
    const QString root = QStringLiteral("/__lmgtheme/%1").arg(hash.left(24));
    if (!QResource::registerResource(filePath, root)) {
        setError(QStringLiteral("Theme RCC registration failed."));
        return false;
    }
    if (!validateThemeResources(root)) {
        QResource::unregisterResource(filePath, root);
        return false;
    }
    QFile manifest(QStringLiteral(":%1/theme/theme.json").arg(root));
    if (!manifest.open(QIODevice::ReadOnly)) {
        QResource::unregisterResource(filePath, root);
        setError(QStringLiteral("Theme package does not contain /theme/theme.json."));
        return false;
    }
    if (manifest.size() <= 0 || manifest.size() > kMaxManifestBytes) {
        QResource::unregisterResource(filePath, root);
        setError(QStringLiteral("Theme manifest size is invalid."));
        return false;
    }
    if (!parseDocument(manifest.readAll(), record, QStringLiteral("qrc:%1/theme").arg(root), filePath, false)) {
        QResource::unregisterResource(filePath, root);
        return false;
    }
    record->registered = true;
    return true;
}

bool ThemeManager::validateThemeResources(const QString &resourceRoot)
{
    const QString themeRoot = QStringLiteral(":%1/theme").arg(resourceRoot);
    QDirIterator it(themeRoot, QDir::Files, QDirIterator::Subdirectories);
    bool foundManifest = false;
    while (it.hasNext()) {
        const QString path = it.next();
        if (path.endsWith(QStringLiteral("/theme.json"), Qt::CaseInsensitive))
            foundManifest = true;
        if (!isAllowedThemeResource(path)) {
            setError(QStringLiteral("Theme packages may contain data/assets only; forbidden resource: %1")
                         .arg(QFileInfo(path).fileName()));
            return false;
        }
    }
    if (!foundManifest) {
        setError(QStringLiteral("Theme package manifest is missing."));
        return false;
    }
    return true;
}

bool ThemeManager::parseDocument(const QByteArray &json, ThemeRecord *record, const QString &resourceRoot, const QString &filePath, bool builtIn)
{
    QJsonParseError error{};
    const QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        setError(QStringLiteral("Invalid theme JSON: %1").arg(error.errorString()));
        return false;
    }
    const QVariantMap map = doc.object().toVariantMap();
    const QString id = map.value(QStringLiteral("id")).toString().trimmed();
    const QString name = map.value(QStringLiteral("name")).toString().trimmed();
    static const QRegularExpression idPattern(QStringLiteral("^[a-z0-9][a-z0-9_.-]{1,127}$"));
    if (!idPattern.match(id).hasMatch() || name.isEmpty() || name.size() > 160 || !map.value(QStringLiteral("tokens")).canConvert<QVariantMap>()) {
        setError(QStringLiteral("Theme id, name or tokens are invalid."));
        return false;
    }
    const QVariantMap tokens = map.value(QStringLiteral("tokens")).toMap();
    const QVariantMap aliases = map.value(QStringLiteral("aliases")).toMap();
    const QVariantMap surfaces = map.value(QStringLiteral("surfaces")).toMap();
    if (tokens.size() > kMaxTokens || aliases.size() > kMaxAliases || surfaces.size() > kMaxSurfaces) {
        setError(QStringLiteral("Theme token/alias/surface limits exceeded."));
        return false;
    }
    record->id = id;
    record->name = name;
    record->version = map.value(QStringLiteral("version"), QStringLiteral("1.0.0")).toString().left(64);
    record->publisher = map.value(QStringLiteral("publisher"), QStringLiteral("Local")).toString().left(160);
    record->filePath = filePath;
    record->resourceRoot = resourceRoot;
    record->document = map;
    record->builtIn = builtIn;
    return true;
}

bool ThemeManager::recordContainsKey(const ThemeRecord &record, const QString &key) const
{
    return record.document.value(QStringLiteral("aliases")).toMap().contains(key)
        || record.document.value(QStringLiteral("tokens")).toMap().contains(key);
}

QVariant ThemeManager::resolveKey(const QString &key, int depth) const
{
    if (depth > kMaxAliasDepth)
        return QVariant{};
    const ThemeRecord active = m_records.value(m_activeId);
    const ThemeRecord fallback = m_records.value(m_defaultId);
    const QVariantMap aliases = active.document.value(QStringLiteral("aliases")).toMap();
    const QVariantMap tokens = active.document.value(QStringLiteral("tokens")).toMap();
    const QVariantMap fallbackAliases = fallback.document.value(QStringLiteral("aliases")).toMap();
    const QVariantMap fallbackTokens = fallback.document.value(QStringLiteral("tokens")).toMap();
    QVariant v;
    if (isProtectedPrimitiveKey(key)) {
        if (fallbackAliases.contains(key))
            v = fallbackAliases.value(key);
        else
            v = fallbackTokens.value(key);
    } else if (aliases.contains(key)) {
        v = aliases.value(key);
    } else if (tokens.contains(key)) {
        v = tokens.value(key);
    } else if (fallbackAliases.contains(key)) {
        v = fallbackAliases.value(key);
    } else {
        v = fallbackTokens.value(key);
    }
    if (!v.isValid())
        return QVariant{};
    if (v.metaType().id() == QMetaType::QString) {
        const QString reference = v.toString();
        if (recordContainsKey(active, reference) || recordContainsKey(fallback, reference))
            return resolveKey(reference, depth + 1);
    }
    return v;
}

QVariant ThemeManager::resolveNested(const QVariant &input, const QString &assetRoot, int depth) const
{
    if (depth > kMaxAliasDepth)
        return input;
    if (input.metaType().id() == QMetaType::QVariantMap) {
        QVariantMap out;
        const QVariantMap source = input.toMap();
        for (auto it = source.cbegin(); it != source.cend(); ++it)
            out.insert(it.key(), resolveNested(it.value(), assetRoot, depth + 1));
        return out;
    }
    if (input.metaType().id() == QMetaType::QVariantList) {
        QVariantList out;
        for (const QVariant &v : input.toList())
            out.push_back(resolveNested(v, assetRoot, depth + 1));
        return out;
    }
    if (input.metaType().id() == QMetaType::QString) {
        const QString string = input.toString();
        const ThemeRecord active = m_records.value(m_activeId);
        const ThemeRecord fallback = m_records.value(m_defaultId);
        if (recordContainsKey(active, string) || recordContainsKey(fallback, string))
            return resolveKey(string, depth + 1);
        if (string.startsWith(QStringLiteral("./")))
            return assetRoot + QLatin1Char('/') + string.mid(2);
    }
    return input;
}

QString ThemeManager::sha256File(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QString{};
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file))
        return QString{};
    return QString::fromLatin1(hash.result().toHex());
}

void ThemeManager::setError(const QString &message)
{
    m_lastError = message;
    emit errorOccurred(message);
}

void ThemeManager::bumpRevision()
{
    ++m_revision;
    emit revisionChanged();
}

void ThemeManager::restoreSelection()
{
    QSettings settings;
    const QString saved = settings.value(QStringLiteral("appearance/themeId"), m_defaultId).toString();
    if (m_records.contains(saved))
        m_activeId = saved;
    else
        m_activeId = m_defaultId;
    bumpRevision();
}

void ThemeManager::persistSelection() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("appearance/themeId"), m_activeId);
}
