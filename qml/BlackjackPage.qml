// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Flickable {
    id: root
    property var game: App.currentGame

    function messageText() {
        if (!game) return ""
        switch (game.message) {
        case "your_turn": return qsTr("Your turn")
        case "push_both_blackjack": return qsTr("Push — both have Blackjack")
        case "blackjack_win": return qsTr("Blackjack! You win")
        case "dealer_blackjack": return qsTr("Dealer has Blackjack")
        case "bust": return qsTr("Bust — dealer wins")
        case "you_win": return qsTr("You win")
        case "dealer_win": return qsTr("Dealer wins")
        case "push": return qsTr("Push")
        default: return game.message
        }
    }

    contentWidth: width
    contentHeight: content.implicitHeight + Constants.n("alias.game.blackjack.contentBottomPadding")
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    Connections {
        target: game
        function onRoundStarted() {
            GameAudio.playEffect("qrc:/sfx/shuffle.wav")
            GameAudio.playEffect("qrc:/sfx/deal.wav")
        }
        function onCardDealt(hand) {
            GameAudio.playEffect("qrc:/sfx/deal.wav")
        }
        function onStateChanged() {
            if (!game.roundOver) return
            if (game.message === "blackjack_win" || game.message === "you_win")
                GameAudio.playEffect("qrc:/sfx/win.wav")
            else if (game.message === "dealer_blackjack" || game.message === "bust" || game.message === "dealer_win")
                GameAudio.playEffect("qrc:/sfx/lose.wav")
            else if (game.message === "push" || game.message === "push_both_blackjack")
                GameAudio.playEffect("qrc:/sfx/success.wav")
        }
    }

    Column {
        id: content
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: Constants.n("alias.game.blackjack.contentSpacing")
        spacing: Constants.n("alias.game.blackjack.contentSpacing")

        ThemeSurface {
            width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.blackjack.messageMaxWidth"))
            height: Constants.n("alias.game.blackjack.messageHeight")
            anchors.horizontalCenter: parent.horizontalCenter
            radius: Constants.radiusLg
            color: Constants.surfaceRaised
            borderColor: Constants.borderStrong

            Text {
                anchors.centerIn: parent
                text: messageText()
                color: game.roundOver ? Constants.accent : Constants.text
                font.pixelSize: Constants.n("alias.game.blackjack.messageFontSize")
                font.bold: true
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Constants.n("alias.game.blackjack.statGap")
            StatChip { label: qsTr("Wins"); value: game.wins.toString(); accent: Constants.success }
            StatChip { label: qsTr("Pushes"); value: game.pushes.toString(); accent: Constants.accent }
            StatChip { label: qsTr("Losses"); value: game.losses.toString(); accent: Constants.danger }
        }

        ThemeSurface {
            width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.blackjack.tableMaxWidth"))
            height: dealerZone.implicitHeight + Constants.n("alias.game.blackjack.tablePadding")
            anchors.horizontalCenter: parent.horizontalCenter
            radius: Constants.radiusLg
            color: Constants.surfaceRaised

            Column {
                id: dealerZone
                width: parent.width - Constants.n("alias.game.blackjack.tablePadding")
                anchors.centerIn: parent
                spacing: Constants.n("alias.game.blackjack.zoneGap")

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Dealer") + " · " + game.dealerValue
                    color: Constants.primaryHover
                    font.bold: true
                }

                Flow {
                    width: parent.width
                    spacing: Constants.n("alias.game.blackjack.cardGap")
                    Repeater {
                        model: game.dealerCards
                        delegate: CardVisual { required property string modelData; label: modelData }
                    }
                }
            }
        }

        ThemeSurface {
            width: Math.min(parent.width - Constants.n("alias.game.common.horizontalInset"), Constants.n("alias.game.blackjack.tableMaxWidth"))
            height: playerZone.implicitHeight + Constants.n("alias.game.blackjack.tablePadding")
            anchors.horizontalCenter: parent.horizontalCenter
            radius: Constants.radiusLg
            color: Constants.surfaceRaised
            borderColor: Constants.borderStrong

            Column {
                id: playerZone
                width: parent.width - Constants.n("alias.game.blackjack.tablePadding")
                anchors.centerIn: parent
                spacing: Constants.n("alias.game.blackjack.zoneGap")

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Player") + " · " + game.playerValue
                    color: Constants.accent
                    font.bold: true
                }

                Flow {
                    width: parent.width
                    spacing: Constants.n("alias.game.blackjack.cardGap")
                    Repeater {
                        model: game.playerCards
                        delegate: CardVisual { required property string modelData; label: modelData }
                    }
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Constants.n("alias.game.blackjack.actionGap")

            BronzeButton {
                text: qsTr("Hit")
                enabled: game.canHit
                soundOnPress: false
                onClicked: game.hit()
            }
            BronzeButton {
                text: qsTr("Stand")
                enabled: game.canStand
                soundOnPress: false
                onClicked: game.stand()
            }
            BronzeButton {
                text: qsTr("New hand")
                enabled: game.roundOver
                onClicked: game.newRound()
            }
        }

        BronzeButton {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Reset score")
            onClicked: game.resetScore()
        }
    }
}
