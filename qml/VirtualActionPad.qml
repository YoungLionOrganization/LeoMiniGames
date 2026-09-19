// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    property bool showDirections: true
    property bool showAction: true
    property string actionName: "fire"
    implicitWidth: Constants.gameJoystickSize * Constants.u2 + Constants.gameBoardGap
    implicitHeight: Constants.gameJoystickSize

    Item {
        id: directions
        visible: root.showDirections
        width: Constants.gameJoystickSize
        height: width
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        ThemeIconButton { iconName: "up"; anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top; onPressedChanged: pressed ? GameInput.press("up") : GameInput.release("up") }
        ThemeIconButton { iconName: "down"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; onPressedChanged: pressed ? GameInput.press("down") : GameInput.release("down") }
        ThemeIconButton { iconName: "back"; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; onPressedChanged: pressed ? GameInput.press("move_left") : GameInput.release("move_left") }
        ThemeIconButton { iconName: "play"; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; onPressedChanged: pressed ? GameInput.press("move_right") : GameInput.release("move_right") }
    }
    ThemeIconButton {
        visible: root.showAction
        iconName: "play"
        width: Constants.gameActionSize
        height: width
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        onPressedChanged: pressed ? GameInput.press(root.actionName) : GameInput.release(root.actionName)
    }
}
