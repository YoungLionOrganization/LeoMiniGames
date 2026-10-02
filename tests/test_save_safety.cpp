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
    GameSave future(&paths); future.activate("future_game", "2.0.0", 2);
    future.set("coins", 99); check(future.save(), "future schema fixture");
    const QString futurePath = paths.saves()+"/future_game/autosave.lmgsave";
    auto bytes = [](const QString &path) { QFile f(path); if (!f.open(QIODevice::ReadOnly)) return QByteArray{}; return f.readAll(); };
    const QByteArray original = bytes(futurePath);
    GameSave older(&paths); older.activate("future_game", "1.0.0", 1);
    check(!older.load(), "future schema rejected"); older.set("coins", 0);
    check(!older.forceSave(), "failed load blocks automatic overwrite");
    check(bytes(futurePath)==original, "future schema bytes preserved");
    future.set("coins", 100); check(future.save(), "backup fixture");
    const QByteArray backup = bytes(futurePath+".bak");
    check(future.forceSave() && future.forceSave(), "repeated force save succeeds");
    check(bytes(futurePath+".bak")==backup, "repeated force save preserves backup");
    QFile damaged(futurePath); check(damaged.open(QIODevice::WriteOnly), "damage primary"); damaged.write("broken"); damaged.close();
    check(future.load(), "recover good backup"); check(future.forceSave(), "commit recovered state");
    check(bytes(futurePath+".bak")==backup, "recovery preserves good backup");
    check(future.save("other"), "same state in second slot");
    check(future.save("autosave"), "unchanged slot save");
    check(future.currentSlot()=="autosave" && future.loadedSchemaVersion()==2, "unchanged save selects requested slot");
    reopened.activate("..","1.0.0",1);check(!reopened.save(),"game id traversal rejected");
    reopened.activate("test_game","1.0.0",1);check(!reopened.save(".."),"dot-dot slot rejected");
    qInfo()<<"PASS:"<<count<<"save assertions";
}
