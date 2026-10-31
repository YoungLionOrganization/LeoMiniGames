// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/GameInput.h"
#include <QGuiApplication>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QDebug>
#include <QList>
#include <QPair>
#include <QPointer>
#include <cstdlib>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    int assertions = 0;
    const auto check = [&](bool condition, const char *message) {
        ++assertions;
        if (!condition) { qCritical("FAIL: %s", message); std::exit(1); }
    };
    {
        GameInput input;
        QList<qreal> values;
        QObject::connect(&input, &GameInput::actionPressed, &input, [&](const QString &action) { input.release(action); });
        QObject::connect(&input, &GameInput::actionValueChanged, &input, [&](const QString &, qreal value) { values.append(value); });
        input.press("jump");
        check(!input.isPressed("jump") && values == QList<qreal>{0.0}, "release in pressed observer suppresses retired positive value");
    }
    {
        GameInput input;
        input.press("jump");
        QList<qreal> values;
        QObject::connect(&input, &GameInput::actionReleased, &input, [&](const QString &action) { input.press(action); });
        QObject::connect(&input, &GameInput::actionValueChanged, &input, [&](const QString &, qreal value) { values.append(value); });
        input.release("jump");
        check(input.isPressed("jump") && values == QList<qreal>{1.0}, "replacement press suppresses retired zero value");
    }
    {
        GameInput input;
        QList<qreal> values;
        bool replaced = false;
        QObject::connect(&input, &GameInput::actionPressed, &input, [&](const QString &action) {
            if (replaced) return;
            replaced = true;
            input.release(action);
            input.press(action);
        });
        QObject::connect(&input, &GameInput::actionValueChanged, &input, [&](const QString &, qreal value) { values.append(value); });
        input.press("jump");
        check(input.isPressed("jump") && values == QList<qreal>{0.0, 1.0}, "equal-valued replacement still retires the original press");
    }
    {
        GameInput input;
        QList<QString> actions;
        QObject::connect(&input, &GameInput::actionPressed, &input, [&](const QString &action) {
            if (action == "left") input.press("right");
        });
        QObject::connect(&input, &GameInput::actionValueChanged, &input, [&](const QString &action) { actions.append(action); });
        input.press("left");
        check(actions == QList<QString>{"right", "left"}, "unrelated actions retain their own notifications");
    }
    {
        GameInput input;
        input.press("left"); input.press("right");
        int releases = 0;
        QList<qreal> values;
        QObject::connect(&input, &GameInput::actionReleased, &input, [&](const QString &) {
            ++releases;
            if (releases == 1) { input.press("left"); input.press("right"); }
        });
        QObject::connect(&input, &GameInput::actionValueChanged, &input, [&](const QString &, qreal value) { values.append(value); });
        input.reset();
        check(releases == 1 && values == QList<qreal>{1.0, 1.0}, "reset cannot release replacement actions from its old batch");
        check(input.isPressed("left") && input.isPressed("right"), "replacement input state survives reset observers");
    }
    for (const int phase : {0, 1, 2}) {
        QPointer<GameInput> input = new GameInput;
        if (phase != 0) input->press("jump");
        if (phase == 0) {
            QObject::connect(input, &GameInput::actionPressed, input, [&] { delete input.data(); });
            input->press("jump");
        } else {
            QObject::connect(input, &GameInput::actionReleased, input, [&] { delete input.data(); });
            if (phase == 1) input->release("jump"); else input->reset();
        }
        check(!input, "input observers may retire the service during notifications");
    }
    {
        GameInput input;
        input.handleKey(Qt::Key_W, 17, "w", true);
        QObject::connect(&input, &GameInput::actionReleased, &input, [&] { input.reset(); });
        input.handleKey(Qt::Key_Left, 17, "", true);
        check(!input.isPressed("up") && !input.isPressed("move_left"),
              "retired physical key remapping cannot press into a replacement session");
    }
    {
        GameInput input;
        input.handleKey(Qt::Key_W, 17, "w", true);
        QObject::connect(&input, &GameInput::actionReleased, &input, [&](const QString &action) {
            if (action == "up") input.handleKey(Qt::Key_Right, 17, "", true);
        });
        input.handleKey(Qt::Key_Left, 17, "", true);
        check(input.isPressed("move_right") && !input.isPressed("move_left"),
              "newest physical mapping wins after a released-action observer");
    }
    {
        QQuickWindow window;
        QQuickItem first(window.contentItem());
        QQuickItem replacement(window.contentItem());
        window.show(); replacement.forceActiveFocus(); app.processEvents();
        GameInput input;
        input.setFocusRoot(&first);
        input.press("held");
        bool replaced = false;
        QObject::connect(&input, &GameInput::actionReleased, &input, [&] {
            if (!replaced) { replaced = true; input.setFocusRoot(&replacement); }
        });
        input.setFocusRoot(&first);
        QKeyEvent key(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier, 17, 0, 0, "w");
        QCoreApplication::sendEvent(&window, &key);
        check(input.isPressed("up"), "newest focus-root request wins after reset observers");
    }
    qInfo() << "PASS:" << assertions << "input reentrancy assertions";
}
