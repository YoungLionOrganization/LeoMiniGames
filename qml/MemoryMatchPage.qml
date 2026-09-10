// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var game: App.currentGame

    function playSfx(name) {
        Audio.play(name)
    }

    Connections {
        target: game
        function onStateChanged() {
            if (game.status === "cleared")
                root.playSfx("win")
        }
        function onPairResolved(matched) {
            root.playSfx(matched ? "match" : "miss")
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.memory.maxWidth"))
        spacing: Constants.n("alias.game.memory.contentSpacing")

        Flow {
            width: parent.width
            spacing: Constants.n("alias.game.memory.statGap")
            StatChip { label: qsTr("Moves"); value: game.moves.toString(); accent: Constants.accent }
            StatChip { label: qsTr("Pairs"); value: game.matches + "/" + GameRules.memoryPairCount; accent: Constants.primaryHover }
            StatChip { label: qsTr("Time"); value: game.elapsedSeconds + "s"; accent: Constants.info }
            StatChip { label: qsTr("Best"); value: game.bestMoves > 0 ? game.bestMoves.toString() : "—"; accent: Constants.success }
        }

        Text {
            width: parent.width
            text: game.gameOver ? qsTr("All pairs found!") : qsTr("Remember the cards and match every pair.")
            color: game.gameOver ? Constants.success : Constants.textMuted
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Constants.n("alias.game.memory.helperFontSize")
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
                anchors.margins: Constants.n("alias.game.memory.boardInset")
                columns: GameRules.memoryColumns
                spacing: Constants.n("alias.game.memory.cellGap")

                Repeater {
                    model: game.cards

                    delegate: ThemeSurface {
                        id: card
                        required property int index
                        required property var modelData

                        width: (board.width - board.spacing * (GameRules.memoryColumns - Constants.ratio100)) / GameRules.memoryColumns
                        height: (board.height - board.spacing * (GameRules.memoryColumns - Constants.ratio100)) / GameRules.memoryColumns
                        radius: Constants.n("alias.game.memory.cellRadius")
                        color: modelData.matched ? Constants.borderStrong
                             : modelData.faceUp ? Constants.accent
                             : Constants.surfaceRaised
                        borderColor: modelData.faceUp || modelData.matched ? Constants.accent : Constants.borderStrong

                        Text {
                            id: face
                            anchors.centerIn: parent
                            text: modelData.faceUp || modelData.matched ? modelData.symbol : "?"
                            color: modelData.faceUp ? Constants.background : Constants.text
                            font.pixelSize: Constants.n("alias.game.memory.cardFontSize")
                            font.bold: true
                        }

                        scale: tap.pressed ? Constants.n("alias.game.memory.pressedScale") : Constants.scaleNormal
                        Behavior on scale {
                            NumberAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : Constants.durationInstant }
                        }
                        Behavior on color {
                            ColorAnimation { duration: Settings.animationsEnabled ? Constants.durationNormal : Constants.durationInstant }
                        }

                        TapHandler {
                            id: tap
                            enabled: !game.locked && !modelData.matched
                            onTapped: {
                                if (game.flip(index))
                                    playSfx("flip")
                            }
                        }
                    }
                }
            }
        }

        BronzeButton {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("New game")
            onClicked: {
                Audio.play("shuffle")
                game.reset()
            }
        }
    }
}
