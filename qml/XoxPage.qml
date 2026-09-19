// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var game: App.currentGame

    function statusText() {
        if (!game) return ""
        if (game.status === "win") return qsTr("Winner") + ": " + game.currentPlayer
        if (game.status === "draw") return qsTr("Draw")
        return qsTr("Turn") + ": " + game.currentPlayer
    }

    Connections {
        target: game
        function onStatusChanged() {
            if (game.status === "win") Audio.play("win")
            else if (game.status === "draw") Audio.play("success")
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.xox.maxWidth"))
        spacing: Constants.n("alias.game.xox.contentSpacing")

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: statusText()
            color: Constants.accent
            font.pixelSize: Constants.n("alias.game.xox.statusFontSize")
            font.bold: true
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Constants.n("alias.game.xox.statGap")
            StatChip { label: qsTr("X wins"); value: game.xWins.toString(); accent: Constants.accent }
            StatChip { label: qsTr("Draws"); value: game.draws.toString(); accent: Constants.textMuted }
            StatChip { label: qsTr("O wins"); value: game.oWins.toString(); accent: Constants.primaryHover }
        }

        ThemeSurface {
            width: parent.width
            height: width
            radius: Constants.radiusLg
            color: Constants.surfaceRaised
            borderColor: Constants.surfaceInteractive

            Grid {
                id: board
                anchors.fill: parent
                anchors.margins: Constants.n("alias.game.xox.boardInset")
                columns: GameRules.xoxColumns
                spacing: Constants.n("alias.game.xox.statGap")

                Repeater {
                    model: GameRules.xoxCellCount

                    delegate: ThemeSurface {
                        id: cell
                        required property int index
                        width: (board.width - board.spacing * (GameRules.xoxColumns - Constants.ratio100)) / GameRules.xoxColumns
                        height: (board.height - board.spacing * (GameRules.xoxColumns - Constants.ratio100)) / GameRules.xoxColumns
                        radius: Constants.n("alias.game.xox.cellRadius")
                        color: tap.pressed ? Constants.surfaceInteractive : Constants.surfaceRaised
                        borderColor: game.board[index] === "X" ? Constants.accent
                                    : game.board[index] === "O" ? Constants.primaryHover
                                    : Constants.surfaceInteractive

                        Text {
                            id: mark
                            anchors.centerIn: parent
                            text: game.board[index]
                            color: text === "X" ? Constants.accent : Constants.primaryHover
                            font.pixelSize: Math.max(Constants.n("alias.game.xox.markMinFont"), cell.width * Constants.n("alias.game.xox.markFontRatio"))
                            font.bold: true

                            onTextChanged: {
                                if (mark.text.length > 0 && Settings.animationsEnabled)
                                    pop.restart()
                            }
                        }

                        SequentialAnimation {
                            id: pop
                            NumberAnimation { target: mark; property: "scale"; from: Constants.n("alias.game.xox.mark.enterScale"); to: Constants.n("alias.game.xox.mark.overshootScale"); duration: Constants.n("alias.game.xox.mark.enterDuration"); easing.type: Constants.easing("alias.motion.enter.easing") }
                            NumberAnimation { target: mark; property: "scale"; to: Constants.scaleNormal; duration: Constants.n("alias.game.xox.mark.settleDuration"); easing.type: Constants.easing("alias.motion.standard.easing") }
                        }

                        TapHandler {
                            id: tap
                            enabled: !game.gameOver && game.board[index] === ""
                            onTapped: {
                                if (game.play(index) && game.status === "turn")
                                    Audio.play("move")
                            }
                        }
                    }
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Constants.n("alias.game.xox.actionGap")
            BronzeButton { text: qsTr("New round"); onClicked: game.newRound() }
            BronzeButton { text: qsTr("Reset score"); onClicked: game.resetScore() }
        }
    }
}
