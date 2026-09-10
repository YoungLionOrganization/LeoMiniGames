// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameI18n.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>

GameI18n::GameI18n(QObject *parent) : QObject(parent) {}
QString GameI18n::gameId() const { return m_gameId; }
QString GameI18n::language() const { return m_language; }
QString GameI18n::defaultLocale() const { return m_defaultLocale; }

QStringList GameI18n::capabilities() const
{
    return {QStringLiteral("fallback-chain"), QStringLiteral("runtime-language"),
            QStringLiteral("placeholders"), QStringLiteral("plural-one-other"),
            QStringLiteral("missing-key-signal"), QStringLiteral("locale-normalization")};
}

QString GameI18n::normalized(const QString &locale) const
{
    QString v = locale.trimmed().toLower();
    v.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (v.startsWith(QStringLiteral("zh"))) {
        if (v.contains(QStringLiteral("tw")) || v.contains(QStringLiteral("hk")) || v.contains(QStringLiteral("hant")))
            return QStringLiteral("zh_tw");
        return QStringLiteral("zh_cn");
    }
    static const QRegularExpression safe(QStringLiteral("^[a-z]{2,3}(?:_[a-z0-9]{2,8})?$"));
    return safe.match(v).hasMatch() ? v : QStringLiteral("en");
}

QVariantMap GameI18n::loadLocale(const QString &locale) const
{
    const QString code = normalized(locale);
    if (m_cache.contains(code))
        return m_cache.value(code);
    QVariantMap map;
    if (!m_gameId.isEmpty()) {
        QFile file(QStringLiteral(":/mods/%1/i18n/%2.json").arg(m_gameId, code));
        if (file.open(QIODevice::ReadOnly)) {
            QJsonParseError error;
            const QByteArray bytes = file.read(2 * 1024 * 1024 + 1);
            if (bytes.size() <= 2 * 1024 * 1024) {
                const QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
                if (error.error == QJsonParseError::NoError && doc.isObject())
                    map = doc.object().toVariantMap();
            }
        }
    }
    m_cache.insert(code, map);
    return map;
}

QStringList GameI18n::fallbackChain() const
{
    const QString requested = normalized(m_language);
    const QString base = requested.section(QLatin1Char('_'), 0, 0);
    const QString defaultCode = normalized(m_defaultLocale);
    const QString defaultBase = defaultCode.section(QLatin1Char('_'), 0, 0);
    QStringList chain{requested, base, defaultCode, defaultBase, QStringLiteral("en")};
    QSet<QString> seen;
    QStringList unique;
    for (const QString &locale : chain) {
        if (!locale.isEmpty() && !seen.contains(locale)) {
            seen.insert(locale);
            unique.append(locale);
        }
    }
    return unique;
}

QVariant GameI18n::lookup(const QString &key) const
{
    if (key.isEmpty())
        return QVariant();
    for (const QString &locale : fallbackChain()) {
        const QVariantMap map = loadLocale(locale);
        const auto it = map.constFind(key);
        if (it != map.constEnd() && it.value().isValid())
            return it.value();
    }
    return QVariant();
}

bool GameI18n::hasKey(const QString &key) const
{
    return lookup(key).isValid();
}

QString GameI18n::text(const QString &key)
{
    return text(key, QString{});
}

QString GameI18n::text(const QString &key, const QString &fallback)
{
    const QVariant value = lookup(key);
    if (value.isValid() && value.canConvert<QString>())
        return value.toString();
    emit missingKey(key, normalized(m_language));
    return fallback.isEmpty() ? key : fallback;
}

QString GameI18n::applyArguments(QString value, const QVariantMap &arguments)
{
    int positional = 1;
    for (auto it = arguments.constBegin(); it != arguments.constEnd(); ++it) {
        value.replace(QStringLiteral("%{") + it.key() + QLatin1Char('}'), it.value().toString());
        value.replace(QStringLiteral("{%") + it.key() + QLatin1Char('}'), it.value().toString());
        bool numeric = false;
        const int explicitIndex = it.key().toInt(&numeric);
        if (numeric && explicitIndex > 0)
            value.replace(QStringLiteral("%%1").arg(explicitIndex), it.value().toString());
        else
            value.replace(QStringLiteral("%%1").arg(positional++), it.value().toString());
    }
    return value;
}

QString GameI18n::format(const QString &key, const QVariantMap &arguments)
{
    return format(key, arguments, QString{});
}

QString GameI18n::format(const QString &key, const QVariantMap &arguments, const QString &fallback)
{
    return applyArguments(text(key, fallback), arguments);
}

QString GameI18n::plural(const QString &key, qint64 count)
{
    return plural(key, count, QString{});
}

QString GameI18n::plural(const QString &key, qint64 count, const QString &fallback)
{
    const QString formKey = key + (count == 1 ? QStringLiteral(".one") : QStringLiteral(".other"));
    QVariantMap args{{QStringLiteral("count"), count}, {QStringLiteral("1"), count}};
    if (hasKey(formKey))
        return format(formKey, args, fallback);
    if (hasKey(key))
        return format(key, args, fallback);
    emit missingKey(key, normalized(m_language));
    return applyArguments(fallback.isEmpty() ? key : fallback, args);
}

void GameI18n::activate(const QString &gameId, const QString &defaultLocale, const QStringList &locales)
{
    QStringList normalizedLocales;
    QSet<QString> seen;
    for (const QString &locale : locales) {
        const QString value = normalized(locale);
        if (!seen.contains(value)) {
            seen.insert(value);
            normalizedLocales.append(value);
        }
    }
    const QString normalizedDefault = normalized(defaultLocale);
    if (!seen.contains(normalizedDefault))
        normalizedLocales.append(normalizedDefault);
    const bool different = m_gameId != gameId || m_defaultLocale != normalizedDefault || m_locales != normalizedLocales;
    m_gameId = gameId;
    m_defaultLocale = normalizedDefault;
    m_locales = normalizedLocales;
    m_cache.clear();
    if (different)
        emit changed();
}

void GameI18n::setLanguage(const QString &language)
{
    const QString value = normalized(language);
    if (m_language == value)
        return;
    m_language = value;
    m_cache.clear();
    emit languageChanged();
    emit changed();
}
