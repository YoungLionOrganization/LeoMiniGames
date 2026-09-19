// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

PressableSurface {
    id: root
    property string iconName: ""
    property string accessibleName: iconName
    role: "iconButton"
    surfaceToken: "surface.control"
    implicitWidth: Math.max(Constants.iconButtonTouchTarget, Constants.iconButtonHeight)
    implicitHeight: Math.max(Constants.iconButtonTouchTarget, Constants.iconButtonHeight)
    Accessible.name: accessibleName
    Accessible.role: Accessible.Button
    AppIcon {
        anchors.centerIn: parent
        width: Constants.iconButtonIconSize
        height: Constants.iconButtonIconSize
        name: root.iconName
        color: root.enabled ? Constants.iconButtonIconNormal : Constants.iconButtonIconDisabled
    }
}
