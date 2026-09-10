// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QString>
#include <QVariantList>
class LanguageManager;
class LegacyLanguageFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)
public:
    explicit LegacyLanguageFacade(LanguageManager *language, QObject *parent = nullptr);
    QString language() const;
    QVariantList availableLanguages() const;
    Q_INVOKABLE QString text(const QString &source, const QString &languageDependency = QString{}) const;
public slots:
    void setLanguage(const QString &code);
signals:
    void languageChanged();
private:
    LanguageManager *m_language = nullptr;
};
