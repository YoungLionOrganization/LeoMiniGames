// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

ThemeSurface {
    id: root
    property string label: ""
    property string value: ""
    property color accent: Constants.statChipAccentNormal

    surfaceToken: "surface.badge"
    implicitWidth: Math.max(Constants.n("alias.statChip.minWidth"), content.implicitWidth + Constants.n("alias.statChip.contentPaddingX"))
    implicitHeight: Constants.n("alias.statChip.height")
    radius: Constants.n("alias.statChip.radius")
    color: Constants.statChipBackgroundNormal
    borderColor: Qt.rgba(accent.r, accent.g, accent.b, Constants.n("alias.statChip.borderAlpha"))
    borderWidth: Constants.statChipBorderWidth

    Column {
        id: content
        anchors.centerIn: parent
        spacing: Constants.n("alias.statChip.gap")

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.value
            color: root.accent
            font.pixelSize: Constants.n("alias.statChip.valueFontSize")
            font.weight: Constants.typographyGameScoreWeight
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: Constants.statChipTextNormal
            font.pixelSize: Constants.n("alias.statChip.labelFontSize")
        }
    }
}
