// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var game: App.currentGame
    property real startX: Constants.spaceNone
    property real startY: Constants.spaceNone

    focus: true
    Component.onCompleted: forceActiveFocus()

    function doMove(direction) {
        if (game.move(direction))
            Audio.play("merge")
        else
            Audio.play("error")
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Left) { doMove("left"); event.accepted = true }
        else if (event.key === Qt.Key_Right) { doMove("right"); event.accepted = true }
        else if (event.key === Qt.Key_Up) { doMove("up"); event.accepted = true }
        else if (event.key === Qt.Key_Down) { doMove("down"); event.accepted = true }
    }

    Connections {
        target: game
        function onStateChanged() {
            if (game.status === "won") Audio.play("win")
            else if (game.status === "game_over") Audio.play("lose")
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.2048.maxWidth"))
        spacing: Constants.n("alias.game.2048.contentSpacing")

        Flow {
            width: parent.width
            spacing: Constants.n("alias.game.twentyFortyEight.statGap")
            StatChip { label: qsTr("Score"); value: game.score.toString(); accent: Constants.accent }
            StatChip { label: qsTr("Best"); value: game.bestScore.toString(); accent: Constants.success }
            StatChip { label: qsTr("Moves"); value: game.moves.toString(); accent: Constants.primaryHover }
            StatChip { label: qsTr("Max"); value: game.highestTile.toString(); accent: Constants.info }
        }

        Text {
            width: parent.width
            text: game.gameOver ? qsTr("No moves left.")
                 : game.won ? qsTr("2048 reached! Keep going.")
                 : qsTr("Swipe or use arrow keys.")
            color: game.status === "game_over" ? Constants.danger : Constants.textMuted
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Constants.n("alias.game.twentyFortyEight.helperFontSize")
        }

        ThemeSurface {
            id: board
            width: parent.width
            height: width
            radius: Constants.radiusLg
            color: Constants.surfaceRaised
            borderColor: Constants.surfaceInteractive

            Grid {
                anchors.fill: parent
                anchors.margins: Constants.n("alias.game.2048.boardInset")
                columns: GameRules.twentyFortyEightColumns
                spacing: Constants.n("alias.game.2048.cellGap")

                Repeater {
                    model: game.tiles

                    delegate: ThemeSurface {
                        id: tile
                        required property int index
                        required property var modelData
                        width: (parent.width - parent.spacing * (GameRules.twentyFortyEightColumns - Constants.ratio100)) / GameRules.twentyFortyEightColumns
                        height: (parent.height - parent.spacing * (GameRules.twentyFortyEightColumns - Constants.ratio100)) / GameRules.twentyFortyEightColumns
                        radius: Constants.n("alias.game.2048.cellRadius")

                        property int value: Number(modelData)
                        color: value === GameRules.twentyFortyEightEmpty ? Constants.surfaceRaised
                             : value <= GameRules.twentyFortyEightBronzeLowMax ? Constants.borderStrong
                             : value <= GameRules.twentyFortyEightBronzeHighMax ? Constants.primary
                             : value <= GameRules.twentyFortyEightGoldLowMax ? Constants.accent
                             : value <= GameRules.twentyFortyEightGoldHighMax ? Constants.accent
                             : Constants.text

                        Text {
                            anchors.centerIn: parent
                            text: tile.value === GameRules.twentyFortyEightEmpty ? "" : tile.value
                            color: tile.value >= GameRules.twentyFortyEightDarkTextFrom ? Constants.background : Constants.text
                            font.pixelSize: tile.value >= GameRules.twentyFortyEightSmallFontFrom ? Constants.n("alias.game.2048.fontSmall") : tile.value >= GameRules.twentyFortyEightDarkTextFrom ? Constants.n("alias.game.2048.fontMedium") : Constants.n("alias.game.2048.fontLarge")
                            font.bold: true
                        }

                        onValueChanged: {
                            if (value > GameRules.twentyFortyEightEmpty && Settings.animationsEnabled)
                                pop.restart()
                        }

                        SequentialAnimation {
                            id: pop
                            NumberAnimation { target: tile; property: "scale"; from: Constants.n("alias.game.2048.spawnScale"); to: Constants.n("alias.game.2048.spawnOvershootScale"); duration: Constants.n("alias.game.2048.spawnDuration") }
                            NumberAnimation { target: tile; property: "scale"; to: Constants.scaleNormal; duration: Constants.n("alias.game.2048.spawnDuration") }
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onPressed: function(mouse) { root.startX = mouse.x; root.startY = mouse.y }
                onReleased: function(mouse) {
                    const dx = mouse.x - root.startX
                    const dy = mouse.y - root.startY
                    if (Math.max(Math.abs(dx), Math.abs(dy)) < Constants.n("alias.game.2048.swipeThreshold"))
                        return
                    root.doMove(Math.abs(dx) > Math.abs(dy)
                                ? (dx > Constants.spaceNone ? "right" : "left")
                                : (dy > Constants.spaceNone ? "down" : "up"))
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
