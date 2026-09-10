// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

ThemeSurface {
    id: root
    property string keyText: ""
    property string description: ""
    surfaceToken: "surface.control"
    radius: Constants.n("alias.tooltip.radius")
    borderWidth: Constants.n("alias.tooltip.borderWidth")
    implicitHeight: row.implicitHeight + Constants.n("alias.keyHint.verticalPadding") * Constants.u2
    implicitWidth: row.implicitWidth + Constants.n("alias.keyHint.horizontalPadding") * Constants.u2

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Constants.n("alias.tooltip.gap")
        ThemeBadge { text: root.keyText }
        TokenText { visible: root.description.length > Constants.spaceNone; text: root.description; role: "caption"; colorToken: "color.textMuted" }
    }
}
