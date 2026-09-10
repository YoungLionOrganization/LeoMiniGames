// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QString>

class AppPaths final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appData READ appData CONSTANT)
    Q_PROPERTY(QString plugins READ plugins CONSTANT)
    Q_PROPERTY(QString saves READ saves CONSTANT)
    Q_PROPERTY(QString cache READ cache CONSTANT)
    Q_PROPERTY(QString mods READ mods CONSTANT)
    Q_PROPERTY(QString themes READ themes CONSTANT)

public:
    explicit AppPaths(QObject *parent = nullptr);

    QString appData() const;
    QString plugins() const;
    QString saves() const;
    QString cache() const;
    QString mods() const;
    QString themes() const;

private:
    static QString ensure(const QString &path);
};
