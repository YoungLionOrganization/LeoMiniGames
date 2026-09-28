#include "core/GameSave.h"
#include "core/AppPaths.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QJSEngine>
#include <QDebug>

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir dir;
    qputenv("XDG_DATA_HOME", dir.path().toUtf8());
    qputenv("XDG_CACHE_HOME", dir.path().toUtf8());
    app.setOrganizationName("LMGTests");app.setApplicationName("SaveSafety");
    int count=0;
    auto check=[&](bool ok,const char *name){++count;if(!ok)qFatal("FAIL: %s",name);};
    AppPaths paths; GameSave save(&paths); save.activate("test_game","1.0.0",1);
    QVariantMap nested{{"coins",123},{"inventory",QVariantList{1,2,QVariantMap{{"item","gem"}}}}};
    save.set("state",nested);check(save.save("main"),"save initial state");
    check(!save.createSlot("main"),"cannot overwrite existing slot");
    check(save.createSlot("empty"),"create independent slot");
    check(save.currentSlot()=="main","create keeps active slot");
    check(save.get("state").toMap()==nested,"create keeps memory");
    GameSave reopened(&paths);reopened.activate("test_game","1.0.0",1);
    check(reopened.load("main"),"reload disk state");check(reopened.get("state").toMap()==nested,"nested state round trip");
    save.set("counter",1);check(save.save("main"),"create backup");save.set("counter",2);check(save.save("main"),"new primary");
    QFile file(paths.saves()+"/test_game/main.lmgsave");check(file.open(QIODevice::WriteOnly),"corrupt fixture");file.write("broken");file.close();
    check(reopened.load("main"),"backup recovery");check(reopened.get("counter").toInt()==1,"backup content");
    check(!save.createSlot("../escape"),"slot traversal rejected");check(!save.save("/absolute"),"absolute slot rejected");
    save.set("quick",10);check(save.autosave(),"first autosave");save.set("quick",11);check(save.autosave(),"immediate dirty autosave");
    check(reopened.load(),"reload autosave");check(reopened.get("quick").toInt()==11,"latest dirty state retained");
    QJSEngine engine;save.setEngine(&engine);save.set("js",QVariant::fromValue(engine.evaluate("({list:[1,2], nested:{value:7}})")));
    check(save.save(),"JS object save");check(reopened.load(),"JS object load");check(reopened.get("js").toMap().value("nested").toMap().value("value").toInt()==7,"JS nested value round trip");
    reopened.activate("..","1.0.0",1);check(!reopened.save(),"game id traversal rejected");
    reopened.activate("test_game","1.0.0",1);check(!reopened.save(".."),"dot-dot slot rejected");
    qInfo()<<"PASS:"<<count<<"save assertions";
}
