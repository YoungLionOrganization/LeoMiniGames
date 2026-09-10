// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

struct RccPackageInspection
{
    bool valid = false;
    bool legacyLayout = false;
    QString packageId;
    QString entryPath;
    QString mountRoot;
    QString resourceRoot;
    QString error;
    QStringList resources;
    QJsonObject manifest;
};

class RccPackageInspector final
{
public:
    static RccPackageInspection inspect(const QString &rccFile,
                                        const QString &expectedId = QString{},
                                        const QString &expectedEntry = QStringLiteral("Main.qml"));
    static bool mount(const QString &rccFile, const RccPackageInspection &inspection);
    static void unmount(const QString &rccFile, const RccPackageInspection &inspection);
    static bool validId(const QString &id);
    static bool safeRelativePath(const QString &path);
};
