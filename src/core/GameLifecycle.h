// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QByteArray>

class GameLifecycle final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY gameChanged)
public:
    explicit GameLifecycle(QObject *parent = nullptr);
    QString gameId() const;
    Q_INVOKABLE void attach(QObject *gameObject, const QString &gameId);
    Q_INVOKABLE void load(); Q_INVOKABLE void start(); Q_INVOKABLE void pause(); Q_INVOKABLE void resume();
    Q_INVOKABLE void background(); Q_INVOKABLE void foreground(); Q_INVOKABLE void save(); Q_INVOKABLE void close(); Q_INVOKABLE void unload();
signals:
    void gameChanged(); void loaded(); void started(); void paused(); void resumed(); void backgrounded(); void foregrounded(); void saveRequested(); void closed(); void unloaded();
private:
    bool invoke(const char *method);
    bool active() const;
    quint64 m_generation = 0;
    QSet<QByteArray> m_invoking;
    bool m_paused = false;
    bool m_started = false;
    bool m_closed = false;
    QPointer<QObject> m_object; QString m_gameId;
};
