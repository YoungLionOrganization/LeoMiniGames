// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QVariant>
class GameEvents final : public QObject
{
    Q_OBJECT
public:
    explicit GameEvents(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE void emitEvent(const QString &name, const QVariant &payload = QVariant{}) { if (!name.trimmed().isEmpty()) emit eventEmitted(name, payload); }
signals:
    void eventEmitted(const QString &name, const QVariant &payload);
};
