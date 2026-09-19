// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var game: App.currentGame
    property bool flagMode: false

    function playSfx(name) { Audio.play(name) }

    function statusText() {
        if (!game) return ""
        if (game.status === "ready") return qsTr("Ready")
        if (game.status === "playing") return qsTr("Playing")
        if (game.status === "mine_hit") return qsTr("Mine hit")
        if (game.status === "cleared") return qsTr("Board cleared")
        return game.status
    }

    Connections {
        target: game
        function onStatusChanged() {
            if (game.status === "mine_hit") Audio.play("lose")
            else if (game.status === "cleared") Audio.play("win")
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.game.minesweeper.pageInset")
        spacing: Constants.n("alias.game.minesweeper.toolbarGap")

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Constants.n("alias.game.minesweeper.sectionGap")
            BronzeButton { text: qsTr("Easy"); checkable: true; checked: game.difficulty === GameRules.minesweeperEasy; onClicked: game.setDifficulty(GameRules.minesweeperEasy) }
            BronzeButton { text: qsTr("Medium"); checkable: true; checked: game.difficulty === GameRules.minesweeperMedium; onClicked: game.setDifficulty(GameRules.minesweeperMedium) }
            BronzeButton { text: qsTr("Hard"); checkable: true; checked: game.difficulty === GameRules.minesweeperHard; onClicked: game.setDifficulty(GameRules.minesweeperHard) }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Constants.n("alias.game.minesweeper.sectionGap")
            StatChip { label: qsTr("Time"); value: game.elapsedSeconds + "s"; accent: Constants.accent }
            StatChip { label: qsTr("Best"); value: game.bestSeconds > 0 ? game.bestSeconds + "s" : "-"; accent: Constants.success }
            StatChip { label: qsTr("Wins"); value: game.wins.toString(); accent: Constants.primaryHover }
        }

        Row {
            width: parent.width
            spacing: Constants.n("alias.game.minesweeper.actionGap")

            BronzeButton {
                text: root.flagMode ? qsTr("Flag mode: ON") : qsTr("Flag mode")
                iconName: "flag"
                iconSize: Constants.n("alias.game.minesweeper.actionIconSize")
                checkable: true
                checked: root.flagMode
                onClicked: root.flagMode = checked
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: Constants.n("alias.game.minesweeper.flagGap")
                AppIcon { width: Constants.n("alias.game.minesweeper.actionIconSize"); height: Constants.n("alias.game.minesweeper.actionIconSize"); name: "flag" }
                Text { text: game.flagsRemaining; color: Constants.accent; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
            }

            Item { width: Constants.n("alias.game.minesweeper.spacerSize"); height: Constants.n("alias.game.minesweeper.spacerSize") }
            BronzeButton { text: qsTr("Reset"); onClicked: game.reset() }
        }

        Text {
            width: parent.width
            text: qsTr("Tap to reveal · hold to flag · or use Flag mode")
            color: Constants.textMuted
            font.pixelSize: Constants.n("alias.game.minesweeper.toolbarGap")
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            text: statusText()
            color: game.won ? Constants.success : (game.gameOver ? Constants.danger : Constants.primaryHover)
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
        }

        Flickable {
            id: flick
            width: parent.width
            height: parent.height - y
            contentWidth: board.width
            contentHeight: board.height
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: contentWidth > width || contentHeight > height

            property real cellSize: Math.max(Constants.n("alias.game.minesweeper.cellMin"), Math.min(Constants.n("alias.game.minesweeper.cellMax"), width / game.columns - Constants.n("alias.game.minesweeper.cellGap")))

            Grid {
                id: board
                columns: game.columns
                spacing: Constants.n("alias.game.minesweeper.cellGap")
                width: game.columns * (flick.cellSize + spacing) - spacing
                height: game.rows * (flick.cellSize + spacing) - spacing
                anchors.horizontalCenter: parent.width > width ? parent.horizontalCenter : undefined

                Repeater {
                    model: game.cells

                    delegate: ThemeSurface {
                        id: cell
                        required property int index
                        required property var modelData

                        width: flick.cellSize
                        height: flick.cellSize
                        radius: Constants.n("alias.game.minesweeper.cellRadius")
                        color: modelData.revealed ? Constants.surfaceInteractive
                             : modelData.flagged ? Constants.surfaceInteractive
                             : Constants.surfaceRaised
                        borderColor: modelData.flagged ? Constants.accent : Constants.borderStrong
                        borderWidth: modelData.revealed ? Constants.n("alias.game.minesweeper.revealedBorderWidth") : Constants.n("alias.game.minesweeper.hiddenBorderWidth")

                        scale: modelData.revealed ? Constants.n("alias.game.minesweeper.revealedScale") : Constants.scaleNormal
                        Behavior on scale { NumberAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : Constants.durationInstant; easing.type: Constants.easing("alias.motion.enter.easing") } }
                        Behavior on color { ColorAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : Constants.durationInstant } }

                        AppIcon {
                            anchors.centerIn: parent
                            width: Math.max(Constants.n("alias.game.minesweeper.iconMin"), flick.cellSize * Constants.n("alias.game.minesweeper.iconRatio"))
                            height: width
                            name: modelData.mine ? "mine" : "flag"
                            visible: modelData.mine || modelData.flagged
                            opacity: modelData.flagged ? Constants.n("alias.game.minesweeper.flaggedOpacity") : Constants.opacityFull
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: modelData.revealed && !modelData.mine && modelData.adjacent > Constants.spaceNone
                            text: modelData.adjacent
                            color: modelData.adjacent === GameRules.minesweeperNeighborLow ? Constants.info
                                 : modelData.adjacent === GameRules.minesweeperNeighborMedium ? Constants.success
                                 : modelData.adjacent >= GameRules.minesweeperNeighborDanger ? Constants.danger
                                 : Constants.text
                            font.bold: true
                            font.pixelSize: Math.max(Constants.n("alias.game.minesweeper.numberFontMin"), flick.cellSize * Constants.n("alias.game.minesweeper.numberFontRatio"))
                        }

                        TapHandler {
                            id: primaryTap
                            acceptedButtons: Qt.LeftButton
                            gesturePolicy: TapHandler.ReleaseWithinBounds
                            longPressThreshold: Constants.ratioSpecial0_62

                            onTapped: function(eventPoint, button) {
                                if (!root.game || cell.modelData.revealed)
                                    return
                                if (root.flagMode) {
                                    if (root.game.toggleFlag(cell.index))
                                        root.playSfx("flag")
                                } else {
                                    const opened = root.game.openCell(cell.index)
                                    if (opened && !root.game.gameOver)
                                        root.playSfx("move")
                                }
                            }

                            onLongPressed: {
                                if (!root.game || cell.modelData.revealed)
                                    return
                                if (root.game.toggleFlag(cell.index))
                                    root.playSfx("flag")
                            }
                        }

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
                            gesturePolicy: TapHandler.ReleaseWithinBounds
                            onTapped: {
                                if (!root.game || cell.modelData.revealed)
                                    return
                                if (root.game.toggleFlag(cell.index))
                                    root.playSfx("flag")
                            }
                        }
                    }
                }
            }
        }
    }
}
