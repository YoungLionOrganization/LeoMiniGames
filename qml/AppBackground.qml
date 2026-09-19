// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    ThemeSurface {
        anchors.fill: parent
        surfaceToken: "surface.app"
    }

    ThemeSurface {
        width: Math.max(parent.width * Constants.n("alias.appBackground.primaryGlow.widthRatio"), Constants.n("alias.appBackground.primaryGlow.minSize"))
        height: width
        radius: width * Constants.ratio50
        x: -width * Constants.n("alias.appBackground.primaryGlow.xOffsetRatio")
        y: -height * Constants.n("alias.appBackground.primaryGlow.yOffsetRatio")
        color: Constants.primary
        opacity: Constants.ratioSpecial0_055
    }

    ThemeSurface {
        width: Math.max(parent.width * Constants.n("alias.appBackground.secondaryGlow.widthRatio"), Constants.n("alias.appBackground.secondaryGlow.minSize"))
        height: width
        radius: width * Constants.ratio50
        x: parent.width - width * Constants.n("alias.appBackground.secondaryGlow.xOffsetRatio")
        y: parent.height - height * Constants.n("alias.appBackground.secondaryGlow.yOffsetRatio")
        color: Constants.accent
        opacity: Constants.ratioSpecial0_035
    }
}
