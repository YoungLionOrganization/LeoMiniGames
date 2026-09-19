// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "LegacyLanguageFacade.h"
#include "LanguageManager.h"
LegacyLanguageFacade::LegacyLanguageFacade(LanguageManager *language, QObject *parent)
    : QObject(parent), m_language(language)
{
    if (m_language)
        connect(m_language, &LanguageManager::languageChanged, this, &LegacyLanguageFacade::languageChanged);
}
QString LegacyLanguageFacade::language() const { return m_language ? m_language->language() : QStringLiteral("en"); }
QVariantList LegacyLanguageFacade::availableLanguages() const { return m_language ? m_language->availableLanguages() : QVariantList{}; }
QString LegacyLanguageFacade::text(const QString &source, const QString &dependency) const
{
    return m_language ? m_language->text(source, dependency.isEmpty() ? m_language->language() : dependency) : source;
}
void LegacyLanguageFacade::setLanguage(const QString &code) { if (m_language) m_language->setLanguage(code); }
