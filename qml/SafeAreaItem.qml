// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    default property alias contentData: content.data
    property real extraTop: Constants.spaceNone
    property real extraBottom: Constants.spaceNone
    property real horizontalPadding: Constants.pagePadding
    readonly property real safeTop: Math.max(Constants.spaceNone, Viewport.safeTop) + extraTop
    readonly property real safeBottom: Math.max(Constants.spaceNone, Viewport.safeBottom) + extraBottom
    readonly property alias contentItem: content

    Item {
        id: content
        anchors.fill: parent
        anchors.leftMargin: root.horizontalPadding
        anchors.rightMargin: root.horizontalPadding
        anchors.topMargin: root.safeTop
        anchors.bottomMargin: root.safeBottom
    }
}
