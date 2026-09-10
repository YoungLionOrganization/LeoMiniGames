// SPDX-License-Identifier: GPL-3.0-or-later
#include "BackNavigation.h"

#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>

BackNavigation::BackNavigation(QObject *parent)
    : QObject(parent)
{
    if (QCoreApplication::instance())
        QCoreApplication::instance()->installEventFilter(this);
}

bool BackNavigation::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return false;

    const auto *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() != Qt::Key_Back)
        return false;

    // Consume both halves of Android's Back key so the Activity does not finish.
    // Emit only once, on release, to avoid double navigation.
    if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat())
        emit backPressed();
    return true;
}
