// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "AppPaths.h"

#include <QDir>
#include <QStandardPaths>

AppPaths::AppPaths(QObject *parent) : QObject(parent)
{
    appData();
    plugins();
    saves();
    cache();
    mods();
    themes();
}

QString AppPaths::ensure(const QString &path)
{
    if (!path.isEmpty())
        QDir().mkpath(path);
    return QDir::cleanPath(path);
}

QString AppPaths::appData() const
{
    return ensure(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
}

QString AppPaths::plugins() const
{
    return ensure(appData() + QStringLiteral("/plugins"));
}

QString AppPaths::saves() const
{
    return ensure(appData() + QStringLiteral("/saves"));
}

QString AppPaths::cache() const
{
    return ensure(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
}

QString AppPaths::mods() const
{
    return ensure(appData() + QStringLiteral("/mods"));
}

QString AppPaths::themes() const
{
    return ensure(appData() + QStringLiteral("/themes"));
}
