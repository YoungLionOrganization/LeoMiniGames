// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
class Haptics final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
public:
    explicit Haptics(QObject *parent=nullptr);
    bool available() const;
    Q_INVOKABLE void light(); Q_INVOKABLE void medium(); Q_INVOKABLE void heavy(); Q_INVOKABLE void pulse(const QString &level);
private: void vibrate(int milliseconds);
};
