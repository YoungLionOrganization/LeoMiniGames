// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QPointer>

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
    void invoke(const char *method);
    QPointer<QObject> m_object; QString m_gameId;
};
