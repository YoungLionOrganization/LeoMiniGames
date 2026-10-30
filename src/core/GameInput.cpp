// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameInput.h"
#include <Qt>
#include <QtGlobal>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <cmath>

GameInput::GameInput(QObject *parent) : QObject(parent) { if (qApp) qApp->installEventFilter(this); }
void GameInput::setFocusRoot(QObject *root)
{
    QPointer<GameInput> guard(this);
    QPointer<QObject> nextRoot(root);
    const quint64 generation = m_resetGeneration + 1;
    reset();
    if (guard && m_resetGeneration == generation) m_focusRoot = nextRoot;
}
bool GameInput::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate || event->type() == QEvent::FocusOut) { reset(); return false; }
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease) return false;
    auto *window = qobject_cast<QQuickWindow *>(watched);
    if (!window || !m_focusRoot) return false;
    QQuickItem *focus = window->activeFocusItem();
    for (QQuickItem *item = focus; item; item = item->parentItem()) {
        if (item->inherits("QQuickTextInput") || item->inherits("QQuickTextEdit")) return false;
        if (item == m_focusRoot) {
            auto *key = static_cast<QKeyEvent *>(event);
            handleKey(key->key(), key->nativeScanCode(), key->text(), event->type() == QEvent::KeyPress, key->isAutoRepeat());
            break;
        }
    }
    return false;
}
void GameInput::press(const QString &action) { press(action, 1.0); }
bool GameInput::ownsNotification(const QString &action, quint64 serial) const
{
    const auto pending = m_pendingNotifications.constFind(action);
    return pending != m_pendingNotifications.cend() && pending.value() == serial;
}
void GameInput::press(const QString &action, qreal value)
{
    if (action.isEmpty()) return;
    if (!std::isfinite(value)) { release(action); return; }
    value = qBound<qreal>(-1.0, value, 1.0);
    if (m_values.contains(action) && qFuzzyCompare(m_values.value(action) + 1.0, value + 1.0)) return;
    m_values.insert(action, value);
    // A nested mutation of this action retires its companion notification;
    // mutations of unrelated actions keep their own notification sequence.
    const quint64 serial = ++m_notificationSerial;
    m_pendingNotifications.insert(action, serial);
    QPointer<GameInput> guard(this);
    emit actionPressed(action, value);
    if (!guard || !ownsNotification(action, serial)) return;
    emit actionValueChanged(action, value);
    if (guard && ownsNotification(action, serial)) m_pendingNotifications.remove(action);
}
void GameInput::release(const QString &action)
{
    if (action.isEmpty() || !m_values.contains(action)) return;
    m_values.remove(action);
    const quint64 serial = ++m_notificationSerial;
    m_pendingNotifications.insert(action, serial);
    QPointer<GameInput> guard(this);
    emit actionReleased(action);
    if (!guard || !ownsNotification(action, serial)) return;
    emit actionValueChanged(action, 0.0);
    if (guard && ownsNotification(action, serial)) m_pendingNotifications.remove(action);
}
void GameInput::setValue(const QString &action, qreal value) { if (qFuzzyIsNull(value)) { release(action); return; } press(action, value); }
bool GameInput::isPressed(const QString &action) const { return m_values.contains(action) && !qFuzzyIsNull(m_values.value(action)); }
qreal GameInput::value(const QString &action) const { return m_values.value(action, 0.0); }
QStringList GameInput::capabilities() const { return {QStringLiteral("logical_keys"), QStringLiteral("physical_wasd"), QStringLiteral("actions")}; }

QString GameInput::actionForKey(int key, quint32 nativeScanCode, const QString &text) const
{
    switch (key) {
    case Qt::Key_Left: return QStringLiteral("move_left");
    case Qt::Key_Right: return QStringLiteral("move_right");
    case Qt::Key_Up: return QStringLiteral("up");
    case Qt::Key_Down: return QStringLiteral("down");
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter: return QStringLiteral("fire");
    case Qt::Key_Escape: return QStringLiteral("pause");
    default: break;
    }

    // Native scan codes are centralized here so games never need platform tables.
#if defined(Q_OS_MACOS)
    if (nativeScanCode == 0 && key != 0 && key != Qt::Key_unknown) return QStringLiteral("move_left");   // A
    if (nativeScanCode == 2) return QStringLiteral("move_right");  // D
    if (nativeScanCode == 13) return QStringLiteral("up");         // W
    if (nativeScanCode == 1) return QStringLiteral("down");        // S
#elif defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
    if (nativeScanCode == 30) return QStringLiteral("move_left");  // physical A
    if (nativeScanCode == 32) return QStringLiteral("move_right"); // physical D
    if (nativeScanCode == 17) return QStringLiteral("up");         // physical W
    if (nativeScanCode == 31) return QStringLiteral("down");       // physical S
#elif defined(Q_OS_LINUX)
    const QString backend = QGuiApplication::platformName();
    const bool xkbCodes = backend == QStringLiteral("xcb") || backend.startsWith(QStringLiteral("wayland"));
    const quint32 scan = xkbCodes && nativeScanCode >= 8 ? nativeScanCode - 8 : nativeScanCode;
    if (scan == 30) return QStringLiteral("move_left");
    if (scan == 32) return QStringLiteral("move_right");
    if (scan == 17) return QStringLiteral("up");
    if (scan == 31) return QStringLiteral("down");
#endif

    // Logical fallback keeps standard layouts and synthetic key events working.
    const QString t = text.left(1).toLower();
    if (key == Qt::Key_A || t == QStringLiteral("a")) return QStringLiteral("move_left");
    if (key == Qt::Key_D || t == QStringLiteral("d")) return QStringLiteral("move_right");
    if (key == Qt::Key_W || t == QStringLiteral("w")) return QStringLiteral("up");
    if (key == Qt::Key_S || t == QStringLiteral("s")) return QStringLiteral("down");
    return QString();
}

bool GameInput::handleKey(int key, quint32 nativeScanCode, const QString &text, bool pressed, bool autoRepeat)
{
    const quint64 generation = m_resetGeneration;
    QPointer<GameInput> guard(this);
    const QString action = actionForKey(key, nativeScanCode, text);
    // A native scan identifies the physical key even if its logical Qt key
    // changes while held (for example after switching keyboard layouts).
    const quint64 token = nativeScanCode ? (quint64(nativeScanCode) << 32) : quint32(key);
    if (autoRepeat) return !action.isEmpty() || m_physicalKeys.contains(token);
    if (!pressed) {
        const QString previous = m_physicalKeys.take(token);
        if (!previous.isEmpty() && !m_physicalKeys.values().contains(previous)) release(previous);
        return !previous.isEmpty() || !action.isEmpty();
    }
    if (action.isEmpty()) return false;
    const QString previous = m_physicalKeys.value(token);
    m_physicalKeys.insert(token, action);
    if (!previous.isEmpty() && previous != action && !m_physicalKeys.values().contains(previous)) release(previous);
    if (!guard || m_resetGeneration != generation || m_physicalKeys.value(token) != action) return true;
    press(action);
    return true;
}
void GameInput::reset()
{
    ++m_resetGeneration;
    m_physicalKeys.clear();
    const auto keys = m_values.keys();
    m_values.clear();
    m_pendingNotifications.clear();
    // Reserve the whole old release batch before calling observers. New presses
    // replace these tokens and cannot be released by the retired batch.
    QHash<QString, quint64> retired;
    for (const QString &key : keys) {
        const quint64 serial = ++m_notificationSerial;
        retired.insert(key, serial);
        m_pendingNotifications.insert(key, serial);
    }
    QPointer<GameInput> guard(this);
    for (const QString &key : keys) {
        if (!guard) return;
        const quint64 serial = retired.value(key);
        if (!ownsNotification(key, serial)) continue;
        emit actionReleased(key);
        if (!guard) return;
        if (!ownsNotification(key, serial)) continue;
        emit actionValueChanged(key, 0.0);
        if (guard && ownsNotification(key, serial)) m_pendingNotifications.remove(key);
    }
}
