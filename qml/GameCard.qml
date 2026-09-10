// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

ThemeSurface {
    id: root
    required property string gameId
    required property string gameName
    required property string gameDescription
    required property string gameCategory
    required property string gameVersion
    required property string gameIcon
    required property string gameIconUrl
    required property string gamePublisher
    required property string publisherStatus
    required property bool publisherVerified
    required property bool builtIn
    required property string gameSource
    required property int pluginLevel
    signal playRequested(string id)

    radius: Constants.n("alias.gameCard.radius")
    color: Constants.gameCardBackgroundNormal
    borderColor: pointer.containsMouse ? Constants.gameCardBorderHover : Constants.gameCardBorderNormal
    borderWidth: pointer.containsMouse ? Constants.n("alias.gameCard.borderHover") : Constants.n("alias.gameCard.borderNormal")
    scale: pointer.pressed ? Constants.n("alias.gameCard.pressedScale") : (pointer.containsMouse ? Constants.n("alias.gameCard.hoverScale") : Constants.scaleNormal)

    Behavior on scale { NumberAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : 0; easing.type: Easing.OutCubic } }
    Behavior on borderColor { ColorAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : 0 } }

    MouseArea {
        id: pointer
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            Audio.play("click")
            root.playRequested(root.gameId)
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.gameCard.contentPadding")
        spacing: Constants.n("alias.gameCard.contentGap")

        Row {
            width: parent.width
            spacing: Constants.n("alias.gameCard.headerGap")

            GameIcon { width: Constants.n("alias.gameCard.iconSize"); height: Constants.n("alias.gameCard.iconSize"); gameId: root.gameId; iconUrl: root.gameIconUrl }

            Column {
                width: parent.width - Constants.n("alias.gameCard.iconReservedWidth")
                spacing: Constants.n("alias.gameCard.headerTextGap")

                Text {
                    width: parent.width
                    text: Lang.text(root.gameName, Lang.language)
                    color: Constants.text
                    font.pixelSize: Constants.n("alias.gameCard.titleFontSize")
                    font.bold: true
                    elide: Text.ElideRight
                }

                Row {
                    width: parent.width
                    spacing: Constants.u5
                    Text { width: Math.max(Constants.u1, parent.width - publisherBadge.width - parent.spacing); text: root.gamePublisher; color: root.publisherVerified ? Constants.accent : Constants.textMuted; font.pixelSize: Constants.n("alias.gameCard.metaFontSize"); font.bold: root.publisherVerified; elide: Text.ElideRight }
                    PublisherBadge { id: publisherBadge; width: Constants.u16; height: Constants.u16; anchors.verticalCenter: parent.verticalCenter; status: root.publisherStatus; verified: root.publisherVerified }
                }

                Row {
                    spacing: Constants.n("alias.gameCard.metaGap")
                    Text { text: Lang.text(root.gameCategory, Lang.language); color: Constants.primaryHover; font.pixelSize: Constants.n("alias.gameCard.metaFontSize"); font.bold: true }
                    ThemeSurface { width: Constants.n("alias.gameCard.metaDotSize"); height: Constants.n("alias.gameCard.metaDotSize"); radius: Constants.n("alias.gameCard.metaDotRadius"); color: Constants.textSubtle; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "v" + root.gameVersion; color: Constants.textMuted; font.pixelSize: Constants.n("alias.gameCard.metaFontSize") }
                }
            }
        }

        Text {
            width: parent.width
            text: Lang.text(root.gameDescription, Lang.language)
            color: Constants.textMuted
            wrapMode: Text.WordWrap
            maximumLineCount: Constants.n("alias.gameCard.descriptionLines")
            elide: Text.ElideRight
            font.pixelSize: Constants.n("alias.gameCard.descriptionFontSize")
            lineHeight: Constants.ratioSpecial1_12
        }

        Item { width: Constants.n("alias.gameCard.spacerSize"); height: Constants.n("alias.gameCard.spacerSize") }

        Row {
            width: parent.width
            spacing: Constants.n("alias.gameCard.footerGap")

            ThemeSurface {
                width: sourceText.implicitWidth + Constants.n("alias.gameCard.badgeHorizontalExtra")
                height: Constants.n("alias.gameCard.badgeHeight")
                radius: Constants.n("alias.gameCard.badgeRadius")
                color: Constants.surfaceInteractive
                Text {
                    id: sourceText
                    anchors.centerIn: parent
                    text: root.builtIn ? qsTr("Built-in") : (root.pluginLevel === 3 ? qsTr("Trusted native") : qsTr("Mod"))
                    color: Constants.primaryHover
                    font.pixelSize: Constants.n("alias.gameCard.badgeFontSize")
                }
            }

            Item { width: Math.max(Constants.n("alias.gameCard.minSpacerWidth"), parent.width - sourceText.implicitWidth - Constants.n("alias.gameCard.trailingReservedWidth") - playRow.implicitWidth - Constants.n("alias.gameCard.trailingGap")); height: Constants.n("alias.gameCard.spacerSize") }

            Row {
                id: playRow
                spacing: Constants.n("alias.gameCard.playGap")
                Text { text: qsTr("PLAY"); color: Constants.accent; font.pixelSize: Constants.n("alias.gameCard.playFontSize"); font.bold: true }
                AppIcon { width: Constants.n("alias.gameCard.playIconSize"); height: Constants.n("alias.gameCard.playIconSize"); name: "play"; anchors.verticalCenter: parent.verticalCenter }
            }
        }
    }
}
