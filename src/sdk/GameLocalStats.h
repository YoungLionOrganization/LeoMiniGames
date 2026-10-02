// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QCborValue>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QVariantMap>
#include <QRegularExpression>

// Internal helper for built-in games. Persistent game statistics live in
// saves/<gameId>/local_stats.cbor, never in QSettings. QSettings is touched only to
// migrate v0.5.x legacy keys once, then those keys are removed.
class GameLocalStats final
{
public:
    static QVariant value(const QString &gameId, const QString &key, const QVariant &fallback = QVariant())
    {
        return load(gameId).value(key, fallback);
    }

    static void setValues(const QString &gameId, const QVariantMap &values)
    {
        QVariantMap data = load(gameId);
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            data.insert(it.key(), it.value());
        if (!save(gameId, data))
            qWarning() << "Could not persist built-in game statistics:" << gameId;
    }

    static void migrateLegacy(const QString &gameId, const QHash<QString, QString> &legacyToNew)
    {
        QVariantMap data = load(gameId);
        QSettings settings;
        bool changed = false;
        QStringList migrated;
        for (auto it = legacyToNew.cbegin(); it != legacyToNew.cend(); ++it) {
            if (!data.contains(it.value()) && settings.contains(it.key())) {
                data.insert(it.value(), settings.value(it.key()));
                changed = true;
            }
            if (settings.contains(it.key()))
                migrated.append(it.key());
        }
        if (changed && !save(gameId, data))
            return;
        for (const QString &key : migrated)
            settings.remove(key);
    }

private:
    static QString filePath(const QString &gameId)
    {
        const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        static const QRegularExpression token(QStringLiteral("\\A[a-zA-Z0-9][a-zA-Z0-9_-]{0,127}\\z"));
        if (root.isEmpty() || !token.match(gameId).hasMatch()) return QString{};
        const QString dir = root + QStringLiteral("/saves/") + gameId;
        if (!QDir().mkpath(dir)) return QString{};
        return dir + QStringLiteral("/local_stats.cbor");
    }

    static QVariantMap load(const QString &gameId)
    {
        QFile file(filePath(gameId));
        if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024)
            return QVariantMap{};
        QCborParserError error;
        const QCborValue value = QCborValue::fromCbor(file.readAll(), &error);
        return error.error == QCborError::NoError && value.isMap()
            ? value.toVariant().toMap()
            : QVariantMap{};
    }

    static bool save(const QString &gameId, const QVariantMap &data)
    {
        QSaveFile file(filePath(gameId));
        if (!file.open(QIODevice::WriteOnly))
            return false;
        const QByteArray bytes = QCborValue::fromVariant(data).toCbor();
        if (bytes.size() > 1024 * 1024 || file.write(bytes) != bytes.size()) {
            file.cancelWriting();
            return false;
        }
        return file.commit();
    }
};
