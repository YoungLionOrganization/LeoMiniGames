// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Item {
    id: root
    property alias fillMode: logo.fillMode

    ThemeSurface {
        anchors.fill: parent
        radius: Math.min(width, height) * Constants.n("alias.brandLogo.radiusRatio")
        color: Constants.background
        borderColor: Constants.primary
        borderWidth: Math.max(Constants.ratio100, Math.round(Math.min(width, height) / Constants.n("alias.brandLogo.borderDivisor")))
    }

    Image {
        id: logo
        anchors.centerIn: parent
        width: parent.width * Constants.n("alias.brandLogo.imageRatio")
        height: parent.height * Constants.n("alias.brandLogo.imageRatio")
        source: "qrc:/branding/leominigames_icon.png"
        sourceSize.width: Math.max(Constants.n("alias.brandLogo.sourceMin"), root.width * Constants.n("alias.brandLogo.sourceScale"))
        sourceSize.height: Math.max(Constants.n("alias.brandLogo.sourceMin"), root.height * Constants.n("alias.brandLogo.sourceScale"))
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        asynchronous: false
    }
}
