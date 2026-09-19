// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QPointer>
#include <QHash>
#include <QUrl>
#include <memory>
class QQmlEngine;
class QQuickItem;
class PluginDiagnostics;

class ExternalGameRuntime final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *item READ item NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString lastError READ lastError NOTIFY changed)
public:
    explicit ExternalGameRuntime(PluginDiagnostics *diagnostics, QObject *parent=nullptr);
    ~ExternalGameRuntime() override;
    QObject *item() const; QString status() const; QString lastError() const;
    void setServices(const QHash<QString,QObject*> &services);
    Q_INVOKABLE bool load(QQuickItem *parentItem,const QUrl &source,const QString &gameId,bool allowHttpsNetwork=true);
    Q_INVOKABLE void unload();
signals: void changed(); void loaded(QObject *item); void failed(const QString &message);
private:
    struct Impl; std::unique_ptr<Impl> m_impl; PluginDiagnostics *m_diagnostics=nullptr; QHash<QString,QObject*> m_services; QPointer<QQuickItem> m_item; QString m_status=QStringLiteral("null"),m_error;
};
