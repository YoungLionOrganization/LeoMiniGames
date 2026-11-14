// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Flickable {
    id: root
    contentWidth: width
    readonly property real topInset: Math.max(Constants.spaceMd, Viewport.safeTop)
    readonly property real bottomInset: Math.max(Constants.spaceMd, Viewport.safeBottom)
    contentHeight: Math.max(height, content.implicitHeight + topInset + bottomInset)
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    interactive: contentHeight > height
    ScrollBar.vertical: ScrollBar {}
    property var game: App.currentGame

    function titleText() {
        if (game.state === "idle") return qsTr("Ready?")
        if (game.state === "waiting") return qsTr("Wait…")
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
            if (game.state === "ready") GameAudio.playEffect("qrc:/sfx/ready.wav")
            else if (game.state === "false_start") GameAudio.playEffect("qrc:/sfx/error.wav")
            else if (game.state === "result") GameAudio.playEffect("qrc:/sfx/win.wav")
        }
    }

    Column {
        id: content
        anchors.horizontalCenter: parent.horizontalCenter
        y: root.topInset + Math.max(0, (root.height - root.topInset - root.bottomInset - height) / 2)
        width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.reaction.maxWidth"))
        spacing: Constants.n("alias.game.reaction.contentSpacing")

        Flow {
            id: stats
            width: parent.width
            spacing: Constants.n("alias.game.reaction.statGap")
            StatChip { label: qsTr("Last"); value: game.lastMs > 0 ? game.lastMs + " ms" : "—"; accent: Constants.accent }
            StatChip { label: qsTr("Best"); value: game.bestMs > 0 ? game.bestMs + " ms" : "—"; accent: Constants.success }
            StatChip { label: qsTr("Average"); value: game.averageMs > 0 ? game.averageMs + " ms" : "—"; accent: Constants.info }
            StatChip { label: qsTr("Rounds"); value: game.rounds.toString(); accent: Constants.primaryHover }
        }

        ThemeSurface {
            id: pad
            objectName: "reactionPad"
            width: Math.min(parent.width, Constants.n("alias.game.reaction.padMaxSize"))
            height: {
                const available = root.height - root.topInset - root.bottomInset - stats.implicitHeight - content.spacing
                const minimum = Math.max(Constants.touchTarget,
                                         padCopy.implicitHeight + 2 * Constants.spaceMd)
                return Math.max(minimum, Math.min(width, available))
            }
            anchors.horizontalCenter: parent.horizontalCenter
            radius: Math.min(width, height) * Constants.ratio50
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
                id: padCopy
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
                    wrapMode: Text.WordWrap
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
                        GameAudio.playEffect("qrc:/sfx/click.wav")
                        game.startRound()
                    } else {
                        game.tap()
                    }
                }
            }
        }
    }
}
