#include "core/GameInput.h"
#include "core/GameClock.h"
#include "core/SemVer.h"
#include "MemoryMatch/MemoryMatchGame.h"
#include "ReactionTap/ReactionTapGame.h"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QQuickWindow>
#include <QQuickItem>
#include <QKeyEvent>

static void wait(int ms) { QEventLoop loop; QTimer::singleShot(ms, &loop, &QEventLoop::quit); loop.exec(); }
int main(int argc, char **argv) {
    QTemporaryDir temp;
    qputenv("XDG_DATA_HOME", temp.path().toUtf8());
    QGuiApplication app(argc, argv);
    app.setOrganizationName("LMGTests"); app.setApplicationName("PatchRuntime");
    int count=0;
    auto check=[&](bool condition, const char *name){ ++count; if (!condition) qFatal("FAIL: %s", name); };
    using namespace LmgSemVer;
    for (const QString &version : {"1.2.3-alpha..1", "1.2.3-01", "2147483648.0.0", "1.2.3+build..2"})
        check(!parseSemVer(version).valid, "reject malformed/overflow version");
    check(parseSemVer("0.7.3+build.2").valid, "valid metadata");
    check(compareSemVer(parseSemVer("1.0.0-99999999999999999999999"),parseSemVer("1.0.0-100000000000000000000000"))<0, "arbitrary prerelease numeric ordering");
    check(compareSemVer(parseSemVer("1.0.0"),parseSemVer("1.0.0-rc.1"))>0, "stable outranks prerelease");
    GameInput input;
    input.handleKey(Qt::Key_A,30,"a",true);
    input.handleKey(Qt::Key_Left,105,"",true);
    input.handleKey(Qt::Key_A,30,"a",false);
    check(input.isPressed("move_left"), "second key keeps action held");
    input.handleKey(Qt::Key_Left,105,"",false);
    check(!input.isPressed("move_left"), "last key releases action");
    input.handleKey(Qt::Key_W,17,"w",true);
    input.handleKey(Qt::Key_W,17,"w",false,true);
    check(input.isPressed("up"), "autorepeat release ignored"); input.reset();
    check(!input.isPressed("up"), "reset clears held keys");
    QQuickWindow window;
    QQuickItem root(window.contentItem());
    QQuickItem outside(window.contentItem());
    window.show(); root.forceActiveFocus(); app.processEvents();
    input.setFocusRoot(&root);
    QKeyEvent nativePress(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier, 17, 0, 0, "w");
    QCoreApplication::sendEvent(&window, &nativePress);
    check(input.isPressed("up"), "native Qt window key event reaches game action");
    QEvent deactivated(QEvent::WindowDeactivate);
    QCoreApplication::sendEvent(&window, &deactivated);
    check(!input.isPressed("up"), "window deactivation clears native action");
    outside.forceActiveFocus();
    QCoreApplication::sendEvent(&window, &nativePress);
    check(!input.isPressed("up"), "key outside game focus root ignored");
    input.setFocusRoot(nullptr);
    GameClock clock; clock.pause(); clock.setTimeScale(3); clock.reset();
    check(!clock.paused() && clock.timeScale()==1.0, "new session resets clock pause and scale");
    MemoryMatchGame memory;
    const auto cards=memory.cards();
    int second=1;
    while(cards[second].toMap().value("symbol")==cards[0].toMap().value("symbol")) ++second;
    memory.flip(0); memory.flip(second); check(memory.locked(), "mismatch pending");
    memory.pause(); wait(700); check(memory.locked(), "pending pair suspended");
    memory.resume(); memory.reset(); memory.flip(0);
    wait(700); check(memory.cards()[0].toMap().value("faceUp").toBool(), "old pair cannot flip new board");
    ReactionTapGame reaction; reaction.startRound(); reaction.pause(); wait(3600);
    check(reaction.state()=="idle", "background invalidates reaction round");
    reaction.tap(); check(reaction.state()=="idle", "paused tap ignored");
    reaction.resume(); reaction.startRound(); reaction.tap();
    check(reaction.state()=="false_start" && reaction.rounds()==0, "early tap does not create a record");
    qInfo()<<"PASS:"<<count<<"patch runtime assertions";
}
