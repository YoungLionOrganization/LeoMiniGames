#include "core/AppPaths.h"
#include "core/SettingsManager.h"
#include "core/LegacySettingsFacade.h"
#include "core/LegacyAudioFacade.h"
#include "core/LegacyLanguageFacade.h"
#include "core/LanguageManager.h"
#include "core/GameThemeFacade.h"
#include "core/GameResources.h"
#include "core/GameRuntime.h"
#include "core/GameAudio.h"
#include "core/GameLifecycleFacade.h"
#include "core/AppController.h"
#include "core/LegacyAppFacade.h"
#include "core/ExternalGameRuntime.h"
#include "core/GameSave.h"
#include "core/GameLifecycle.h"
#include "core/PluginDiagnostics.h"
#include "core/RccPackageInspector.h"
#include <QGuiApplication>
#include <QQuickItem>
#include <QJSEngine>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QPointer>
#include <QDebug>

int main(int argc, char **argv) {
    QTemporaryDir data; qputenv("XDG_DATA_HOME",data.path().toUtf8()); qputenv("XDG_CONFIG_HOME",data.path().toUtf8());
    QGuiApplication app(argc,argv); app.setOrganizationName("LMGTests"); app.setApplicationName("ExternalSession");
    int count=0; auto check=[&](bool ok,const char *name){++count; if (!ok) qFatal("FAIL: %s",name);};
    check(argc==2,"fixture argument");
    AppPaths paths; GameSave original(&paths); original.activate("migration_v07","1.0.0",1); original.set("coins",7); check(original.save(),"old schema fixture");
    GameSave save(&paths); QJSEngine hostEngine; save.setEngine(&hostEngine); save.activate("migration_v07","1.0.0",2);
    PluginDiagnostics diagnostics; ExternalGameRuntime runtime(&diagnostics); AppController controller(nullptr); LegacyAppFacade facade(&controller);
    GameLifecycle lifecycle; QQuickItem container; container.setWidth(400);container.setHeight(300);
    runtime.setServices({{"GameSave",&save},{"App",&facade}});
    QObject::connect(&runtime,&ExternalGameRuntime::loaded,&lifecycle,[&](QObject *item){lifecycle.attach(item,controller.currentGameId());lifecycle.load();lifecycle.start();});
    QObject::connect(&controller,&AppController::gameClosing,&lifecycle,[&]{lifecycle.save();check(save.forceSave(),"coordinated save");lifecycle.close();lifecycle.unload();});
    QObject::connect(&controller,&AppController::gameClosed,&runtime,&ExternalGameRuntime::unload);
    const QString file=QString::fromLocal8Bit(argv[1])+"/v0_7_migration.rcc";
    const auto inspection=RccPackageInspector::inspect(file,"migration_v07");check(inspection.valid,"valid fixture");check(RccPackageInspector::mount(file,inspection),"mount fixture");
    const QUrl url("qrc:/mods/migration_v07/Main.qml"); controller.openExternalSession("migration_v07",url,"1.0.0");
    check(runtime.load(&container,url,"migration_v07",false),"external engine creates game");
    QPointer<QObject> root=runtime.item();check(root && root->property("migrationSucceeded").toBool(),"migration runs in external engine");
    check(root->property("coins").toInt()==8,"migrated state returned");check(root->property("parentAttached").toBool(),"parent attached before onCompleted");
    check(root->property("loads").toInt()==1,"one lifecycle load");
    check(QMetaObject::invokeMethod(root,"requestClose",Qt::DirectConnection),"close from running JS callback");
    check(controller.currentGameId().isEmpty(),"session closed");check(root && root->property("closes").toInt()==1,"close hook once without deleting executing root");
    check(root->property("saves").toInt()==1,"save hook once");
    QCoreApplication::processEvents();check(!root,"root retired after callback");
    GameSave reopened(&paths);reopened.activate("migration_v07","1.0.0",2);check(reopened.load(),"migrated file reopens");check(reopened.get("coins").toInt()==8,"saved migrated payload");
    check(diagnostics.lastError().isEmpty(),"no QML errors");RccPackageInspector::unmount(file,inspection);
    SettingsManager settings;
    LegacySettingsFacade legacySettings(&settings); LegacyAudioFacade legacyAudio(nullptr);
    LanguageManager language(&settings); LegacyLanguageFacade legacyLanguage(&language);
    GameThemeFacade theme(nullptr); GameResources resources; GameRuntime gameRuntime;
    GameAudio gameAudio(nullptr,&settings); GameLifecycleFacade legacyLifecycle(&lifecycle);
    runtime.setServices({{"GameSave",&save},{"App",&facade},{"Settings",&legacySettings},{"Audio",&legacyAudio},{"Lang",&legacyLanguage},{"GameTheme",&theme},{"GameAudio",&gameAudio},{"Lifecycle",&legacyLifecycle},{"GameRuntime",&gameRuntime},{"GameResources",&resources}});
    for (const QString &fixture : {QStringLiteral("v0_5_canonical"),QStringLiteral("v0_5_legacy_prefix"),QStringLiteral("v0_6_services"),QStringLiteral("v0_7_modern")}) {
        const QString package=QString::fromLocal8Bit(argv[1])+QLatin1Char('/')+fixture+QStringLiteral(".rcc");
        const auto inspected=RccPackageInspector::inspect(package); check(inspected.valid,"legacy inspection");
        check(RccPackageInspector::mount(package,inspected),"legacy mount");
        const QString id=inspected.packageId;
        diagnostics.activate(id,"fixture","ExternalRcc"); legacySettings.activate(id);legacyAudio.activate(id);gameAudio.activate(id,true);resources.activate(id);
        save.activate(id,"fixture",inspected.manifest.value(QStringLiteral("save_version")).toInt(1));
        const QUrl entry(QStringLiteral("qrc:/mods/%1/%2").arg(id,inspected.entryPath));
        controller.openExternalSession(id,entry,"fixture");
        check(runtime.load(&container,entry,id,false),"legacy QML creation");check(diagnostics.lastError().isEmpty(),"legacy services without QML errors");
        controller.closeGame();QCoreApplication::processEvents();RccPackageInspector::unmount(package,inspected);
    }
    qInfo()<<"PASS:"<<count<<"external session assertions";
}
