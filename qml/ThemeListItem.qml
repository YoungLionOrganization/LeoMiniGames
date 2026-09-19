// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

PressableSurface {
    id: root
    property string title: ""
    property string subtitle: ""
    property string leadingIcon: ""
    property string trailingIcon: ""
    role: "listItem"
    surfaceToken: "surface.control"
    implicitHeight: Math.max(Constants.n("alias.listItem.height"), textColumn.implicitHeight + Constants.n("alias.listItem.paddingY") * Constants.u2)

    Row {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.listItem.padding")
        spacing: Constants.n("alias.listItem.gap")
        AppIcon { visible: root.leadingIcon.length > Constants.spaceNone; width: Constants.n("alias.listItem.iconSize"); height: width; name: root.leadingIcon; color: Constants.c("alias.listItem.icon.normal") }
        Column {
            id: textColumn
            width: Math.max(Constants.spaceNone, parent.width - (root.leadingIcon.length > Constants.spaceNone ? Constants.n("alias.listItem.iconSize") + parent.spacing : Constants.spaceNone) - (root.trailingIcon.length > Constants.spaceNone ? Constants.n("alias.listItem.iconSize") + parent.spacing : Constants.spaceNone))
            anchors.verticalCenter: parent.verticalCenter
            spacing: Constants.n("alias.listItem.gap")
            TokenText { width: parent.width; text: root.title; role: "body"; elide: Text.ElideRight }
            TokenText { width: parent.width; visible: root.subtitle.length > Constants.spaceNone; text: root.subtitle; role: "caption"; colorToken: "color.textMuted"; elide: Text.ElideRight }
        }
        AppIcon { visible: root.trailingIcon.length > Constants.spaceNone; width: Constants.n("alias.listItem.iconSize"); height: width; name: root.trailingIcon; color: Constants.c("alias.listItem.icon.normal") }
    }
}
