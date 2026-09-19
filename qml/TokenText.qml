// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Text {
    id: root
    property string role: "body"
    property string colorToken: "color.text"

    color: Constants.c(colorToken)
    font.pixelSize: Constants.n("alias.typography." + role + ".size")
    font.weight: Math.round(Constants.n("alias.typography." + role + ".weight"))
    font.letterSpacing: Constants.n("alias.typography." + role + ".letterSpacing")
    lineHeightMode: Text.ProportionalHeight
    lineHeight: Constants.n("alias.typography." + role + ".lineHeight")
    renderType: Text.QtRendering
}
