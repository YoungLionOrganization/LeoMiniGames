// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    visible: true
    width: Constants.windowDefaultWidth
    height: Constants.windowDefaultHeight
    minimumWidth: Constants.windowMinimumWidth
    minimumHeight: Constants.windowMinimumHeight
    title: "LeoMiniGames"
    color: Constants.background

    function closeCurrentGame() {
        if (App.currentGameId.length === 0)
            return
        Lifecycle.save()
        GameSave.forceSave()
        Lifecycle.close()
        Lifecycle.unload()
        GameAudio.pauseAll()
        Audio.releasePrefix("qrc:/mods/" + App.currentGameId + "/")
        App.closeGame()
    }

    function navigateBack() {
        if (stack.depth > 1) {
            closeCurrentGame()
            stack.pop()
            return
        }
        Qt.quit()
    }

    palette.window: Constants.background
    palette.windowText: Constants.text
    palette.button: Constants.buttonBackgroundNormal
    palette.buttonText: Constants.buttonTextNormal
    palette.base: Constants.surfaceInteractive
    palette.text: Constants.text
    palette.highlight: Constants.primary
    palette.highlightedText: Constants.background

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: libraryComponent

        pushEnter: Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: Constants.ratio0
                    to: Constants.opacityFull
                    duration: Settings.animationsEnabled ? Constants.motionPageDuration : Constants.durationInstant
                }
                NumberAnimation {
                    property: "x"
                    from: stack.width * Constants.n("alias.motion.page.offsetRatio")
                    to: Constants.spaceNone
                    duration: Settings.animationsEnabled ? Constants.motionPageDuration : Constants.durationInstant
                    easing.type: Constants.easing("alias.motion.page.easing")
                }
            }
        }
        pushExit: Transition {
            NumberAnimation {
                property: "opacity"
                from: Constants.opacityFull
                to: Constants.n("alias.motion.page.dimOpacity")
                duration: Settings.animationsEnabled ? Constants.motionFastDuration : Constants.durationInstant
            }
        }
        popEnter: Transition {
            NumberAnimation {
                property: "opacity"
                from: Constants.n("alias.motion.page.dimOpacity")
                to: Constants.opacityFull
                duration: Settings.animationsEnabled ? Constants.motionFastDuration : Constants.durationInstant
            }
        }
        popExit: Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: Constants.opacityFull
                    to: Constants.ratio0
                    duration: Settings.animationsEnabled ? Constants.motionPageDuration : Constants.durationInstant
                }
                NumberAnimation {
                    property: "x"
                    from: Constants.spaceNone
                    to: stack.width * Constants.n("alias.motion.page.offsetRatio")
                    duration: Settings.animationsEnabled ? Constants.motionPageDuration : Constants.durationInstant
                    easing.type: Constants.easing("alias.motion.exit.easing")
                }
            }
        }
    }

    Component {
        id: libraryComponent
        LibraryPage {
            onGameRequested: function(id) { App.openGame(id) }
            onSettingsRequested: stack.push(settingsComponent)
            onModsRequested: stack.push(modsComponent)
            onThemesRequested: stack.push(themesComponent)
        }
    }

    Component { id: settingsComponent; SettingsPage { onBackRequested: window.navigateBack(); onThemesRequested: stack.push(themesComponent); onDeveloperLabRequested: stack.push(developerLabComponent) } }
    Component {
        id: developerLabComponent
        DeveloperLabPage {
            developer: Developer
            gameLogger: GameLogger
            onBackRequested: window.navigateBack()
        }
    }
    Component { id: themesComponent; ThemesPage { onBackRequested: window.navigateBack() } }
    Component {
        id: modsComponent
        ModsPage {
            onBackRequested: window.navigateBack()
            onLaunchRequested: function(id) { App.openGame(id) }
        }
    }
    Component { id: gameHostComponent; GameHost { onBackRequested: window.navigateBack() } }
    // Kept in the module for source compatibility; RCC v1 games open through GameHost.
    Component { id: legacyModHostComponent; ModHost { onBackRequested: window.navigateBack() } }

    Connections { target: App; function onGameOpened() { stack.push(gameHostComponent) } }
    Connections { target: Back; function onBackPressed() { window.navigateBack() } }
}
