// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/AppPaths.h"
#include "core/AppController.h"
#include "core/LegacyAppFacade.h"
#include "core/LegacySettingsFacade.h"
#include "core/LegacyLanguageFacade.h"
#include "core/LegacyAudioFacade.h"
#include "core/SettingsManager.h"
#include "core/LanguageManager.h"
#include "core/AudioManager.h"
#include "core/GameAudio.h"
#include "core/GameSave.h"
#include "core/GameSettings.h"
#include "core/GameInput.h"
#include "core/GameI18n.h"
#include "core/GameResources.h"
#include "core/GameStats.h"
#include "core/Achievements.h"
#include "core/Haptics.h"
#include "core/GameThemeFacade.h"
#include "core/ThemeManager.h"
#include "core/GameLifecycle.h"
#include "core/GameLifecycleFacade.h"
#include "core/GameViewport.h"
#include "core/GameRuntime.h"
#include "core/GameLoggerFacade.h"
#include "core/PluginDiagnostics.h"
#include "core/ExternalGameRuntime.h"
#include "core/RccPackageInspector.h"
#include "core/PackageCompatibility.h"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QScopeGuard>
#include <cstdio>
#include <cstdlib>
#include <QQuickItem>
#include <QKeyEvent>
#include <QFile>
#include <QDebug>

int main(int argc,char **argv) {
    QTemporaryDir data;
    qputenv("XDG_DATA_HOME",data.path().toUtf8());qputenv("XDG_CONFIG_HOME",data.path().toUtf8());
    QGuiApplication app(argc,argv);app.setOrganizationName("LMGTests");
    app.setApplicationName("SDK-"+QFileInfo(data.path()).fileName());app.setApplicationVersion("0.7.3");
    QStandardPaths::setTestModeEnabled(true);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,data.path());
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    const QString appData=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString cache=QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    const auto cleanup=qScopeGuard([&]{QDir(appData).removeRecursively();QDir(cache).removeRecursively();});
    int count=0;auto check=[&](bool ok,const char *name){++count;if(!ok){std::fprintf(stderr,"FAIL: SDK assertion %d: %s\n",count,name);std::fflush(stderr);std::abort();}};check(argc==2,"SDK fixtures supplied");
    const QString dir=QString::fromLocal8Bit(argv[1]);
    AppPaths paths;SettingsManager settings;LanguageManager language(&settings);AudioManager audio(&settings);GameAudio gameAudio(&audio,&settings);
    LegacySettingsFacade legacySettings(&settings);LegacyLanguageFacade legacyLanguage(&language);LegacyAudioFacade legacyAudio(&audio);
    AppController controller(nullptr);LegacyAppFacade legacyApp(&controller);
    GameSave save(&paths);GameSettings prefs(&settings);GameInput input;GameI18n i18n;GameResources resources;GameStats stats(&paths);Achievements achievements(&paths);Haptics haptics;
    ThemeManager themes(&paths);GameThemeFacade theme(&themes);GameLifecycle lifecycle;GameLifecycleFacade facade(&lifecycle);GameViewport viewport;GameRuntime gameRuntime;
    PluginDiagnostics diagnostics;GameLoggerFacade logger(&diagnostics);ExternalGameRuntime runtime(&diagnostics);
    QQuickWindow window;window.resize(430,820);window.show();QQuickItem container(window.contentItem());container.setWidth(430);container.setHeight(820);viewport.update(430,820);
    runtime.setServices({{"App",&legacyApp},{"Settings",&legacySettings},{"Lang",&legacyLanguage},{"Audio",&legacyAudio},{"GameSave",&save},{"GameSettings",&prefs},{"GameInput",&input},{"GameI18n",&i18n},{"GameResources",&resources},{"GameAudio",&gameAudio},{"GameStats",&stats},{"Achievements",&achievements},{"Haptics",&haptics},{"GameTheme",&theme},{"ThemeRuntime",&theme},{"Lifecycle",&facade},{"Viewport",&viewport},{"GameRuntime",&gameRuntime},{"GameLogger",&logger}});
    QObject::connect(&runtime,&ExternalGameRuntime::loaded,&lifecycle,[&](QObject *root){lifecycle.attach(root,controller.currentGameId());lifecycle.load();lifecycle.start();input.setFocusRoot(root);qobject_cast<QQuickItem *>(root)->forceActiveFocus();});
    QObject::connect(&lifecycle,&GameLifecycle::paused,&gameAudio,&GameAudio::pauseAll);QObject::connect(&lifecycle,&GameLifecycle::resumed,&gameAudio,&GameAudio::resumeAll);
    auto open=[&](const QString &name){
        auto inspection=RccPackageInspector::inspect(dir+"/"+name+".rcc");check(inspection.valid,"SDK package inspected");
        check(PackageCompatibility::error(inspection.manifest).isEmpty(),"SDK manifest host compatibility");
        check(RccPackageInspector::mount(dir+"/"+name+".rcc",inspection),"SDK package mounted");
        const QString id=inspection.packageId;
        controller.openExternalSession(id,QUrl(QStringLiteral("qrc:/mods/%1/%2").arg(id,inspection.entryPath)),inspection.manifest.value("version").toString());
        save.activate(id,"sdk",inspection.manifest.value("save_version").toInt(1));prefs.activate(id,inspection.manifest.value("settings_schema").toObject().toVariantMap());
        legacySettings.activate(id);legacyAudio.activate(id);gameAudio.activate(id,true);resources.activate(id);i18n.activate(id,"en",{"en","tr","az"});stats.activate(id);achievements.activate(id);
        viewport.update(container.width(),container.height());diagnostics.activate(id,"sdk","ExternalRcc");
        check(runtime.load(&container,controller.currentGameUrl(),id,false),"actual SDK QML loads in external engine");
        return inspection;
    };
    auto close=[&](const QString &name,const RccPackageInspection &inspection,bool expectedSave=true){input.setFocusRoot(nullptr);lifecycle.save();check(save.forceSave()==expectedSave,"SDK close respects write protection");lifecycle.close();lifecycle.unload();gameAudio.activate("",false);runtime.unload();app.processEvents();controller.closeGame();RccPackageInspector::unmount(dir+"/"+name+".rcc",inspection);};
    // Real old-schema snapshot must migrate in the package's engine before load.
    save.activate("example_modern","old",1);save.set("score",3);check(save.save(),"schema one SDK fixture saved");
    auto inspected=open("ExampleModernMod");QObject *root=runtime.item();
    check(root->property("taps").toInt()==3&&save.loadedSchemaVersion()==2,"modern SDK migration and lifecycle load");
    input.press("fire");input.release("fire");check(root->property("taps").toInt()==4&&stats.counter("taps")==1,"keyboard uses shared stats/persistence action");
    auto *button=qobject_cast<QQuickItem *>(root->findChild<QObject *>("sdkTapButton"));check(button,"SDK button created");
    check(QMetaObject::invokeMethod(button,"activated"),"touch/button action invokable");check(root->property("taps").toInt()==5&&stats.counter("taps")==2,"button matches keyboard action");
    lifecycle.pause();input.press("fire");input.release("fire");check(root->property("taps").toInt()==5,"SDK pause gates input");lifecycle.resume();
    button->forceActiveFocus();app.processEvents();
    QKeyEvent down(QEvent::KeyPress,Qt::Key_Space,Qt::NoModifier),up(QEvent::KeyRelease,Qt::Key_Space,Qt::NoModifier);
    QCoreApplication::sendEvent(&window,&down);QCoreApplication::sendEvent(&window,&up);app.processEvents();
    check(root->property("taps").toInt()==6,"focused button keyboard activation fires once");
    for(int n=0;n<4;++n)check(QMetaObject::invokeMethod(root,"registerTap"),"shared SDK action callable");
    check(root->property("taps").toInt()==10&&achievements.isUnlocked("ten_taps"),"SDK achievement follows unified action");
    prefs.setValue("difficulty","hard");app.processEvents();
    check(root->findChild<QObject *>("sdkDifficulty")->property("text").toString().endsWith("hard"),"SDK settings view follows valueChanged");
    i18n.setLanguage("tr");app.processEvents();
    check(root->findChild<QObject *>("sdkTitle")->property("text").toString()==QStringLiteral("Modern API Örneği"),"SDK text updates with language");
    check(root->property("status").toString()==QStringLiteral("İlerleme kaydedildi"),"SDK status updates with language after an action");
    const QColor oldPanel=root->findChild<QObject *>("sdkPanel")->property("color").value<QColor>();
    check(themes.installThemeRcc(dir+"/ExampleTheme.rcc"),"actual SDK theme installed");check(themes.applyTheme("example.graphite-gold"),"actual SDK theme applied");app.processEvents();
    check(oldPanel!=themes.color("alias.panel.background.normal"),"SDK theme has a visibly distinct palette");
    check(root->findChild<QObject *>("sdkPanel")->property("color").value<QColor>()==themes.color("alias.panel.background.normal"),"SDK component follows theme revision");
    container.setWidth(640);container.setHeight(360);viewport.update(640,360);app.processEvents();check(root->findChild<QObject *>("sdkPanel")->property("width").toReal()>0,"short landscape SDK width remains positive");
    check(diagnostics.lastError().isEmpty(),"SDK launch has no QML errors");close("ExampleModernMod",inspected);
    inspected=open("ExampleModernMod");check(runtime.item()->property("taps").toInt()==10,"SDK progress reopens");close("ExampleModernMod",inspected);
    // A backup-only damaged slot must not be confused with a first-run game.
    const QString primary=paths.saves()+"/example_modern/autosave.lmgsave";
    check(QFile::remove(primary),"remove test primary to simulate backup-only slot");
    QFile backup(primary+".bak");check(backup.open(QIODevice::WriteOnly|QIODevice::Truncate),"write backup-only fixture");backup.write("damaged backup");backup.close();
    inspected=open("ExampleModernMod");check(!runtime.item()->property("writable").toBool(),"SDK recognizes protected backup-only data through QML helper");
    check(QMetaObject::invokeMethod(runtime.item(),"registerTap"),"invalid-session action callable");check(runtime.item()->property("taps").toInt()==0,"failed load blocks new gameplay snapshot");
    close("ExampleModernMod",inspected,false);
    check(backup.open(QIODevice::ReadOnly),"backup still present");check(backup.readAll()==QByteArray("damaged backup"),"SDK does not overwrite corrupt backup-only data");backup.close();
    inspected=open("ExampleHelloMod");check(diagnostics.lastError().isEmpty(),"legacy SDK Audio calls still load");close("ExampleHelloMod",inspected);
    qInfo()<<"PASS:"<<count<<"SDK example assertions";
}
