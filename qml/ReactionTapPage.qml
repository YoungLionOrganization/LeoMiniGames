// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var game: App.currentGame

    function titleText() {
        if (game.state === "idle") return qsTr("Ready?")
        if (game.state === "waiting") return qsTr("Wait for gold…")
        if (game.state === "ready") return qsTr("TAP!")
        if (game.state === "false_start") return qsTr("Too early!")
        return game.lastMs + " ms"
    }

    function subtitleText() {
        if (game.state === "idle") return qsTr("Tap the pad to start.")
        if (game.state === "waiting") return qsTr("Do not tap yet.")
        if (game.state === "ready") return qsTr("Now!")
        if (game.state === "false_start") return qsTr("Tap again to retry.")
        return qsTr("Tap again for another round.")
    }

    Connections {
        target: game
        function onStateChanged() {
            if (game.state === "ready") Audio.play("ready")
            else if (game.state === "false_start") Audio.play("error")
            else if (game.state === "result") Audio.play("win")
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.reaction.maxWidth"))
        spacing: Constants.n("alias.game.reaction.contentSpacing")

        Flow {
            width: parent.width
            spacing: Constants.n("alias.game.reaction.statGap")
            StatChip { label: qsTr("Last"); value: game.lastMs > 0 ? game.lastMs + " ms" : "—"; accent: Constants.accent }
            StatChip { label: qsTr("Best"); value: game.bestMs > 0 ? game.bestMs + " ms" : "—"; accent: Constants.success }
            StatChip { label: qsTr("Average"); value: game.averageMs > 0 ? game.averageMs + " ms" : "—"; accent: Constants.info }
            StatChip { label: qsTr("Rounds"); value: game.rounds.toString(); accent: Constants.primaryHover }
        }

        ThemeSurface {
            id: pad
            width: Math.min(parent.width, Constants.n("alias.game.reaction.padMaxSize"))
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            radius: width * Constants.ratio50
            color: game.state === "ready" ? Constants.accent
                 : game.state === "false_start" ? Constants.danger
                 : Constants.surfaceRaised
            borderWidth: Constants.n("alias.game.reaction.padBorderWidth")
            borderColor: game.state === "ready" ? Constants.accent : Constants.borderStrong

            property real pulseScale: Constants.scaleNormal
            scale: (tap.pressed ? Constants.n("alias.game.reaction.pressedScale") : Constants.scaleNormal) * pulseScale
            Behavior on scale { NumberAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : Constants.durationInstant } }
            Behavior on color { ColorAnimation { duration: Settings.animationsEnabled ? Constants.durationNormal : Constants.durationInstant } }

            SequentialAnimation on pulseScale {
                running: Settings.animationsEnabled && game.state === "ready" && !tap.pressed
                loops: Animation.Infinite
                NumberAnimation { to: Constants.n("alias.game.reaction.pulseScale"); duration: Constants.n("alias.game.reaction.pulseDuration"); easing.type: Easing.InOutSine }
                NumberAnimation { to: Constants.scaleNormal; duration: Constants.n("alias.game.reaction.pulseDuration"); easing.type: Easing.InOutSine }
            }

            Column {
                anchors.centerIn: parent
                width: parent.width * Constants.n("alias.game.reaction.contentWidthRatio")
                spacing: Constants.n("alias.game.reaction.innerGap")

                Text {
                    width: parent.width
                    text: titleText()
                    color: game.state === "ready" ? Constants.background : Constants.text
                    font.pixelSize: game.state === "ready" ? Constants.n("alias.game.reaction.readyFontSize") : Constants.n("alias.game.reaction.normalFontSize")
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                Text {
                    width: parent.width
                    text: subtitleText()
                    color: game.state === "ready" ? Constants.surfaceRaised : Constants.textMuted
                    font.pixelSize: Constants.n("alias.game.reaction.subtitleFontSize")
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }

            TapHandler {
                id: tap
                onTapped: {
                    if (game.state === "idle" || game.state === "result" || game.state === "false_start") {
                        Audio.play("click")
                        game.startRound()
                    } else {
                        game.tap()
                    }
                }
            }
        }
    }
}
