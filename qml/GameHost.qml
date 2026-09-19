// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    signal backRequested()
    readonly property bool externalGame: App.currentGameSource === "ExternalRcc" || App.currentGameSource === "DeveloperRcc"

    function syncViewport() {
        const simulated = App.currentGameSource === "DeveloperRcc" && Developer.profileWidth > 0
        Viewport.update(simulated ? Developer.profileWidth : Math.max(Constants.viewportMinWidth, root.externalGame ? externalContainer.width : gameLoader.width),
                        simulated ? Developer.profileHeight : Math.max(Constants.viewportMinHeight, root.externalGame ? externalContainer.height : gameLoader.height),
                        simulated ? Developer.profileSafeTop : Constants.spaceNone,
                        simulated ? Developer.profileSafeBottom : Constants.spaceNone,
                        Screen.devicePixelRatio)
    }

    AppBackground { anchors.fill: parent }

    Column {
        anchors.fill: parent
        spacing: Constants.spaceNone

        ThemeSurface {
            width: parent.width
            height: Constants.n("alias.gameHost.header.height")
            surfaceToken: "surface.panel"
            color: Constants.gameHudBackgroundNormal
            borderColor: Constants.gameHudBorderNormal

            Row {
                anchors.fill: parent
                anchors.margins: Constants.n("alias.gameHost.header.margin")
                spacing: Constants.n("alias.gameHost.header.gap")

                BronzeButton {
                    text: ""
                    iconName: "back"
                    iconSize: Math.round(Constants.n("alias.gameHost.backIcon"))
                    width: Constants.n("alias.gameHost.backSize")
                    height: Constants.n("alias.gameHost.backSize")
                    onClicked: root.backRequested()
                }

                Column {
                    width: parent.width - Constants.n("alias.gameHost.header.height")
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: Constants.n("alias.gameHost.titleGap")
                    Text {
                        width: parent.width
                        text: { const n = Games.nameFor(App.currentGameId); return n.length > 0 ? Lang.text(n, Lang.language) : App.currentGameId }
                        color: Constants.gameHudTextNormal
                        font.pixelSize: Constants.n("alias.gameHost.titleFont")
                        font.weight: Constants.typographyHeadingWeight
                        elide: Text.ElideRight
                    }
                    Text {
                        width: parent.width
                        text: { const d = Games.descriptionFor(App.currentGameId); return d.length > 0 ? Lang.text(d, Lang.language) : (App.currentGameSource === "DeveloperRcc" ? qsTr("Local Developer Lab package") : "") }
                        color: Constants.textMuted
                        font.pixelSize: Constants.n("alias.gameHost.subtitleFont")
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: Math.max(Constants.viewportMinHeight, parent.height - Constants.n("alias.gameHost.header.height"))

            Item {
                id: externalContainer
                readonly property bool simulated: App.currentGameSource === "DeveloperRcc" && Developer.profileWidth > 0
                width: simulated ? Developer.profileWidth : parent.width
                height: simulated ? Developer.profileHeight : parent.height
                anchors.centerIn: parent
                scale: simulated ? Math.min(parent.width / Math.max(1, width), parent.height / Math.max(1, height)) : 1
                visible: root.externalGame
                clip: true

                Rectangle {
                    visible: parent.simulated && Developer.profileSafeTop > 0
                    anchors.top: parent.top
                    width: parent.width
                    height: Developer.profileSafeTop
                    color: Constants.danger
                    opacity: Constants.ratio20
                    z: Constants.u100
                }
                Rectangle {
                    visible: parent.simulated && Developer.profileSafeBottom > 0
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: Developer.profileSafeBottom
                    color: Constants.danger
                    opacity: Constants.ratio20
                    z: Constants.u100
                }
            }

            Loader {
                id: gameLoader
                anchors.fill: parent
                source: root.externalGame ? "" : App.currentGameUrl
                focus: true
                onWidthChanged: root.syncViewport()
                onHeightChanged: root.syncViewport()
                onStatusChanged: {
                    root.syncViewport()
                    if (status === Loader.Ready) {
                        Lifecycle.attach(item, App.currentGameId)
                        Lifecycle.load()
                        Lifecycle.start()
                        forceActiveFocus()
                    } else if (status === Loader.Error) {
                        GameLogger.log("error", "QML game entry failed to load", source.toString(), 0)
                        Audio.play("error")
                    }
                }

                Keys.onPressed: function(event) {
                    if (GameInput.handleKey(event.key, event.nativeScanCode, event.text, true, event.isAutoRepeat))
                        event.accepted = false
                }
                Keys.onReleased: function(event) {
                    if (GameInput.handleKey(event.key, event.nativeScanCode, event.text, false, event.isAutoRepeat))
                        event.accepted = false
                }
            }

            Connections {
                target: Developer
                function onProfileChanged() { root.syncViewport() }
            }

            Connections {
                target: ExternalRuntime
                function onLoaded(item) {
                    Lifecycle.attach(item, App.currentGameId)
                    Lifecycle.load()
                    Lifecycle.start()
                    gameLoader.forceActiveFocus()
                }
                function onFailed(message) {
                    GameLogger.log("error", message, App.currentGameUrl.toString(), 0)
                    Audio.play("error")
                }
            }

            ThemeSurface {
                anchors.fill: parent
                visible: (!root.externalGame && gameLoader.status === Loader.Error) || (root.externalGame && ExternalRuntime.status === "error")
                surfaceToken: "surface.overlay"
                color: Constants.background
                Column {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - Constants.n("alias.gameHost.errorInset"), Constants.n("alias.gameHost.error.maxWidth"))
                    spacing: Constants.n("alias.gameHost.errorGap")
                    AppIcon {
                        width: Constants.n("alias.gameHost.errorIcon")
                        height: width
                        name: "error"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        width: parent.width
                        text: qsTr("This game could not be loaded.")
                        color: Constants.text
                        font.pixelSize: Constants.n("alias.gameHost.errorTitleFont")
                        font.weight: Constants.typographyHeadingWeight
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        width: parent.width
                        text: GameLogger.lastError
                        color: Constants.textMuted
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: Constants.n("alias.gameHost.errorBodyFont")
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        syncViewport()
        if (root.externalGame)
            ExternalRuntime.load(externalContainer, App.currentGameUrl, App.currentGameId, App.currentGameSource === "ExternalRcc" && Mods.networkAllowedFor(App.currentGameId))
    }
}
