// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QString>

class PluginDiagnostics;

class GameLoggerFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
public:
    explicit GameLoggerFacade(PluginDiagnostics *diagnostics, QObject *parent = nullptr);
    QString lastError() const;
    Q_INVOKABLE void log(const QString &level, const QString &message);
    Q_INVOKABLE void log(const QString &level, const QString &message, const QString &source);
    Q_INVOKABLE void log(const QString &level, const QString &message, const QString &source, int line);
signals:
    void lastErrorChanged();
private:
    PluginDiagnostics *m_diagnostics = nullptr;
};
