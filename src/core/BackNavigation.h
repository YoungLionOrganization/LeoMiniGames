// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>

class BackNavigation final : public QObject
{
    Q_OBJECT

public:
    explicit BackNavigation(QObject *parent = nullptr);

signals:
    void backPressed();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
