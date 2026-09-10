// SPDX-License-Identifier: GPL-3.0-or-later
#include "LanguageManager.h"
#include "SettingsManager.h"

#include <QCoreApplication>
#include <QLocale>
#include <QQmlEngine>
#include <QVariantMap>

namespace {
QString normalizeCode(QString code)
{
    code = code.trimmed().replace(QLatin1Char('-'), QLatin1Char('_')).toLower();
    if (code.startsWith(QStringLiteral("zh"))) {
        if (code.contains(QStringLiteral("tw")) || code.contains(QStringLiteral("hk")) || code.contains(QStringLiteral("hant")))
            return QStringLiteral("zh_tw");
        return QStringLiteral("zh_cn");
    }
    if (code.size() > 2)
        code = code.left(2);
    return code;
}
}

LanguageManager::LanguageManager(SettingsManager *settings, QObject *parent)
    : QObject(parent), m_settings(settings), m_translator(this)
{
}

QString LanguageManager::language() const { return m_language; }

QVariantList LanguageManager::availableLanguages() const
{
    return {
        QVariantMap{{"code", "en"}, {"name", "English"}},
        QVariantMap{{"code", "az"}, {"name", "Azərbaycanca"}},
        QVariantMap{{"code", "tr"}, {"name", "Türkçe"}},
        QVariantMap{{"code", "ru"}, {"name", "Русский"}},
        QVariantMap{{"code", "de"}, {"name", "Deutsch"}},
        QVariantMap{{"code", "fr"}, {"name", "Français"}},
        QVariantMap{{"code", "es"}, {"name", "Español"}},
        QVariantMap{{"code", "pt"}, {"name", "Português"}},
        QVariantMap{{"code", "it"}, {"name", "Italiano"}},
        QVariantMap{{"code", "nl"}, {"name", "Nederlands"}},
        QVariantMap{{"code", "pl"}, {"name", "Polski"}},
        QVariantMap{{"code", "uk"}, {"name", "Українська"}},
        QVariantMap{{"code", "ar"}, {"name", "العربية"}},
        QVariantMap{{"code", "fa"}, {"name", "فارسی"}},
        QVariantMap{{"code", "ja"}, {"name", "日本語"}},
        QVariantMap{{"code", "ko"}, {"name", "한국어"}},
        QVariantMap{{"code", "zh_cn"}, {"name", "简体中文"}},
        QVariantMap{{"code", "zh_tw"}, {"name", "繁體中文"}},
        QVariantMap{{"code", "hi"}, {"name", "हिन्दी"}},
        QVariantMap{{"code", "id"}, {"name", "Bahasa Indonesia"}},
        QVariantMap{{"code", "vi"}, {"name", "Tiếng Việt"}},
        QVariantMap{{"code", "sv"}, {"name", "Svenska"}},
        QVariantMap{{"code", "el"}, {"name", "Ελληνικά"}}
    };
}

QString LanguageManager::text(const QString &source, const QString &languageDependency) const
{
    Q_UNUSED(languageDependency)
    if (m_language == QStringLiteral("en"))
        return source;
    const QByteArray utf8 = source.toUtf8();
    const QString translated = m_translator.translate(nullptr, utf8.constData());
    return translated.isEmpty() ? source : translated;
}

void LanguageManager::setEngine(QQmlEngine *engine) { m_engine = engine; }

bool LanguageManager::isSupported(const QString &code) const
{
    static const QStringList supported = {
        QStringLiteral("en"), QStringLiteral("az"), QStringLiteral("tr"), QStringLiteral("ru"),
        QStringLiteral("de"), QStringLiteral("fr"), QStringLiteral("es"), QStringLiteral("pt"),
        QStringLiteral("it"), QStringLiteral("nl"), QStringLiteral("pl"), QStringLiteral("uk"),
        QStringLiteral("ar"), QStringLiteral("fa"), QStringLiteral("ja"), QStringLiteral("ko"),
        QStringLiteral("zh_cn"), QStringLiteral("zh_tw"), QStringLiteral("hi"), QStringLiteral("id"),
        QStringLiteral("vi"), QStringLiteral("sv"), QStringLiteral("el")
    };
    return supported.contains(code);
}

void LanguageManager::restore()
{
    QString code = normalizeCode(m_settings->value(QStringLiteral("ui/language")).toString());
    if (!isSupported(code)) {
        code = normalizeCode(QLocale::system().name());
        if (!isSupported(code))
            code = QStringLiteral("en");
    }
    setLanguage(code);
}

void LanguageManager::setLanguage(const QString &requested)
{
    const QString code = normalizeCode(requested);
    if (!isSupported(code))
        return;

    if (m_installed) {
        QCoreApplication::removeTranslator(&m_translator);
        m_installed = false;
    }
    m_translator.clear();

    if (code != QStringLiteral("en")) {
        if (m_translator.loadJson(QStringLiteral(":/i18n/%1.json").arg(code))) {
            QCoreApplication::installTranslator(&m_translator);
            m_installed = true;
        }
    }

    const bool changed = (m_language != code);
    m_language = code;
    m_settings->setValue(QStringLiteral("ui/language"), m_language);

    if (m_engine)
        m_engine->retranslate();
    if (changed)
        emit languageChanged();
}
