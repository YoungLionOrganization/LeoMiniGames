#include "core/GameSave.h"
#include "core/AppPaths.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QJSEngine>
#include <QDebug>
#include <functional>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

class SaveSessionProbe final : public QObject {
    Q_OBJECT
public:
    std::function<void()> onSwitch;
    Q_INVOKABLE void switchSession(){if(onSwitch)onSwitch();}
};

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
    GameSave deleted(&paths); deleted.activate("delete_game", "1.0.0", 1);
    deleted.set("score", 3); check(deleted.save(), "delete fixture seeded");
    const QString deletePath=paths.saves()+"/delete_game/autosave.lmgsave";
    QFile broken(deletePath);check(broken.open(QIODevice::WriteOnly),"corrupt deletion fixture");broken.write("broken");broken.close();
    check(!deleted.load(),"invalid slot protection seeded");
    check(deleted.deleteSlot("autosave"),"explicitly delete protected slot");
    deleted.set("score",4);check(deleted.forceSave(),"explicit deletion clears slot protection");
    check(deleted.lastError().isEmpty(),"successful deletion and recreation clear stale error");
    const QByteArray beforeInvalidDelete=bytes(deletePath);
    check(QDir().mkdir(deletePath+".bak"),"non-file backup fixture");
    check(!deleted.deleteSlot("autosave"),"non-file backup makes deletion fail");
    check(bytes(deletePath)==beforeInvalidDelete,"failed backup deletion preserves primary");
    check(!deleted.lastError().isEmpty(),"failed deletion reports a diagnostic");
    check(QDir().rmdir(deletePath+".bak"),"remove non-file backup fixture");
#ifdef Q_OS_WIN
    deleted.set("score",5);check(deleted.save(),"locked primary fixture seeded");
    const QByteArray lockedPrimary=bytes(deletePath),lockedBackup=bytes(deletePath+".bak");
    const HANDLE locked=CreateFileW(reinterpret_cast<LPCWSTR>(deletePath.utf16()),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    check(locked!=INVALID_HANDLE_VALUE,"hold primary without delete sharing");
    check(!deleted.deleteSlot("autosave"),"locked primary rejects slot deletion");
    check(bytes(deletePath)==lockedPrimary&&bytes(deletePath+".bak")==lockedBackup,"failed retirement restores staged backup");
    CloseHandle(locked);
#endif
    check(deleted.deleteSlot("autosave")&&!deleted.hasStoredSlot("autosave"),"deletion removes both slot paths");
    check(deleted.deleteSlot("autosave")&&deleted.lastError().isEmpty(),"missing slot deletion is idempotent");
    GameSave interruptedWriter(&paths);interruptedWriter.activate("interrupted_game","1.0.0",1);
    interruptedWriter.set("coins",41);check(interruptedWriter.save("main"),"interrupted migration seed");
    interruptedWriter.set("coins",42);check(interruptedWriter.save("main"),"interrupted migration backup seed");
    const QString interruptedPath=paths.saves()+"/interrupted_game/main.lmgsave";
    const QByteArray interruptedPrimary=bytes(interruptedPath),interruptedBackup=bytes(interruptedPath+".bak");
    GameSave interrupted(&paths);SaveSessionProbe session;
    engine.globalObject().setProperty("Session",engine.newQObject(&session));
    bool closingSaveAccepted=false;
    session.onSwitch=[&]{closingSaveAccepted=interrupted.forceSave("main");interrupted.activate("replacement_game","1.0.0",2);interrupted.set("marker","keep");};
    for(bool restore : {false,true}) {
        interrupted.activate("interrupted_game","2.0.0",2);interrupted.setEngine(&engine);
        check(interrupted.registerMigration(1,2,engine.evaluate("(function(data){Session.switchSession();data.coins=99;return data;})")),"interrupted migration registered");
        check(!(restore?interrupted.restoreBackup("main"):interrupted.load("main")),"session switch cancels migration operation");
        check(!closingSaveAccepted,"closing save cannot overwrite a migrating slot");
        check(interrupted.gameId()=="replacement_game"&&interrupted.get("marker")=="keep"&&!interrupted.contains("coins"),"interrupted migration cannot populate replacement memory");
        check(bytes(interruptedPath)==interruptedPrimary&&bytes(interruptedPath+".bak")==interruptedBackup,"interrupted migration preserves both disk files");
    }
    interrupted.activate("interrupted_game","2.0.0",2);interrupted.setEngine(&engine);interrupted.set("marker","keep");
    session.onSwitch=[&]{interrupted.setEngine(nullptr);};
    check(interrupted.registerMigration(1,2,engine.evaluate("(function(data){Session.switchSession();return data;})")),"retired engine migration registered");
    check(!interrupted.load("main")&&interrupted.get("marker")=="keep"&&!interrupted.contains("coins"),"engine retirement cancels migration without changing memory");
    interrupted.activate("snapshot_old","1.0.0",1);
    session.onSwitch=[&]{interrupted.activate("snapshot_new","1.0.0",1);interrupted.set("marker","keep");};
    interrupted.set("state",QVariant::fromValue(engine.evaluate("({get value(){Session.switchSession();return 5;}})")));
    check(interrupted.gameId()=="snapshot_new"&&interrupted.get("marker")=="keep"&&!interrupted.contains("state"),"snapshot getters cannot insert old data into a replacement session");
    GameSave notification(&paths);notification.activate("notification_old","1.0.0",1);notification.set("score",7);
    int lateSaves=0;
    QObject::connect(&notification,&GameSave::saved,&app,[&]{++lateSaves;});
    QObject::connect(&notification,&GameSave::slotChanged,&app,[&]{if(notification.gameId()=="notification_old"){notification.activate("notification_new","1.0.0",1);notification.set("marker","keep");}});
    check(!notification.save("main"),"session switch during save notification retires the request");
    check(lateSaves==0&&notification.gameId()=="notification_new"&&notification.get("marker")=="keep","retired save cannot emit a late saved signal");
    GameSave errorBoundary(&paths);errorBoundary.activate("error_old","1.0.0",1);errorBoundary.set("score",9);
    check(!errorBoundary.save(".."),"seed previous save error");
    QObject::connect(&errorBoundary,&GameSave::lastErrorChanged,&app,[&]{if(errorBoundary.lastError().isEmpty()&&errorBoundary.gameId()=="error_old"){errorBoundary.activate("error_new","1.0.0",1);errorBoundary.set("marker","keep");}});
    check(!errorBoundary.save("main"),"error-clear observer can retire a committing save");
    check(errorBoundary.gameId()=="error_new"&&errorBoundary.autosave(),"retired save preserves replacement dirty state");
    GameSave errorReopened(&paths);errorReopened.activate("error_new","1.0.0",1);
    check(errorReopened.load()&&errorReopened.get("marker")=="keep","replacement state persists after reentrant error notification");
    reopened.activate("..","1.0.0",1);check(!reopened.save(),"game id traversal rejected");
    reopened.activate("test_game","1.0.0",1);check(!reopened.save(".."),"dot-dot slot rejected");
    qInfo()<<"PASS:"<<count<<"save assertions";
}
#include "test_save_safety.moc"
