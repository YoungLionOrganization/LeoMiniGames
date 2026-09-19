// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

TextField {
    id: root
    property string leadingIcon: ""
    property string trailingIcon: ""
    signal trailingClicked()

    implicitHeight: Math.max(Constants.textFieldHeight, Constants.textFieldTouchTarget)
    leftPadding: Constants.textFieldPaddingX + (leadingIcon.length > 0 ? Constants.textFieldIconSize + Constants.textFieldGap : Constants.spaceNone)
    rightPadding: Constants.textFieldPaddingX + (trailingIcon.length > 0 ? Constants.textFieldIconSize + Constants.textFieldGap : Constants.spaceNone)
    topPadding: Constants.textFieldPaddingY
    bottomPadding: Constants.textFieldPaddingY
    color: Constants.textFieldTextNormal
    placeholderTextColor: Constants.textFieldTextDisabled
    selectionColor: Constants.primary
    selectedTextColor: Constants.background
    font.pixelSize: Constants.textFieldFontSize
    font.weight: Constants.textFieldFontWeight

    background: ThemeSurface {
        surfaceToken: "surface.control"
        radius: Constants.textFieldRadius
        borderWidth: root.activeFocus ? Constants.textFieldFocusRingWidth : Constants.textFieldBorderWidth
        borderColor: root.activeFocus ? Constants.textFieldBorderFocus : Constants.textFieldBorderNormal
    }
    AppIcon {
        visible: root.leadingIcon.length > 0
        name: root.leadingIcon
        width: Constants.textFieldIconSize
        height: Constants.textFieldIconSize
        anchors.left: parent.left
        anchors.leftMargin: Constants.textFieldPaddingX
        anchors.verticalCenter: parent.verticalCenter
        color: Constants.textFieldIconNormal
    }
    ThemeIconButton {
        visible: root.trailingIcon.length > 0
        iconName: root.trailingIcon
        width: Constants.textFieldTouchTarget
        height: Constants.textFieldTouchTarget
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        onClicked: root.trailingClicked()
    }
}
