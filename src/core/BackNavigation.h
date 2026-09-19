// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
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
