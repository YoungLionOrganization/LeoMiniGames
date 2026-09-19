// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    implicitWidth: Constants.u56
    implicitHeight: Constants.u56

    ThemeSurface {
        anchors.fill: parent
        radius: Constants.u18
        color: Constants.surfaceRaised
        borderWidth: Constants.u1
        borderColor: Constants.primary
        rotation: Constants.n("alias.logoMark.rotation")
        scale: Constants.ratio76
    }

    Text {
        anchors.centerIn: parent
        text: "L"
        color: Constants.accent
        font.pixelSize: Constants.u29
        font.bold: true
    }

    ThemeSurface {
        width: Constants.u6; height: Constants.u6; radius: Constants.u3
        x: parent.width * Constants.n("alias.logoMark.eyeOneX"); y: parent.height * Constants.n("alias.logoMark.eyeOneY")
        color: Constants.primaryHover
    }
    ThemeSurface {
        width: Constants.u6; height: Constants.u6; radius: Constants.u3
        x: parent.width * Constants.n("alias.logoMark.eyeTwoX"); y: parent.height * Constants.n("alias.logoMark.eyeTwoY")
        color: Constants.primaryHover
    }
}
