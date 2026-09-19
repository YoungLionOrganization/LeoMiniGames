// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "JsonTranslator.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QResource>

JsonTranslator::JsonTranslator(QObject *parent) : QTranslator(parent)
{
}

bool JsonTranslator::loadJson(const QString &resourcePath)
{
    clear();
    QResource resource(resourcePath);
    if (!resource.isValid())
        return false;

    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        return false;

    const auto object = document.object();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.value().isString())
            m_strings.insert(it.key(), it.value().toString());
    }
    return true;
}

void JsonTranslator::clear()
{
    m_strings.clear();
}

QString JsonTranslator::translate(const char *context,
                                  const char *sourceText,
                                  const char *disambiguation,
                                  int n) const
{
    Q_UNUSED(context)
    Q_UNUSED(disambiguation)
    Q_UNUSED(n)
    return m_strings.value(QString::fromUtf8(sourceText));
}
