// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    default property alias contentData: content.data
    property string role: "panel"
    property string surfaceToken: "surface.panel"
    property real padding: Constants.n("alias." + role + ".padding")
    property real contentSpacing: Constants.n("alias." + role + ".gap")
    readonly property alias backgroundItem: background
    readonly property alias contentItem: content

    implicitWidth: Math.max(Constants.n("alias." + role + ".minWidth"), Constants.n("alias." + role + ".width"))
    implicitHeight: Math.max(Constants.n("alias." + role + ".minHeight"), Constants.n("alias." + role + ".height"))

    ThemeSurface {
        id: background
        anchors.fill: parent
        surfaceToken: root.surfaceToken
        radius: Constants.n("alias." + root.role + ".radius")
        borderWidth: Constants.n("alias." + root.role + ".borderWidth")
    }

    Item {
        id: content
        anchors.fill: parent
        anchors.margins: root.padding
    }
}
