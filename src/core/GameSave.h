// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QHash>
#include <QJSValue>
#include <QVariantMap>

class AppPaths;
class QJSEngine;

class GameSave final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY gameChanged)
    Q_PROPERTY(QString currentSlot READ currentSlot NOTIFY slotChanged)
    Q_PROPERTY(int schemaVersion READ schemaVersion NOTIFY gameChanged)
    Q_PROPERTY(int loadedSchemaVersion READ loadedSchemaVersion NOTIFY loadedChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
public:
    explicit GameSave(AppPaths *paths, QObject *parent = nullptr);
    QString gameId() const;
    QString currentSlot() const;
    int schemaVersion() const;
    int loadedSchemaVersion() const;
    QString lastError() const;

    Q_INVOKABLE QVariant get(const QString &key) const;
    Q_INVOKABLE QVariant get(const QString &key, const QVariant &fallback) const;
    Q_INVOKABLE void set(const QString &key, const QVariant &value);
    Q_INVOKABLE void remove(const QString &key);
    Q_INVOKABLE bool contains(const QString &key) const;
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool save(const QString &slot);
    Q_INVOKABLE bool load();
    Q_INVOKABLE bool load(const QString &slot);
    Q_INVOKABLE bool createSlot(const QString &slot);
    Q_INVOKABLE bool deleteSlot(const QString &slot);
    Q_INVOKABLE QStringList listSlots() const;
    Q_INVOKABLE bool autosave();
    Q_INVOKABLE bool forceSave();
    Q_INVOKABLE bool forceSave(const QString &slot);
    Q_INVOKABLE bool restoreBackup(const QString &slot);
    Q_INVOKABLE bool registerMigration(int fromVersion, int toVersion, const QJSValue &callback);
    Q_INVOKABLE void clearMemory();

    void activate(const QString &gameId, const QString &gameVersion, int schemaVersion);
    void setEngine(QJSEngine *engine);

signals:
    void gameChanged();
    void slotChanged();
    void loadedChanged();
    void lastErrorChanged();
    void saved(const QString &slot);
    void loaded(const QString &slot);
    void backupRestored(const QString &slot);

private:
    struct ReadResult { bool ok = false; QVariantMap payload; int schema = 1; QString error; };
    static bool safeToken(const QString &value);
    QString gameDir() const;
    QString slotPath(const QString &slot) const;
    QString backupPath(const QString &slot) const;
    ReadResult readFile(const QString &path) const;
    bool writeFile(const QString &path, const QVariantMap &payload, int schema, bool makeBackup);
    bool atomicCopy(const QString &source, const QString &destination) const;
    bool applyMigrations(QVariantMap &payload, int &version);
    void setError(const QString &error);

    AppPaths *m_paths = nullptr;
    QJSEngine *m_engine = nullptr;
    QString m_gameId;
    QString m_gameVersion;
    QString m_currentSlot;
    int m_schemaVersion = 1;
    int m_loadedSchemaVersion = 1;
    QVariantMap m_payload;
    bool m_dirty = false;
    QString m_lastError;
    struct Migration { int to = 0; QJSValue callback; };
    QHash<int, Migration> m_migrations;
};
