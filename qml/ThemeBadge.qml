// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

ThemeSurface {
    id: root
    property string text: ""
    property string tone: "accent"
    surfaceToken: "surface.badge"
    implicitHeight: Constants.badgeHeight
    implicitWidth: label.implicitWidth + Constants.badgePaddingX * Constants.u2
    radius: Constants.badgeRadius
    color: tone === "success" ? Constants.success
         : tone === "danger" ? Constants.danger
         : tone === "info" ? Constants.info
         : Constants.badgeBackgroundNormal
    TokenText {
        id: label
        anchors.centerIn: parent
        text: root.text
        role: "labelSmall"
        color: root.tone === "accent" ? Constants.badgeTextNormal : Constants.background
    }
}
