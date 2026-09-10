// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

GameTileSurface {
    id: root
    property string iconSource: ""
    property int count: 0
    property bool selected: false
    implicitWidth: Constants.inventorySlotMinWidth
    implicitHeight: Constants.inventorySlotHeight
    borderColor: selected ? Constants.inventorySlotBorderFocus : Constants.inventorySlotBorderNormal
    Image { anchors.fill: parent; anchors.margins: Constants.inventorySlotPadding; source: root.iconSource.length > 0 ? root.iconSource : ""; fillMode: Image.PreserveAspectFit; asynchronous: true }
    ThemeBadge { visible: root.count > 1; text: root.count.toString(); anchors.right: parent.right; anchors.bottom: parent.bottom }
}
