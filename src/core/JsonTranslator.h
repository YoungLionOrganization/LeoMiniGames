// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once

#include <QHash>
#include <QTranslator>

class JsonTranslator final : public QTranslator
{
    Q_OBJECT

public:
    explicit JsonTranslator(QObject *parent = nullptr);

    bool loadJson(const QString &resourcePath);
    void clear();

    QString translate(const char *context,
                      const char *sourceText,
                      const char *disambiguation = nullptr,
                      int n = -1) const override;

private:
    QHash<QString, QString> m_strings;
};
