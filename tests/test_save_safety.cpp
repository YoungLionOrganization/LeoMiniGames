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
    AppPaths paths(dir.path()+"/data", dir.path()+"/cache"); GameSave save(&paths); save.activate("test_game","1.0.0",1);
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
    GameSave recoveryWriter(&paths); recoveryWriter.activate("recovery_game", "1.0.0", 1);
    recoveryWriter.set("coins", 7); check(recoveryWriter.save(), "recovery backup seed");
    recoveryWriter.set("coins", 8); check(recoveryWriter.save(), "recovery primary seed");
    const QString recoveryPath = paths.saves()+"/recovery_game/autosave.lmgsave";
    const QByteArray recoveryPrimary = bytes(recoveryPath), recoveryBackup = bytes(recoveryPath+".bak");
    GameSave migratingRecovery(&paths); migratingRecovery.activate("recovery_game", "2.0.0", 2);
    migratingRecovery.setEngine(&engine); migratingRecovery.set("memory", "keep");
    check(!migratingRecovery.restoreBackup("autosave"), "backup with missing migration rejected");
    check(bytes(recoveryPath)==recoveryPrimary && bytes(recoveryPath+".bak")==recoveryBackup, "missing migration preserves both recovery files");
    check(migratingRecovery.get("memory").toString()=="keep", "failed recovery preserves session memory");
    check(migratingRecovery.registerMigration(1,2,engine.evaluate("(function(state){throw new Error('failed migration');})")), "failing backup migration registered");
    check(!migratingRecovery.restoreBackup("autosave"), "throwing backup migration rejected");
    check(bytes(recoveryPath)==recoveryPrimary && bytes(recoveryPath+".bak")==recoveryBackup, "throwing migration preserves both recovery files");
    engine.evaluate("var recoveryCalls = 0;");
    check(migratingRecovery.registerMigration(1,2,engine.evaluate("(function(state){recoveryCalls++; state.coins += 10; return state;})")), "valid backup migration registered");
    check(migratingRecovery.restoreBackup("autosave"), "compatible backup restored");
    check(migratingRecovery.get("coins").toInt()==17 && migratingRecovery.loadedSchemaVersion()==2, "restored backup adopts migrated state");
    check(engine.evaluate("recoveryCalls").toInt()==1, "backup migration runs once");
    check(bytes(recoveryPath+".bak")==recoveryBackup, "successful recovery preserves original backup");
    check(bytes(recoveryPath)==recoveryBackup, "recovery preserves stored schema and metadata until save");
    check(migratingRecovery.forceSave(), "migrated recovery commits new schema");
    check(bytes(recoveryPath+".bak")==recoveryBackup, "migration commit retains original recovery backup");
    GameSave migratedReopened(&paths); migratedReopened.activate("recovery_game", "2.0.0", 2);
    check(migratedReopened.load() && migratedReopened.get("coins").toInt()==17, "migrated recovery reopens without migration callback");
    GameSave newerWriter(&paths); newerWriter.activate("future_backup_game", "2.0.0", 2);
    newerWriter.set("coins", 10); check(newerWriter.save(), "future backup seed");
    newerWriter.set("coins", 11); check(newerWriter.save(), "future primary seed");
    const QString newerPath = paths.saves()+"/future_backup_game/autosave.lmgsave";
    GameSave futureBackup(&paths); futureBackup.activate("future_backup_game", "1.0.0", 1);
    const QByteArray newerBackup = bytes(newerPath+".bak"), beforeFutureRestore = bytes(newerPath);
    check(!futureBackup.restoreBackup("autosave"), "newer backup schema rejected");
    check(bytes(newerPath)==beforeFutureRestore && bytes(newerPath+".bak")==newerBackup, "newer backup rejection preserves both files");
    GameSave backupOnlyWriter(&paths); backupOnlyWriter.activate("backup_only_game", "2.0.0", 2);
    backupOnlyWriter.set("coins", 30); check(backupOnlyWriter.save(), "backup-only seed");
    backupOnlyWriter.set("coins", 31); check(backupOnlyWriter.save(), "backup-only primary seed");
    const QString backupOnlyPath=paths.saves()+"/backup_only_game/autosave.lmgsave";
    const QByteArray onlyBackup=bytes(backupOnlyPath+".bak");
    check(QFile::remove(backupOnlyPath), "simulate missing primary with intact backup");
    GameSave backupOnlyOlder(&paths); backupOnlyOlder.activate("backup_only_game", "1.0.0", 1);
    backupOnlyOlder.set("coins", 0);
    check(!backupOnlyOlder.forceSave(), "backup-only data cannot be overwritten before loading");
    check(!QFile::exists(backupOnlyPath) && bytes(backupOnlyPath+".bak")==onlyBackup, "unloaded backup-only data preserved");
    GameSave backupOnlyRecovery(&paths); backupOnlyRecovery.activate("backup_only_game", "2.0.0", 2);
    check(backupOnlyRecovery.load() && backupOnlyRecovery.get("coins").toInt()==30, "backup-only data loads at compatible schema");
    check(backupOnlyRecovery.forceSave() && bytes(backupOnlyPath+".bak")==onlyBackup, "loaded backup-only data repairs primary safely");
    reopened.activate("..","1.0.0",1);check(!reopened.save(),"game id traversal rejected");
    reopened.activate("test_game","1.0.0",1);check(!reopened.save(".."),"dot-dot slot rejected");
    qInfo()<<"PASS:"<<count<<"save assertions";
}
