// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QVariantMap>

class GameRuntime final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString apiVersion READ apiVersion CONSTANT)
    Q_PROPERTY(QStringList capabilities READ capabilities CONSTANT)
public:
    explicit GameRuntime(QObject *parent = nullptr);
    bool ready() const { return true; }
    QString version() const;
    QString apiVersion() const { return QStringLiteral("0.7"); }
    QStringList capabilities() const;
    Q_INVOKABLE bool supports(const QString &capability) const;
    Q_INVOKABLE QVariantMap checkCompatibility(const QString &apiVersion,
                                                const QString &minApiVersion,
                                                const QStringList &requiredCapabilities = {}) const;
};
