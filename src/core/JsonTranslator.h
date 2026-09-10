// SPDX-License-Identifier: GPL-3.0-or-later
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
