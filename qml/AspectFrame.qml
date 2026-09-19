// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    default property alias contentData: frame.data
    property real aspectRatio: Constants.ratio100
    property bool fitInside: true
    readonly property alias contentItem: frame

    Item {
        id: frame
        anchors.centerIn: parent
        width: root.fitInside ? Math.min(parent.width, parent.height * Math.max(Constants.ratio1, root.aspectRatio)) : parent.width
        height: root.fitInside ? width / Math.max(Constants.ratio1, root.aspectRatio) : Math.min(parent.height, width / Math.max(Constants.ratio1, root.aspectRatio))
    }
}
