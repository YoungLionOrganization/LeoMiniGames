// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameSave.h"
#include "AppPaths.h"

#include <QCborMap>
#include <QCborValue>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QJSEngine>

namespace {
const QByteArray kMagic("LMGSAVE\0", 8);
constexpr quint32 kFormatVersion = 1;
constexpr qint64 kMaxSaveBytes = 32LL * 1024LL * 1024LL;

QByteArray u32be(quint32 value)
{
    QByteArray out(4, '\0');
    out[0] = static_cast<char>((value >> 24) & 0xff);
    out[1] = static_cast<char>((value >> 16) & 0xff);
    out[2] = static_cast<char>((value >> 8) & 0xff);
    out[3] = static_cast<char>(value & 0xff);
    return out;
}
quint32 readU32be(const QByteArray &b)
{
    if (b.size() != 4) return 0;
    return (static_cast<quint8>(b[0]) << 24) | (static_cast<quint8>(b[1]) << 16) |
           (static_cast<quint8>(b[2]) << 8) | static_cast<quint8>(b[3]);
}
}

GameSave::GameSave(AppPaths *paths, QObject *parent) : QObject(parent), m_paths(paths) {}
void GameSave::setEngine(QJSEngine *engine) { m_engine = engine; }
QString GameSave::gameId() const { return m_gameId; }
QString GameSave::currentSlot() const { return m_currentSlot; }
int GameSave::schemaVersion() const { return m_schemaVersion; }
int GameSave::loadedSchemaVersion() const { return m_loadedSchemaVersion; }
QString GameSave::lastError() const { return m_lastError; }

bool GameSave::safeToken(const QString &value)
{
    static const QRegularExpression rx(QStringLiteral("^[A-Za-z0-9_.-]{1,64}$"));
    return rx.match(value).hasMatch();
}

QString GameSave::gameDir() const
{
    if (!m_paths || !safeToken(m_gameId)) return QString{};
    const QString path = m_paths->saves() + QLatin1Char('/') + m_gameId;
    QDir().mkpath(path);
    return path;
}
QString GameSave::slotPath(const QString &slot) const { if(!safeToken(slot))return QString{};const QString dir=gameDir();return dir.isEmpty()?QString{}:dir+QLatin1Char('/')+slot+QStringLiteral(".lmgsave"); }
QString GameSave::backupPath(const QString &slot) const { const QString p = slotPath(slot); return p.isEmpty() ? QString{} : p + QStringLiteral(".bak"); }

QVariant GameSave::get(const QString &key) const { return get(key, QVariant{}); }
QVariant GameSave::get(const QString &key, const QVariant &fallback) const { return m_payload.value(key, fallback); }
void GameSave::set(const QString &key, const QVariant &value) { if (!key.isEmpty() && m_payload.value(key) != value) { m_payload.insert(key, value); m_dirty = true; } }
void GameSave::remove(const QString &key) { if (m_payload.remove(key) > 0) m_dirty = true; }
bool GameSave::contains(const QString &key) const { return m_payload.contains(key); }
void GameSave::clearMemory() { if (!m_payload.isEmpty()) { m_payload.clear(); m_dirty = true; } }

void GameSave::setError(const QString &error)
{
    if (m_lastError == error) return;
    m_lastError = error;
    emit lastErrorChanged();
}

bool GameSave::atomicCopy(const QString &source, const QString &destination) const
{
    QFile in(source);
    if (!in.open(QIODevice::ReadOnly)) return false;
    QSaveFile out(destination);
    if (!out.open(QIODevice::WriteOnly)) return false;
    while (!in.atEnd()) {
        const QByteArray chunk = in.read(64 * 1024);
        if (chunk.isEmpty() && in.error() != QFile::NoError) return false;
        if (out.write(chunk) != chunk.size()) return false;
    }
    return out.commit();
}

GameSave::ReadResult GameSave::readFile(const QString &path) const
{
    ReadResult result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { result.error = QStringLiteral("Save file not found."); return result; }
    if (file.read(8) != kMagic) { result.error = QStringLiteral("Invalid save magic."); return result; }
    const quint32 headerSize = readU32be(file.read(4));
    if (headerSize == 0 || headerSize > 1024 * 1024) { result.error = QStringLiteral("Invalid save header."); return result; }
    QCborParserError err;
    const QCborValue headerValue = QCborValue::fromCbor(file.read(headerSize), &err);
    if (err.error != QCborError::NoError || !headerValue.isMap()) { result.error = QStringLiteral("Invalid CBOR header."); return result; }
    if (file.bytesAvailable() > kMaxSaveBytes) { result.error = QStringLiteral("Save payload is too large."); return result; }
    const QByteArray payloadBytes = file.readAll();
    const QCborMap header = headerValue.toMap();
    if (header.value(QStringLiteral("formatVersion")).toInteger() != kFormatVersion) { result.error = QStringLiteral("Unsupported save format."); return result; }
    if (header.value(QStringLiteral("gameId")).toString() != m_gameId) { result.error = QStringLiteral("Save belongs to another game."); return result; }
    const QByteArray expected = header.value(QStringLiteral("checksum")).toByteArray();
    const QByteArray actual = QCryptographicHash::hash(payloadBytes, QCryptographicHash::Sha256);
    if (expected.size() != 32 || expected != actual) { result.error = QStringLiteral("Save checksum failed."); return result; }
    const QCborValue payload = QCborValue::fromCbor(payloadBytes, &err);
    if (err.error != QCborError::NoError || !payload.isMap()) { result.error = QStringLiteral("Invalid save payload."); return result; }
    result.payload = payload.toVariant().toMap();
    result.schema = qMax(1, static_cast<int>(header.value(QStringLiteral("schemaVersion")).toInteger(1)));
    result.ok = true;
    return result;
}

bool GameSave::writeFile(const QString &path, const QVariantMap &payload, int schema, bool makeBackup)
{
    if (path.isEmpty()) return false;
    if (makeBackup && QFile::exists(path)) {
        const ReadResult existing = readFile(path);
        if (existing.ok && !atomicCopy(path, path + QStringLiteral(".bak"))) {
            setError(QStringLiteral("Could not create save backup."));
            return false;
        }
    }
    const QByteArray payloadBytes = QCborValue::fromVariant(payload).toCbor();
    if (payloadBytes.size() > kMaxSaveBytes) { setError(QStringLiteral("Save payload is too large.")); return false; }
    QCborMap header;
    header.insert(QStringLiteral("formatVersion"), static_cast<qint64>(kFormatVersion));
    header.insert(QStringLiteral("gameId"), m_gameId);
    header.insert(QStringLiteral("gameVersion"), m_gameVersion);
    header.insert(QStringLiteral("schemaVersion"), schema);
    header.insert(QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
    header.insert(QStringLiteral("checksum"), QCryptographicHash::hash(payloadBytes, QCryptographicHash::Sha256));
    const QByteArray headerBytes = QCborValue(header).toCbor();

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { setError(QStringLiteral("Could not open save file.")); return false; }
    if (file.write(kMagic) != kMagic.size() || file.write(u32be(static_cast<quint32>(headerBytes.size()))) != 4 ||
        file.write(headerBytes) != headerBytes.size() || file.write(payloadBytes) != payloadBytes.size()) {
        file.cancelWriting(); setError(QStringLiteral("Could not write save file.")); return false;
    }
    if (!file.commit()) { setError(QStringLiteral("Atomic save commit failed.")); return false; }
    setError(QString{});
    return true;
}

bool GameSave::save() { return save(QStringLiteral("autosave")); }

bool GameSave::save(const QString &slot)
{
    const QString path = slotPath(slot);
    if (path.isEmpty()) { setError(QStringLiteral("Invalid save slot.")); return false; }
    if (!writeFile(path, m_payload, m_schemaVersion, true)) return false;
    m_currentSlot = slot;
    m_loadedSchemaVersion = m_schemaVersion;
    m_dirty = false;
    emit slotChanged(); emit loadedChanged(); emit saved(slot);
    return true;
}

bool GameSave::applyMigrations(QVariantMap &payload, int &version)
{
    while (version < m_schemaVersion) {
        if (!m_migrations.contains(version)) { setError(QStringLiteral("Missing save migration from schema %1.").arg(version)); return false; }
        const Migration migration = m_migrations.value(version);
        if (migration.to <= version || !migration.callback.isCallable()) { setError(QStringLiteral("Invalid save migration.")); return false; }
        if (!m_engine) { setError(QStringLiteral("Save migration has no JavaScript engine.")); return false; }
        QJSValueList args; args << m_engine->toScriptValue(payload);
        const QJSValue output = migration.callback.call(args);
        if (output.isError() || !output.isObject()) { setError(QStringLiteral("Save migration failed at schema %1.").arg(version)); return false; }
        payload = output.toVariant().toMap();
        version = migration.to;
    }
    return version == m_schemaVersion;
}

bool GameSave::load() { return load(QStringLiteral("autosave")); }

bool GameSave::load(const QString &slot)
{
    const QString primary = slotPath(slot);
    if (primary.isEmpty()) { setError(QStringLiteral("Invalid save slot.")); return false; }
    ReadResult result = readFile(primary);
    bool usedBackup = false;
    if (!result.ok) {
        const ReadResult backup = readFile(backupPath(slot));
        if (!backup.ok) { setError(result.error); return false; }
        result = backup;
        usedBackup = true;
    }
    QVariantMap payload = result.payload;
    int version = result.schema;
    const int sourceSchema = version;
    if (version > m_schemaVersion) { setError(QStringLiteral("Save schema is newer than this game.")); return false; }
    if (version < m_schemaVersion && !applyMigrations(payload, version)) return false;
    m_payload = payload;
    m_currentSlot = slot;
    m_loadedSchemaVersion = version;
    m_dirty = usedBackup || sourceSchema != version;
    setError(QString{});
    emit slotChanged(); emit loadedChanged(); emit loaded(slot);
    return true;
}

bool GameSave::createSlot(const QString &slot) { if (!safeToken(slot)) { setError(QStringLiteral("Invalid save slot.")); return false; } const QVariantMap old = m_payload; const bool oldDirty = m_dirty; m_payload.clear(); m_dirty = true; const bool ok = save(slot); m_payload = old; m_dirty = oldDirty; return ok; }
bool GameSave::deleteSlot(const QString &slot) { const QString p = slotPath(slot); if (p.isEmpty()) return false; QFile::remove(p + QStringLiteral(".bak")); return !QFile::exists(p) || QFile::remove(p); }
QStringList GameSave::listSlots() const { QStringList out; const QString dirPath=gameDir(); if(dirPath.isEmpty())return out; QDir dir(dirPath); const auto files = dir.entryList({QStringLiteral("*.lmgsave")}, QDir::Files, QDir::Name); for (const QString &f : files) out << f.left(f.size() - 8); return out; }
bool GameSave::autosave() { if (m_gameId.isEmpty() || !m_dirty) return true; return save(QStringLiteral("autosave")); }
bool GameSave::forceSave() { return forceSave(QStringLiteral("autosave")); }
bool GameSave::forceSave(const QString &slot) { if (m_gameId.isEmpty()) return true; return save(slot); }

bool GameSave::restoreBackup(const QString &slot)
{
    const QString bak = backupPath(slot); const QString primary = slotPath(slot);
    if (bak.isEmpty() || !readFile(bak).ok) { setError(QStringLiteral("No valid backup exists.")); return false; }
    if (!atomicCopy(bak, primary)) { setError(QStringLiteral("Backup restore failed.")); return false; }
    setError(QString{}); emit backupRestored(slot); return load(slot);
}

bool GameSave::registerMigration(int fromVersion, int toVersion, const QJSValue &callback)
{
    if (fromVersion < 1 || toVersion <= fromVersion || !callback.isCallable()) return false;
    m_migrations.insert(fromVersion, Migration{toVersion, callback}); return true;
}

void GameSave::activate(const QString &gameId, const QString &gameVersion, int schemaVersion)
{
    if (m_gameId == gameId && m_gameVersion == gameVersion && m_schemaVersion == schemaVersion) return;
    m_gameId = gameId; m_gameVersion = gameVersion; m_schemaVersion = qMax(1, schemaVersion);
    m_loadedSchemaVersion = m_schemaVersion; m_currentSlot.clear(); m_payload.clear(); m_dirty = false; m_migrations.clear(); setError(QString{});
    emit gameChanged(); emit slotChanged(); emit loadedChanged();
}
