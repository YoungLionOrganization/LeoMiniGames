// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/GameLifecycle.h"
#include "core/AppController.h"
#include <QCoreApplication>
#include <QDebug>
#include <functional>

class LifecycleProbe final : public QObject {
    Q_OBJECT
public:
    std::function<void()> onStart, onUnload, onPause, onResume;
    Q_INVOKABLE void start() { if(onStart)onStart(); }
    Q_INVOKABLE void unload() { if(onUnload)onUnload(); }
    Q_INVOKABLE void pause() { if(onPause)onPause(); }
    Q_INVOKABLE void resume() { if(onResume)onResume(); }
};

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);int checks=0,failures=0;
    const auto check=[&](bool ok,const char *name){++checks;if(!ok){++failures;qCritical()<<"FAIL:"<<name;}};
    {
        GameLifecycle lifecycle;LifecycleProbe game;
        int starts=0;QObject::connect(&lifecycle,&GameLifecycle::started,&app,[&]{++starts;});
        game.onStart=[&]{lifecycle.close();lifecycle.unload();};
        lifecycle.attach(&game,"close_during_start");lifecycle.start();
        check(lifecycle.gameId().isEmpty()&&starts==0,"closed startup cannot emit a late started signal");
    }
    {
        GameLifecycle lifecycle;LifecycleProbe game,replacement;
        game.onUnload=[&]{lifecycle.attach(&replacement,"replacement");};
        lifecycle.attach(&game,"old");lifecycle.unload();
        check(lifecycle.gameId()=="replacement","unload hook cannot clear a replacement session");
    }
    {
        GameLifecycle lifecycle;LifecycleProbe game,replacement;
        bool emptyOnUnload=false;
        QObject::connect(&lifecycle,&GameLifecycle::unloaded,&app,[&]{emptyOnUnload=lifecycle.gameId().isEmpty();lifecycle.attach(&replacement,"replacement");});
        lifecycle.attach(&game,"old");lifecycle.unload();
        check(emptyOnUnload,"unloaded observers see the retired session cleared");
        check(lifecycle.gameId()=="replacement","unloaded observer can attach the next session");
    }
    {
        GameLifecycle lifecycle;LifecycleProbe game;
        int pauses=0,resumes=0;
        QObject::connect(&lifecycle,&GameLifecycle::paused,&app,[&]{++pauses;});
        QObject::connect(&lifecycle,&GameLifecycle::resumed,&app,[&]{++resumes;});
        lifecycle.attach(&game,"pause_reentry");game.onPause=[&]{lifecycle.resume();};lifecycle.pause();
        check(pauses==0&&resumes==1,"cancelled pause cannot emit a stale paused signal");
        game.onPause=nullptr;lifecycle.pause();pauses=resumes=0;
        game.onResume=[&]{lifecycle.pause();};lifecycle.resume();
        check(pauses==1&&resumes==0,"cancelled resume cannot emit a stale resumed signal");
    }
    {
        AppController controller(nullptr);int opens=0;
        QObject::connect(&controller,&AppController::gameOpened,&app,[&]{++opens;});
        QObject::connect(&controller,&AppController::currentGameChanged,&app,[&]{if(controller.currentGameId()=="first")controller.closeGame();});
        check(!controller.openExternalSession("first",QUrl("qrc:/mods/first/Main.qml"),"1.0.0"),"interrupted open reports failure");
        check(opens==0&&controller.currentGameId().isEmpty(),"closed open cannot emit gameOpened");
    }
    {
        AppController controller(nullptr);int opens=0;
        QObject::connect(&controller,&AppController::gameOpened,&app,[&]{++opens;});
        QObject::connect(&controller,&AppController::currentGameChanged,&app,[&]{if(controller.currentGameId()=="first")controller.openExternalSession("second",QUrl("qrc:/mods/second/Main.qml"),"1.0.0");});
        check(!controller.openExternalSession("first",QUrl("qrc:/mods/first/Main.qml"),"1.0.0"),"replaced open reports failure");
        check(opens==1&&controller.currentGameId()=="second","replacement opens once without stale old notifications");
    }
    qInfo()<<checks<<"lifecycle assertions,"<<failures<<"failures";return failures?1:0;
}
#include "test_lifecycle_reentrancy.moc"
