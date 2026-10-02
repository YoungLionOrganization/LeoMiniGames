// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include <QGuiApplication>
#include <QHash>
#include <QCoreApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSslSocket>
#include <QTimer>
#include <QQuickWindow>
#include <QQmlError>
#include <QImage>
#include <QDir>
#include <cstdio>
#include <QtPlugin>

#include "core/Achievements.h"
#include "core/AppController.h"
#include "core/AppPaths.h"
#include "core/AudioManager.h"
#include "core/BackNavigation.h"
#include "core/GameAudio.h"
#include "core/DeveloperManager.h"
#include "core/ExternalGameRuntime.h"
#include "core/GameLifecycleFacade.h"
#include "core/GameResources.h"
#include "core/GameRuntime.h"
#include "core/GameThemeFacade.h"
#include "core/LegacyLanguageFacade.h"
#include "core/LegacyAudioFacade.h"
#include "core/LegacyAppFacade.h"
#include "core/GameLoggerFacade.h"
#include "core/LegacySettingsFacade.h"
#include "core/GameI18n.h"
#include "core/GameInput.h"
#include "core/GameRandom.h"
#include "core/GameEvents.h"
#include "core/GameClock.h"
#include "core/GameLifecycle.h"
#include "core/GameRegistry.h"
#include "core/GameSave.h"
#include "core/BuiltinGame.h"
#include "core/TlsRuntime.h"
#include "core/GameSettings.h"
#include "core/GameStats.h"
#include "core/GameViewport.h"
#include "core/Haptics.h"
#include "core/LanguageManager.h"
#include "core/ModFilterProxyModel.h"
#include "core/ModManager.h"
#include "core/PluginDiagnostics.h"
#include "core/PluginManager.h"
#include "core/SettingsManager.h"
#include "core/ThemeManager.h"
#include "core/ThemeCatalogManager.h"
#include "core/ThemeInstalledModel.h"
#include "core/UpdateService.h"
#include "core/ThemeFilterProxyModel.h"

Q_IMPORT_PLUGIN(XoxPlugin)
Q_IMPORT_PLUGIN(BlackjackPlugin)
Q_IMPORT_PLUGIN(MinesweeperPlugin)
Q_IMPORT_PLUGIN(TwentyFortyEightPlugin)
Q_IMPORT_PLUGIN(MemoryMatchPlugin)
Q_IMPORT_PLUGIN(ReactionTapPlugin)

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    qputenv("ANDROID_OPENSSL_SUFFIX", "_3");
#endif
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("YoungLion"));
    app.setOrganizationDomain(QStringLiteral("xyz.younglion.leominigames"));
    app.setApplicationName(QStringLiteral("LeoMiniGames"));
    app.setApplicationVersion(QStringLiteral("0.7.3"));
    app.setApplicationDisplayName(QStringLiteral("LeoMiniGames"));
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    app.setDesktopFileName(QStringLiteral("xyz.younglion.leominigames"));
#endif
    TlsRuntime::initialize();
    if (app.arguments().contains(QStringLiteral("--tls-smoke-test"))) return TlsRuntime::probe();
    app.setWindowIcon(QIcon(QStringLiteral(":/branding/leominigames_icon.png")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppPaths paths;
    SettingsManager settings;
    UpdateService updates(&settings);
    ThemeManager theme(&paths);
    ThemeCatalogManager themeCatalog(&paths, &settings, &theme);
    ThemeInstalledModel themeInstalledModel(&theme, &themeCatalog);
    ThemeFilterProxyModel themeExplore;
    themeExplore.setSourceModel(&themeCatalog);
    themeExplore.setSortMode(QStringLiteral("updated"));
    ThemeFilterProxyModel themeInstalled;
    themeInstalled.setSourceModel(&themeInstalledModel);
    themeInstalled.setSortMode(QStringLiteral("active"));
    AudioManager audio(&settings);
    PluginManager plugins(&paths);
    LanguageManager language(&settings);
    ModManager mods(&paths, &settings);
    GameRegistry registry(&plugins, &mods);
    AppController controller(&registry);
    BackNavigation backNavigation;

    GameSettings gameSettings(&settings);
    GameSave gameSave(&paths);
    GameInput gameInput;
    GameRandom gameRandom;
    GameEvents gameEvents;
    GameClock gameClock;
    GameViewport viewport;
    GameLifecycle lifecycle;
    GameLifecycleFacade lifecycleFacade(&lifecycle);
    GameI18n gameI18n;
    GameStats gameStats(&paths);
    Achievements achievements(&paths);
    GameAudio gameAudio(&audio, &settings);
    Haptics haptics;
    PluginDiagnostics diagnostics;
    GameRuntime gameRuntime;
    GameResources gameResources;
    GameThemeFacade gameTheme(&theme);
    LegacySettingsFacade legacySettings(&settings);
    LegacyLanguageFacade legacyLanguage(&language);
    LegacyAudioFacade legacyAudio(&audio);
    LegacyAppFacade legacyApp(&controller);
    GameLoggerFacade gameLogger(&diagnostics);
    DeveloperManager developer(&paths, &registry, &diagnostics);
    ExternalGameRuntime externalRuntime(&diagnostics);

    QHash<QString, QObject *> externalServices;
    externalServices.insert(QStringLiteral("App"), &legacyApp);
    externalServices.insert(QStringLiteral("Settings"), &legacySettings);
    externalServices.insert(QStringLiteral("Lang"), &legacyLanguage);
    externalServices.insert(QStringLiteral("Audio"), &legacyAudio);
    externalServices.insert(QStringLiteral("GameSettings"), &gameSettings);
    externalServices.insert(QStringLiteral("GameSave"), &gameSave);
    externalServices.insert(QStringLiteral("GameInput"), &gameInput);
    externalServices.insert(QStringLiteral("GameRandom"), &gameRandom);
    externalServices.insert(QStringLiteral("GameEvents"), &gameEvents);
    externalServices.insert(QStringLiteral("GameClock"), &gameClock);
    externalServices.insert(QStringLiteral("GameTheme"), &gameTheme);
    externalServices.insert(QStringLiteral("ThemeRuntime"), &gameTheme);
    externalServices.insert(QStringLiteral("Viewport"), &viewport);
    externalServices.insert(QStringLiteral("Lifecycle"), &lifecycleFacade);
    externalServices.insert(QStringLiteral("GameI18n"), &gameI18n);
    externalServices.insert(QStringLiteral("GameStats"), &gameStats);
    externalServices.insert(QStringLiteral("Achievements"), &achievements);
    externalServices.insert(QStringLiteral("GameAudio"), &gameAudio);
    externalServices.insert(QStringLiteral("Haptics"), &haptics);
    externalServices.insert(QStringLiteral("GameLogger"), &gameLogger);
    externalServices.insert(QStringLiteral("GameRuntime"), &gameRuntime);
    externalServices.insert(QStringLiteral("GameResources"), &gameResources);
    externalRuntime.setServices(externalServices);

    ModFilterProxyModel modsExplore;
    modsExplore.setSourceModel(&mods);
    modsExplore.setMode(QStringLiteral("catalog"));
    modsExplore.setSortMode(QStringLiteral("updated"));
    ModFilterProxyModel modsInstalled;
    modsInstalled.setSourceModel(&mods);
    modsInstalled.setMode(QStringLiteral("installed"));
    modsInstalled.setSortMode(QStringLiteral("name"));

    const bool smokeTest = app.arguments().contains(QStringLiteral("--smoke-test"));
    bool smokeQmlWarnings = false;
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlEngine::warnings, &app, [&smokeQmlWarnings](const QList<QQmlError> &errors) { if (!errors.isEmpty()) smokeQmlWarnings = true; });
    gameSave.setEngine(&engine);
    diagnostics.attach(&engine);
    language.setEngine(&engine);
    language.restore();
    gameI18n.setLanguage(language.language());

    QObject::connect(&language, &LanguageManager::languageChanged, &gameI18n, [&language, &gameI18n] {
        gameI18n.setLanguage(language.language());
    });
    QObject::connect(&controller, &AppController::gameOpened, &app, [&] {
        const QString id = controller.currentGameId();
        mods.setActiveGameId(id);
        const QString version = controller.currentGameVersion();
        const QString source = controller.currentGameSource();
        const bool external = source == QStringLiteral("ExternalRcc");
        const bool developerRcc = source == QStringLiteral("DeveloperRcc");
        legacySettings.activate(id);
        legacyAudio.activate(id);
        gameAudio.activate(id, external || developerRcc);
        gameSettings.activate(id, developerRcc ? developer.settingsSchemaFor(id) : (external ? mods.settingsSchemaFor(id) : QVariantMap{}));
        gameSave.setEngine(&engine);
        gameSave.activate(id, version, developerRcc ? developer.saveVersionFor(id) : (external ? mods.saveVersionFor(id) : registry.saveVersionFor(id)));
        if (auto *builtin = qobject_cast<BuiltinGame *>(controller.currentGame())) builtin->setSaveService(&gameSave);
        gameI18n.activate(id, developerRcc ? developer.defaultLocaleFor(id) : (external ? mods.defaultLocaleFor(id) : QStringLiteral("en")), developerRcc ? developer.localesFor(id) : (external ? mods.localesFor(id) : QStringList{}));
        gameResources.activate(id);
        gameStats.activate(id);
        achievements.activate(id);
        diagnostics.activate(id, version, source);
        gameInput.reset();
    });
    QObject::connect(&developer, &DeveloperManager::packageClosing, &controller, [&controller](const QString &id) { if (controller.currentGameId() == id) controller.closeGame(); });
    QObject::connect(&developer, &DeveloperManager::launchRequested, &controller, [&controller](const QString &id, const QUrl &url, const QString &version) { controller.openExternalSession(id, url, version); });
    QObject::connect(&gameSave, &GameSave::lastErrorChanged, &diagnostics, [&] { if (!gameSave.lastError().isEmpty()) diagnostics.log(QStringLiteral("save"), gameSave.lastError(), QStringLiteral("GameSave"), 0); });
    QObject::connect(&gameAudio, &GameAudio::audioError, &diagnostics, [&diagnostics](const QString &message, const QString &source) { diagnostics.log(QStringLiteral("audio"), message, source, 0); });
    QObject::connect(&theme, &ThemeManager::errorOccurred, &diagnostics, [&diagnostics](const QString &message) { diagnostics.log(QStringLiteral("theme"), message, QStringLiteral("ThemeManager"), 0); });

    QObject::connect(&lifecycle, &GameLifecycle::started, &gameStats, &GameStats::startSession);
    QObject::connect(&lifecycle, &GameLifecycle::started, &gameClock, &GameClock::reset);
    QObject::connect(&lifecycle, &GameLifecycle::paused, &gameClock, &GameClock::pause);
    QObject::connect(&lifecycle, &GameLifecycle::resumed, &gameClock, &GameClock::resume);
    QObject::connect(&lifecycle, &GameLifecycle::paused, &gameAudio, &GameAudio::pauseAll);
    QObject::connect(&lifecycle, &GameLifecycle::resumed, &gameAudio, &GameAudio::resumeAll);
    QObject::connect(&lifecycle, &GameLifecycle::closed, &gameStats, &GameStats::endSession);
    QObject::connect(&controller, &AppController::gameClosing, &app, [&] {
        gameInput.setFocusRoot(nullptr);
        lifecycle.save();
        gameSave.forceSave();
        lifecycle.close();
        lifecycle.unload();
        audio.releasePrefix(QStringLiteral("qrc:/mods/%1/").arg(controller.currentGameId()));
    });
    QObject::connect(&controller, &AppController::gameClosed, &app, [&] {
        mods.setActiveGameId(QString{});
        gameStats.endSession();
        gameInput.reset();
        gameAudio.stopMusic();
        externalRuntime.unload();
        legacySettings.activate(QString());
        legacyAudio.activate(QString());
        gameAudio.activate(QString(), false);
        gameResources.activate(QString{});
        gameSettings.activate(QString{});
        gameSave.activate(QString{}, QString{}, 1);
        gameI18n.activate(QString{}, QStringLiteral("en"), QStringList{});
        achievements.activate(QString{});
    });
    QObject::connect(&app, &QGuiApplication::applicationStateChanged, &app, [&](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) gameAudio.resumeAll();
        else gameAudio.pauseAll();
        if (controller.currentGameId().isEmpty())
            return;
        if (state == Qt::ApplicationInactive || state == Qt::ApplicationSuspended || state == Qt::ApplicationHidden) {
            gameInput.reset();
            lifecycle.pause();
            lifecycle.background();
            lifecycle.save();
            gameSave.forceSave();
            gameAudio.pauseAll();
        } else if (state == Qt::ApplicationActive) {
            lifecycle.foreground();
            lifecycle.resume();
            gameAudio.resumeAll();
        }
    });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &controller, &AppController::closeGame);

    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("Paths"), &paths);
    engine.rootContext()->setContextProperty(QStringLiteral("Settings"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("Updates"), &updates);
    engine.rootContext()->setContextProperty(QStringLiteral("ThemeRuntime"), &theme);
    engine.rootContext()->setContextProperty(QStringLiteral("ThemeCatalog"), &themeCatalog);
    engine.rootContext()->setContextProperty(QStringLiteral("ThemesExplore"), &themeExplore);
    engine.rootContext()->setContextProperty(QStringLiteral("ThemesInstalled"), &themeInstalled);
    engine.rootContext()->setContextProperty(QStringLiteral("Audio"), &audio); // Legacy API
    engine.rootContext()->setContextProperty(QStringLiteral("Plugins"), &plugins); // Legacy API
    engine.rootContext()->setContextProperty(QStringLiteral("Lang"), &language);
    engine.rootContext()->setContextProperty(QStringLiteral("Mods"), &mods);
    engine.rootContext()->setContextProperty(QStringLiteral("Back"), &backNavigation);

    engine.rootContext()->setContextProperty(QStringLiteral("Games"), &registry);
    engine.rootContext()->setContextProperty(QStringLiteral("ModsExplore"), &modsExplore);
    engine.rootContext()->setContextProperty(QStringLiteral("ModsInstalled"), &modsInstalled);
    engine.rootContext()->setContextProperty(QStringLiteral("GameSettings"), &gameSettings);
    engine.rootContext()->setContextProperty(QStringLiteral("GameSave"), &gameSave);
    engine.rootContext()->setContextProperty(QStringLiteral("GameInput"), &gameInput);
    engine.rootContext()->setContextProperty(QStringLiteral("GameRandom"), &gameRandom);
    engine.rootContext()->setContextProperty(QStringLiteral("GameEvents"), &gameEvents);
    engine.rootContext()->setContextProperty(QStringLiteral("GameClock"), &gameClock);
    engine.rootContext()->setContextProperty(QStringLiteral("GameTheme"), &gameTheme);
    engine.rootContext()->setContextProperty(QStringLiteral("Viewport"), &viewport);
    engine.rootContext()->setContextProperty(QStringLiteral("Lifecycle"), &lifecycle);
    engine.rootContext()->setContextProperty(QStringLiteral("GameI18n"), &gameI18n);
    engine.rootContext()->setContextProperty(QStringLiteral("GameStats"), &gameStats);
    engine.rootContext()->setContextProperty(QStringLiteral("Achievements"), &achievements);
    engine.rootContext()->setContextProperty(QStringLiteral("GameAudio"), &gameAudio);
    engine.rootContext()->setContextProperty(QStringLiteral("Haptics"), &haptics);
    engine.rootContext()->setContextProperty(QStringLiteral("GameLogger"), &diagnostics);
    engine.rootContext()->setContextProperty(QStringLiteral("GameRuntime"), &gameRuntime);
    engine.rootContext()->setContextProperty(QStringLiteral("GameResources"), &gameResources);
    engine.rootContext()->setContextProperty(QStringLiteral("Developer"), &developer);
    engine.rootContext()->setContextProperty(QStringLiteral("ExternalRuntime"), &externalRuntime);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("LeoMiniGames"), QStringLiteral("Main"));
#if !defined(LMG_PRIVACY_BUILD)
    if (!smokeTest) QTimer::singleShot(1500, &updates, &UpdateService::checkForUpdatesAutomatically);
#endif
    if (smokeTest) {
        const QString sizeText = qEnvironmentVariable("LMG_SMOKE_SIZE");
        const QStringList dimensions = sizeText.split(QLatin1Char('x'));
        auto *smokeWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (smokeWindow && dimensions.size() == 2) {
            smokeWindow->setMinimumSize(QSize(0, 0));
            smokeWindow->resize(dimensions.at(0).toInt(), dimensions.at(1).toInt());
        }
        auto *timer = new QTimer(&app);
        const QStringList games{QStringLiteral("xox"), QStringLiteral("blackjack"), QStringLiteral("minesweeper"), QStringLiteral("2048"), QStringLiteral("memory_match"), QStringLiteral("reaction_tap")};
        QObject::connect(timer, &QTimer::timeout, &app, [&, timer, games, index = 0, opened = false]() mutable {
            if (!opened) {
                if (index == games.size()) {
                    // Distro Qt logging rules can suppress qInfo; test evidence
                    // must remain observable after a successful full session run.
                    std::puts("PASS: six builtin QML sessions opened and closed");
                    std::fflush(stdout);
                    app.exit(0); return;
                }
                if (!controller.openGame(games.at(index))) { qCritical("Smoke: game open failed"); app.exit(2); return; }
                opened = true;
                return;
            }
            if (smokeQmlWarnings || lifecycle.gameId() != games.at(index) || !diagnostics.lastError().isEmpty()) {
                qCritical() << "Smoke: lifecycle/QML failure" << games.at(index) << diagnostics.lastError(); app.exit(3); return;
            }
            const QString screenshotDir = qEnvironmentVariable("LMG_SMOKE_OUTPUT");
            if (!screenshotDir.isEmpty()) {
                QDir().mkpath(screenshotDir);
                auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
                if (window) window->grabWindow().save(screenshotDir + QLatin1Char('/') + games.at(index) + QStringLiteral(".png"));
            }
            controller.closeGame();
            if (!controller.currentGameId().isEmpty() || !lifecycle.gameId().isEmpty()) { app.exit(4); return; }
            ++index; opened = false;
        });
        timer->start(500);
    }
    return app.exec();
}
