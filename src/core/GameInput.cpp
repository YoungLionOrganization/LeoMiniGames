// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameInput.h"
#include <Qt>
#include <QtGlobal>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>

GameInput::GameInput(QObject *parent) : QObject(parent) { if (qApp) qApp->installEventFilter(this); }
void GameInput::setFocusRoot(QObject *root) { reset(); m_focusRoot = root; }
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
void GameInput::press(const QString &action, qreal value)
{
    if (action.isEmpty()) return;
    value = qBound<qreal>(-1.0, value, 1.0);
    if (m_values.contains(action) && qFuzzyCompare(m_values.value(action) + 1.0, value + 1.0)) return;
    m_values.insert(action, value);
    emit actionPressed(action, value);
    emit actionValueChanged(action, value);
}
void GameInput::release(const QString &action)
{
    if (action.isEmpty() || !m_values.contains(action)) return;
    m_values.remove(action);
    emit actionReleased(action);
    emit actionValueChanged(action, 0.0);
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
    const QString action = actionForKey(key, nativeScanCode, text);
    if (action.isEmpty()) return false;
    if (autoRepeat) return true;
    const quint64 token = (quint64(nativeScanCode) << 32) | quint32(key);
    if (pressed) { m_physicalKeys.insert(token, action); press(action); }
    else {
        const QString previous = m_physicalKeys.take(token);
        if (!previous.isEmpty() && !m_physicalKeys.values().contains(previous)) release(previous);
    }
    return true;
}
void GameInput::reset() { m_physicalKeys.clear(); const auto keys = m_values.keys(); m_values.clear(); for (const QString &k : keys) { emit actionReleased(k); emit actionValueChanged(k, 0.0); } }
