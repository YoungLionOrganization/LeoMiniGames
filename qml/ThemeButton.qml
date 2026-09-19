// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

PressableSurface {
    id: root
    property string text: ""
    property string iconName: ""
    property bool primary: false
    property real iconSize: Constants.buttonIconSize
    signal activated()

    role: "button"
    surfaceToken: "surface.control"
    implicitHeight: Constants.buttonHeight
    implicitWidth: Math.max(Constants.buttonTouchTarget, row.implicitWidth + Constants.buttonPaddingX * Constants.u2)
    color: primary ? Constants.primary : Constants.buttonBackgroundNormal
    borderColor: primary ? Constants.primaryHover : Constants.buttonBorderNormal
    onClicked: activated()

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Constants.buttonGap
        AppIcon {
            visible: root.iconName.length > 0
            width: visible ? root.iconSize : Constants.spaceNone
            height: visible ? root.iconSize : Constants.spaceNone
            name: root.iconName
            color: root.primary ? Constants.background : Constants.buttonIconNormal
        }
        TokenText {
            visible: root.text.length > 0
            text: root.text
            role: "button"
            colorToken: root.primary ? "color.background" : "alias.button.text.normal"
        }
    }
}
