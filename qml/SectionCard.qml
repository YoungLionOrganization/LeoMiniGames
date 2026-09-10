// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

ThemeSurface {
    id: root
    default property alias content: contentArea.data
    property string title: ""
    property string subtitle: ""

    surfaceToken: "surface.card"
    implicitHeight: contentArea.implicitHeight + titleBlock.implicitHeight + Constants.n("alias.sectionCard.extraHeight")
    radius: Constants.cardRadius
    color: Constants.cardBackgroundNormal
    borderColor: Constants.cardBorderNormal
    borderWidth: Constants.cardBorderWidth

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.sectionCard.padding")
        spacing: Constants.n("alias.sectionCard.gap")

        Column {
            id: titleBlock
            width: parent.width
            spacing: Constants.n("alias.sectionCard.titleGap")
            Text {
                text: root.title
                color: Constants.cardTextNormal
                font.pixelSize: Constants.n("alias.sectionCard.titleFontSize")
                font.weight: Constants.typographyHeadingWeight
            }
            Text {
                width: parent.width
                visible: root.subtitle.length > 0
                text: root.subtitle
                color: Constants.textMuted
                font.pixelSize: Constants.n("alias.sectionCard.subtitleFontSize")
                wrapMode: Text.WordWrap
            }
        }

        Column {
            id: contentArea
            width: parent.width
            spacing: Constants.n("alias.sectionCard.contentGap")
        }
    }
}
