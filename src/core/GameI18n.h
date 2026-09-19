// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QHash>
#include <QVariantMap>

class GameI18n final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY changed)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString defaultLocale READ defaultLocale NOTIFY changed)
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QStringList locales READ locales NOTIFY changed)
public:
    explicit GameI18n(QObject *parent = nullptr);
    QString gameId() const;
    QString language() const;
    QString defaultLocale() const;
    bool ready() const { return true; }
    QString version() const { return QStringLiteral("0.7"); }
    QStringList locales() const { return m_locales; }

    Q_INVOKABLE QStringList capabilities() const;
    Q_INVOKABLE bool hasKey(const QString &key) const;
    Q_INVOKABLE QString text(const QString &key);
    Q_INVOKABLE QString text(const QString &key, const QString &fallback);
    Q_INVOKABLE QString format(const QString &key, const QVariantMap &arguments);
    Q_INVOKABLE QString format(const QString &key, const QVariantMap &arguments, const QString &fallback);
    Q_INVOKABLE QString plural(const QString &key, qint64 count);
    Q_INVOKABLE QString plural(const QString &key, qint64 count, const QString &fallback);
    void activate(const QString &gameId, const QString &defaultLocale, const QStringList &locales);
public slots:
    void setLanguage(const QString &language);
signals:
    void changed();
    void languageChanged();
    void missingKey(const QString &key, const QString &locale);
private:
    QVariantMap loadLocale(const QString &locale) const;
    QVariant lookup(const QString &key) const;
    QString normalized(const QString &locale) const;
    QStringList fallbackChain() const;
    static QString applyArguments(QString value, const QVariantMap &arguments);
    QString m_gameId;
    QString m_language = QStringLiteral("en");
    QString m_defaultLocale = QStringLiteral("en");
    QStringList m_locales;
    mutable QHash<QString, QVariantMap> m_cache;
};
