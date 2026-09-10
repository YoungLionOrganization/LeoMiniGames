// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantList>

#include "JsonTranslator.h"

class QQmlEngine;
class SettingsManager;

class LanguageManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)

public:
    explicit LanguageManager(SettingsManager *settings, QObject *parent = nullptr);

    QString language() const;
    QVariantList availableLanguages() const;
    Q_INVOKABLE QString text(const QString &source, const QString &languageDependency) const;
    void setEngine(QQmlEngine *engine);
    void restore();

public slots:
    void setLanguage(const QString &code);

signals:
    void languageChanged();

private:
    bool isSupported(const QString &code) const;

    SettingsManager *m_settings = nullptr;
    QPointer<QQmlEngine> m_engine;
    JsonTranslator m_translator;
    QString m_language = QStringLiteral("en");
    bool m_installed = false;
};
